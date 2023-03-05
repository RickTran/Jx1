///////////////////////////////////////////////////////////////////////////////
// HttpDownload.cpp: implementation of the CHttpDownload class.
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "HttpDownload.h"
#include "SocksPacket.h"

#define READ_BUFFER_SIZE	(10 * 1024)				// �����С 10K
#define DEFAULT_SAVE_DIR	_T("C:\\")				// ȱʡ����Ŀ¼
#define DEFAULT_SAVE_FILE	_T("Unknown.htm")		// ȱʡ�����ļ���
#define DEFAULT_USERAGENT	_T("KAVUpdate/1.0")		// ȱʡ��UserAgent
#define Check(a,b)			if((a))return (b);		

// ����BASE64���롢����ľ�̬����
CString CHttpDownload::m_strBase64TAB = _T( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" );
int		CHttpDownload::m_nBase64Mask[]= { 0, 1, 3, 7, 15, 31, 63, 127, 255 };

////////////////////////////////////////////////////////////////////////////////
//						���캯��
////////////////////////////////////////////////////////////////////////////////
CHttpDownload::CHttpDownload()
{
	m_strDownloadUrl	= _T("");
	m_strSavePath		= _T("");
	m_strTempSavePath	= _T("");
	m_bSaveResponse		= TRUE;	//�Ƿ���Ҫ���淵��

	// ֹͣ����
	m_bStopDownload		= FALSE;
	m_hStopEvent		= NULL;

	// ǿ����������(�������е��ļ��Ƿ���Զ���ļ���ͬ)
	m_bForceDownload	= FALSE;

	// �Ƿ�֧�ֶϵ�����(�ٶ���֧��)
	m_bSupportResume	= FALSE;

	// �ļ��Լ����ش�С
	m_dwFileSize			= 0;	// �ļ��ܵĴ�С	
	m_dwFileDownloadedSize	= 0;	// �ļ��ܹ��Ѿ����صĴ�С

	m_dwDownloadSize	= 0;		// ����Request��Ҫ���صĴ�С
	m_dwDownloadedSize	= 0;		// ����Request�Ѿ����صĴ�С

	m_dwHeaderSize		= 0;		// HTTPЭ��ͷ�ĳ���
	m_strHeader			= _T("");	// HTTPЭ��ͷ

	// Referer
	m_strReferer		= _T("");
	
	// UserAgent
	m_strUserAgent		= DEFAULT_USERAGENT;

	// ��ʱTIMEOUT	���ӳ�ʱ�����ͳ�ʱ�����ճ�ʱ(��λ������)
	m_dwConnTimeout		= HTTP_CONN_TIMEOUT;	
	m_dwRecvTimeout		= HTTP_RECV_TIMEOUT;
	m_dwSendTimeout		= HTTP_SEND_TIMEOUT;

	// ���Ի���
	m_nRetryType		= HTTP_RETRY_NONE;	//��������(0:������ 1:����һ������ 2:��������)
	m_nRetryTimes		= 0;				//���Դ���
	m_nRetryDelay		= 0;				//�����ӳ�(��λ������)
	m_nRetryMax			= 0;				//����������

	// ������
	m_nErrorCount		= 0;			//�������
	m_strError			= _T("");		//������Ϣ
	m_dwErrorCode		= 0;			//�������

	// ���������ڷ�����Ϣ
	m_bNotify			= FALSE;		// �Ƿ����ⷢ��֪ͨ��Ϣ	
	m_hNotifyWnd		= NULL;			// ��֪ͨ�Ĵ���
	m_nNotifyMessage	= 0;			// ��֪ͨ����Ϣ

	// �Ƿ������֤ : Request-Header: Authorization
	m_bAuthorization	= FALSE;
	m_strUsername		= _T("");
	m_strPassword		= _T("");

	// �Ƿ�ʹ�ô��� 
	m_bProxy			= FALSE;
	m_strProxyServer	= _T("");
	m_nProxyPort		= 0;
	m_nProxyType		= HTTP_PROXY_NONE;
	
	// �����Ƿ���Ҫ��֤: Request-Header: Proxy-Authorization
	m_bProxyAuthorization = FALSE;
	m_strProxyUsername 	= _T("");
	m_strProxyPassword	= _T("");

	// ���ع��������õı���
	m_strServer			= _T("");
	m_strObject			= _T("");
	m_strFileName		= _T("");
	m_nPort				= DEFAULT_HTTP_PORT ;
	m_nVerb				= HTTP_VERB_GET;
	m_strData			= _T("");

	// �Ƿ�ʹ��Cookie
	m_strCookie			= _T("");
	m_bUseIECookie		= FALSE;
	
}


////////////////////////////////////////////////////////////////////////////////
//		��������
////////////////////////////////////////////////////////////////////////////////
CHttpDownload::~CHttpDownload()
{
	CloseSocket();
	if( m_hStopEvent != NULL )
	{
		CloseHandle( m_hStopEvent );
		m_hStopEvent = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////
//	��������BOOL CreateSocket()
//	��  ;������SOCKET
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ������
//	����ֵ��BOOL
//		TRUE : �ɹ�����SOCKET
//		FALSE: ����SOCKETʧ��
////////////////////////////////////////////////////////////////////////////////
BOOL CHttpDownload::CreateSocket()
{
	CloseSocket();
	return m_cBufSocket.Create(AF_INET,SOCK_STREAM,0,READ_BUFFER_SIZE);
}


//////////////////////////////////////////////////////////////////////////////////
//	��������void CloseSocket()
//	��  ;���ر�SOCKET
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ������
//	����ֵ��void
////////////////////////////////////////////////////////////////////////////////
void CHttpDownload::CloseSocket()
{
	m_cBufSocket.Close(TRUE);	
}


//////////////////////////////////////////////////////////////////////////////////
//	��������int Download(
//				LPCTSTR lpszDownloadUrl,
//				LPCTSTR lpszSavePath	/*= NULL*/,
//				BOOL bForceDownload		/*= FALSE*/,
//				BOOL bSaveResponse		/*= TRUE*/,
//				int nVerb				/*= HTTP_VERB_GET*/,
//				LPCTSTR lpszData		/*= NULL*/,
//				LPTSTR lpszFtpURL		/*= NULL*/) 
//	��  ;��������ڣ����ô˺�������ʼ���ع���
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		lpszDownloadUrl : �����ص�����URL
//		lpszSavePath    : ���ر���·��(������ȫ·�����߽�����Ŀ¼��Ŀ¼����\��β)	
//		bForceDownload  : �Ƿ�ǿ�����أ������ļ��Ƿ���Զ��һ��
//		bSaveResponse   : �Ƿ񱣴�������ķ���
//		nVerb           : HTTP����
//		lpszData        : Ҫ�ڷ�������ʱ����������������(����POST)
//		lpszFtpURL		: ����ض���FTP,��ָ����������FTP��URL
//	����ֵ��int
//		HTTP_RESULT_STOP		: �û�ֹͣ����(������StopDownload����)
//		HTTP_RESULT_FAIL		: ����ʧ��
//		HTTP_RESULT_SUCCESS		: ���سɹ�
//		HTTP_RESULT_SAMEAS		: ����������ȫһ�����ļ�������������
//		HTTP_RESULT_REDIRECT_FTP: �ض���FTP
////////////////////////////////////////////////////////////////////////////////
int CHttpDownload::Download(LPCTSTR lpszDownloadUrl,LPCTSTR lpszSavePath /*= NULL*/,BOOL bForceDownload /*= FALSE*/,BOOL bSaveResponse /*= TRUE*/,int nVerb /*= HTTP_VERB_GET*/,LPCTSTR lpszData /*= NULL*/,LPTSTR lpszFtpURL /*= NULL*/ )
{
	m_bStopDownload	  = FALSE;
	m_bForceDownload  = bForceDownload;
	m_nRetryTimes	  = 0;
	m_nErrorCount	  = 0;
	m_strTempSavePath = _T("");
	
	m_bSaveResponse	  = bSaveResponse;
	m_nVerb			  = nVerb;
	m_strData		  = _T("");
	if( lpszData != NULL )
		m_strData	+=	lpszData;
	

	// ����Ҫ���ص�URL�Ƿ�Ϊ��
	m_strDownloadUrl = lpszDownloadUrl;
	m_strDownloadUrl.TrimLeft();
	m_strDownloadUrl.TrimRight();
	if( m_strDownloadUrl.IsEmpty() )
		return HTTP_RESULT_FAIL;

	// ����Ҫ���ص�URL�Ƿ���Ч
	if ( !ParseURL(m_strDownloadUrl, m_strServer, m_strObject, m_nPort))
	{
		// ��ǰ�����"http://"����
		m_strDownloadUrl = _T("http://") + m_strDownloadUrl;
		if ( !ParseURL(m_strDownloadUrl,m_strServer, m_strObject, m_nPort) )
		{
			TRACE(_T("Failed to parse the URL: %s\n"), m_strDownloadUrl);
			return HTTP_RESULT_FAIL;
		}
	}

	// ��鱾�ر���·��
	m_strSavePath =  lpszSavePath;
	m_strSavePath.TrimLeft();
	m_strSavePath.TrimRight();
	if( m_strSavePath.IsEmpty() )
		m_strSavePath = DEFAULT_SAVE_DIR;
	
	if( m_strSavePath.Right(1) != '\\' )	// ֻ��·����û���ļ���
	{
		m_strTempSavePath =  m_strSavePath;
		m_strTempSavePath += ".tmp";
	}

	// ����ֹͣ�¼�
	if( m_hStopEvent == NULL )
	{
		m_hStopEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
		if( m_hStopEvent == NULL )
			return HTTP_RESULT_FAIL;

		//seawind
		m_cBufSocket.m_hStopEvent = m_hStopEvent;
	}
	ResetEvent( m_hStopEvent );

	
	// ������������
	m_dwDownloadedSize		= 0;
	m_dwFileDownloadedSize	= 0;
	m_dwFileSize			= 0;
	m_dwDownloadSize		= 0;

	BOOL bSendOnce = TRUE;		// ���ڿ�����hWndNotify���ڷ�����Ϣ
	BOOL bSendRequestAgain = TRUE;
	
	while( bSendRequestAgain )
	{
		int nRequestRet = SendRequest( nVerb,lpszFtpURL ) ;

		//seawind
		//if (WaitForSingleObject(m_hStopEvent, 0) == WAIT_OBJECT_0)
		//	return HTTP_RESULT_STOP;
		Check( m_bStopDownload, HTTP_RESULT_STOP);

		switch(nRequestRet)
		{
		case HTTP_REQUEST_SUCCESS:
			// ��Ҫ�����?
			if( !m_bSaveResponse )
			{
				//�ر�SOCKET
				CloseSocket();
				return HTTP_REQUEST_SUCCESS;				
			}
			break;
		case HTTP_REQUEST_STOP:
			return HTTP_RESULT_STOP;
			break;
		case HTTP_REQUEST_FAIL:
			return HTTP_RESULT_FAIL;
			break;
		case HTTP_REQUEST_REDIRECT_FTP:
			return HTTP_RESULT_REDIRECT_FTP;
			break;
		case HTTP_REQUEST_ERROR:

			Check( m_bStopDownload, HTTP_RESULT_STOP);	// �Ƿ�Ӧ��ֹͣ����

			switch( m_nRetryType )
			{
			case HTTP_RETRY_NONE:
				return HTTP_RESULT_FAIL;
				break;
			case HTTP_RETRY_ALWAYS:
				if( m_nRetryDelay > 0 )
				{
					//Ϊ���õȴ�ʱҲ���˳�������ʹ��Sleep����
					if( WaitForSingleObject(m_hStopEvent,m_nRetryDelay) == WAIT_OBJECT_0 )
						return HTTP_RESULT_STOP;
				}
				continue;
				break;
			case HTTP_RETRY_TIMES:
				if( m_nRetryTimes > m_nRetryMax )
					return HTTP_RESULT_FAIL;
				m_nRetryTimes++;
			
				if( m_nRetryDelay > 0 )
				{
					//Ϊ���õȴ�ʱҲ���˳�������ʹ��Sleep����
					if( WaitForSingleObject(m_hStopEvent,m_nRetryDelay) == WAIT_OBJECT_0 )
						return HTTP_RESULT_STOP;
				}
				continue;
				break;
			default:
				return HTTP_RESULT_FAIL;
				break;
			}
			break;
		default:
			return HTTP_RESULT_FAIL;
			break;
		}

		if( m_strSavePath.Right(1) == '\\' )
			m_strSavePath += m_strFileName;

		if( !m_bForceDownload ) // ��ǿ�����أ����Last-Modified
		{
			CFileStatus fileStatus;
			if (CFile::GetStatus(m_strSavePath,fileStatus))
			{
				// ���ܻ����1������
				if (( fileStatus.m_mtime - m_TimeLastModified <=2 && m_TimeLastModified-fileStatus.m_mtime<=2 ) && (DWORD)fileStatus.m_size == m_dwFileSize )
					return HTTP_RESULT_SAMEAS;
			}
		}

		CFile fileDown;
		if(! fileDown.Open(m_strTempSavePath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite|CFile::shareDenyWrite) )
			return HTTP_RESULT_FAIL;	

		// Ӧ���ж�һ���Ƿ�֧�ֶϵ�����
		if( m_bSupportResume )
		{
			try
			{
				fileDown.SeekToEnd();
			}
			catch(CFileException* e)                                         
			{
				e->Delete();
				return HTTP_RESULT_FAIL;
			}	
		}

		// ������Ϣ
		if( bSendOnce && m_bNotify )
		{
			HTTPDOWNLOADSTATUS DownloadStatus;
			
			DownloadStatus.dwFileSize  = m_dwFileSize;
			DownloadStatus.strFileName = m_strFileName;
			DownloadStatus.dwFileDownloadedSize  = m_dwFileDownloadedSize;

			DownloadStatus.nStatusType = HTTP_STATUS_FILESIZE;			
			if (WaitForSingleObject(m_hStopEvent, 0) != WAIT_OBJECT_0)
				::SendMessage(m_hNotifyWnd,m_nNotifyMessage,MSG_HTTPDOWNLOAD_STATUS,(LPARAM)&DownloadStatus);
			else
				return HTTP_RESULT_STOP;

			DownloadStatus.nStatusType = HTTP_STATUS_FILENAME;
			if (WaitForSingleObject(m_hStopEvent, 0) != WAIT_OBJECT_0)
				::SendMessage(m_hNotifyWnd,m_nNotifyMessage,MSG_HTTPDOWNLOAD_STATUS,(LPARAM)&DownloadStatus);	
			else
				return HTTP_RESULT_STOP;
		
			DownloadStatus.nStatusType = HTTP_STATUS_FILEDOWNLOADEDSIZE;
			if (WaitForSingleObject(m_hStopEvent, 0) != WAIT_OBJECT_0)
				::SendMessage(m_hNotifyWnd,m_nNotifyMessage,MSG_HTTPDOWNLOAD_STATUS,(LPARAM)&DownloadStatus);	
			else
				return HTTP_RESULT_STOP;
			
			bSendOnce = FALSE;
		}

		m_dwDownloadedSize = 0;
		// ���ڿ�ʼ��ȡ����
		char szReadBuf[READ_BUFFER_SIZE+1];

		/////////////////////////////////////////////
		// ���m_dwDownloadSize = 0 (˵����Сδ֪)
		if( m_dwDownloadSize == 0 )
			m_dwDownloadSize = 0xFFFFFFFF;
		////////////////////////////////////////////
		bSendRequestAgain = FALSE;
		do
		{
			// �Ƿ�Ӧ��ֹͣ����
			Check( m_bStopDownload, HTTP_RESULT_STOP);
			ZeroMemory(szReadBuf,READ_BUFFER_SIZE+1);
			int nRet = 	m_cBufSocket.BSDGetData(szReadBuf,READ_BUFFER_SIZE,m_dwRecvTimeout);

			//seawind
			Check( m_bStopDownload, HTTP_RESULT_STOP);

			if (nRet <= 0)
			{
				////////////////////////////////////////
				if( m_dwDownloadSize == 0xFFFFFFFF )
					break;
				///////////////////////////////////////
				m_nErrorCount++;
				bSendRequestAgain = TRUE;
				break;// �����ڲ�ѭ��
			}

			// ������д���ļ�
			try
			{
				fileDown.Write(szReadBuf,nRet);
			}
			catch(CFileException* e)
			{
				e->Delete();
				bSendRequestAgain = TRUE;
				break;// �����ڲ�ѭ��
			}

			m_dwDownloadedSize		+= nRet;
			m_dwFileDownloadedSize	+= nRet;

			// ֪ͨ��Ϣ
			if( m_bNotify )
			{
				HTTPDOWNLOADSTATUS DownloadStatus;
				DownloadStatus.nStatusType			= HTTP_STATUS_FILEDOWNLOADEDSIZE;
				DownloadStatus.dwFileDownloadedSize = m_dwFileDownloadedSize;
				DownloadStatus.dwFileSize			= m_dwFileSize;
				DownloadStatus.strFileName			= m_strFileName;
				
				if (WaitForSingleObject(m_hStopEvent, 0) != WAIT_OBJECT_0)
					::SendMessage(m_hNotifyWnd,m_nNotifyMessage,MSG_HTTPDOWNLOAD_STATUS,(LPARAM)&DownloadStatus);
				else
					return HTTP_RESULT_STOP;
			}

		}while(m_dwDownloadedSize < m_dwDownloadSize);

		// �ر��ļ�
		fileDown.Close();

	} // WHILE LOOP
	
	//�ر�SOCKET
	CloseSocket();

	// �ļ�����
	//���Ƚ����е��ļ�ɾ��
	::DeleteFile(m_strSavePath);

	//�ٽ������ص��ļ�����
	::MoveFile(m_strTempSavePath, m_strSavePath);
    
    //�ٽ������ص��ļ���ʱ��Ļ�ȥ
    CFileStatus fileStatus;
    
    fileStatus.m_size = 0;
    fileStatus.m_attribute = 0;
    
    try
    {
        CFile::GetStatus(m_strSavePath, fileStatus);
        
        fileStatus.m_mtime = m_TimeLastModified;
        CFile::SetStatus(m_strSavePath,fileStatus);
    }
    catch(CFileException *e)
    {
        e->Delete();
    }
    
    return HTTP_RESULT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////////
//	��������int SendRequest(
//				int nVerb /*= HTTP_VERB_GET*/,
//				LPTSTR lpszFtpURL/* = NULL*/ ) 
//	��  ;������HTTP����
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		nVerb      : HTTP����
//		lpszFtpURL : ����ض���FTP,��ָ����������FTP��URL
//	����ֵ��int
//		HTTP_REQUEST_STOP			: �û���ֹ
//		HTTP_REQUEST_ERROR			: ����һ��������󣬿�������
//		HTTP_REQUEST_SUCCESS		: ����ɹ�
//		HTTP_REQUEST_FAIL			: ������֤���󣬲�������
//		HTTP_REQUEST_REDIRECT_FTP	: �����ض���FTP
////////////////////////////////////////////////////////////////////////////////
int	CHttpDownload::SendRequest(int nVerb /*= HTTP_VERB_GET*/,LPTSTR lpszFtpURL/* = NULL*/)
{
	// �ж�HTTP�����Ƿ�Ϸ�
	if( nVerb < HTTP_VERB_MIN || nVerb > HTTP_VERB_MAX )
		return HTTP_REQUEST_FAIL;

	// ����HTTP����
	m_nVerb = nVerb;

	while (TRUE)
	{
		CString			strSend,strAuth,strHeader,strVerb,strCookie,strRange,strText;
		int				nRet;
		char			szReadBuf[1025];
		DWORD			dwContentLength,dwStatusCode;

		strSend		= _T("");
		strAuth		= _T("");
		strHeader	= _T("");
		strVerb		= _T("");
		strCookie	= _T("");
		strRange	= _T("");
		strText		= _T("");

		m_dwFileDownloadedSize = 0;			// �ļ��Ѿ����ش�С
		m_dwDownloadSize	   = 0;			// Ҫ���ش�С
		strVerb = HTTP_VERB_STR[m_nVerb];	// HTTP����

		///////////////////////////////////////
		// Ŀǰ�İ汾�У�����Ϣ��û����
		m_strHeader		= _T("");
		m_dwHeaderSize	= 0;
		//////////////////////////////////////
	
		Check( !CreateSocket(), HTTP_REQUEST_FAIL);		// ����SOCKET
		Check( m_bStopDownload, HTTP_REQUEST_STOP);		// �Ƿ�Ӧ��ֹͣ����

		// ��������
		nRet = MakeConnection( &m_cBufSocket,m_strServer,m_nPort);
		Check( nRet != HTTP_REQUEST_SUCCESS, nRet);
		Check( m_bStopDownload, HTTP_REQUEST_STOP);		// �Ƿ�Ӧ��ֹͣ����

		if( m_nProxyType == HTTP_PROXY_HTTPGET )		// HTTP_GET����
		{
			strSend  = strVerb  + m_strDownloadUrl + " HTTP/1.1\r\n";
			if( m_bProxyAuthorization )
			{
				strAuth = _T("");
				Base64Encode(m_strProxyUsername+":"+m_strProxyPassword,strAuth);
				strSend += "Proxy-Authorization: Basic "+strAuth+"\r\n";
			}
		}
		else	// û��Proxy���߲���HTTP_GET����
			strSend  = strVerb  + m_strObject + " HTTP/1.1\r\n";
		
		// ���ʵ�ҳ���Ƿ����ܱ�����
		if( m_bAuthorization )
		{
			strAuth = _T("");
			Base64Encode(m_strUsername+":"+m_strPassword,strAuth);
			strSend += "Authorization: Basic "+strAuth+"\r\n";
		}

		strSend += "Host: " + m_strServer + "\r\n";
		strSend += "Accept: */*\r\n";
		strSend += "Pragma: no-cache\r\n"; 
		strSend += "Cache-Control: no-cache\r\n";
		strSend += "User-Agent: " + m_strUserAgent + "\r\n";
		if( !m_strReferer.IsEmpty() )
			strSend += "Referer: " + m_strReferer + "\r\n";
		strSend += "Connection: close\r\n";

		// �ж��Ƿ�ʹ��Cookie
		if( m_bUseIECookie )
		{
			// ��ȡIE��Cookie
			if( GetIECookie(strCookie,m_strDownloadUrl) )
				m_strCookie = strCookie;
			else
				m_strCookie = _T("");
		}
		
		// Cookie
		if( !m_strCookie.IsEmpty() )
			strSend += "Cookie: " + m_strCookie + "\r\n";
		
		// �Ƿ񱣴�������ķ���
		if( m_bSaveResponse )
		{
			// �ж�m_strSavePath�Ƿ�Ϊ·��
			GetFileName();
			if( m_strSavePath.Right(1) == '\\' )
			{
				m_strTempSavePath = m_strSavePath;
				m_strTempSavePath += m_strFileName;
				m_strTempSavePath += ".tmp";
			}

			// �鿴�ļ��Ѿ����صĳ���
			CFileStatus fileDownStatus;
			if (CFile::GetStatus(m_strTempSavePath,fileDownStatus) && !m_bForceDownload )
			{
				m_dwFileDownloadedSize = fileDownStatus.m_size;
				if (m_dwFileDownloadedSize > 0)
				{
					strRange.Format(_T("Range: bytes=%d-\r\n"),m_dwFileDownloadedSize );
				}
			}
			strSend += strRange;
		}

		// Post?
		switch( m_nVerb )
		{
		case HTTP_VERB_GET:
		case HTTP_VERB_HEAD:
		//	strSend += strRange;
			break;
		case HTTP_VERB_POST:
			strSend += "Content-Type: application/x-www-form-urlencoded\r\n";
			strText.Format("Content-Length: %d\r\n", m_strData.GetLength() );
			strSend += strText;
			break;
		case HTTP_VERB_PUT:
		default:
			break;
		}

		//����Ҫ��һ�����У�����Http������������Ӧ��
		strSend += "\r\n";

		// Post Data?
		switch( m_nVerb )
		{
		case HTTP_VERB_GET:
		case HTTP_VERB_HEAD:
			break;
		case HTTP_VERB_POST:
		case HTTP_VERB_PUT:
			strSend += m_strData;
			break;
		default:
			break;
		}

		//��������
		nRet = 	m_cBufSocket.Send((LPCTSTR)strSend,strSend.GetLength(),m_dwSendTimeout);
		if( nRet < strSend.GetLength() )
		{
			if ( m_cBufSocket.GetLastError() == WSAETIMEDOUT)	// ��ʱ
				continue;
			else	// �������󣬿�����������ˣ��ȴ�һ��ʱ�������
				return HTTP_REQUEST_ERROR;
		}

		// ����HTTPͷ
		BOOL bAgain = TRUE;
		while( bAgain )
		{
			bAgain = FALSE;
			strHeader.Empty();
			while( TRUE )
			{
				Check(m_bStopDownload,HTTP_REQUEST_STOP);
				ZeroMemory(szReadBuf,1025);
				nRet = m_cBufSocket.BSDGetString(szReadBuf,1024,m_dwRecvTimeout);

				//seawind
				Check(m_bStopDownload,HTTP_REQUEST_STOP);

				Check( nRet == SOCKET_FAIL ,HTTP_REQUEST_ERROR);
				
				if( szReadBuf[0] == '\0' ) // We have encountered "\r\n\r\n"
					break; 
				
				strHeader += szReadBuf;
				if( nRet == SOCKET_SUCCESS )
					strHeader += "\r\n";
			}
			
			///////////////////////////////////////
			// Ŀǰ�İ汾�У�����Ϣ��û����
			m_strHeader		= strHeader;
			m_dwHeaderSize	= m_strHeader.GetLength();
			//////////////////////////////////////
			nRet = GetInfo(strHeader,dwContentLength,dwStatusCode,m_TimeLastModified,strCookie,lpszFtpURL);
			switch ( nRet )
			{
			case HTTP_FAIL:
				return HTTP_REQUEST_FAIL;
				break;
			case HTTP_REDIRECT:
				m_nVerb = HTTP_VERB_GET;
				continue;
				break;
			case HTTP_REDIRECT_FTP:
				return HTTP_REQUEST_REDIRECT_FTP;
				break;
			case HTTP_SUCCESS:
				
				if( dwStatusCode >= 100  && dwStatusCode <200 )
				{
					bAgain = TRUE;
					continue;	// Inner Loop
				}
				
				if( m_bSaveResponse )
				{
					m_dwDownloadSize = dwContentLength;
					// Ӧ���ж�һ�·������Ƿ�֧�ֶϵ�����
					if( strRange.IsEmpty() )
					{
						m_dwFileSize = dwContentLength; // �����ļ��ĳ���
						
						//seawind
						m_bSupportResume = FALSE;
					}
					else
					{
						if ( dwStatusCode == 206 )	//֧�ֶϵ�����
						{
							m_dwFileSize = (dwContentLength == 0 )?0:(m_dwFileDownloadedSize +dwContentLength);
							m_bSupportResume = TRUE;
						}
						else						//��֧�ֶϵ�����
						{
							m_bSupportResume = FALSE;
							m_dwFileDownloadedSize	= 0; //��֧�ֶϵ���������ֵҪ��Ϊ0
							m_dwFileSize = dwContentLength;
						}
					}// strRange
				}// m_bSaveResponse
				return HTTP_REQUEST_SUCCESS;
				break;
			default:
				return HTTP_REQUEST_FAIL;
				break;
			}// SWITCH
			
		} // WHILE bAgain
		
	}// WHILE LOOP

	return HTTP_REQUEST_SUCCESS;
}



//////////////////////////////////////////////////////////////////////////////////
//	��������void SetProxy(
//				LPCTSTR lpszProxyServer,
//				int nProxyPort,
//				BOOL bProxy/*=TRUE*/,
//				BOOL bProxyAuthorization /*= FALSE*/,
//				LPCTSTR lpszProxyUsername /*= NULL*/,
//				LPCTSTR lpszProxyPassword /*= NULL*/,
//				int nProxyType /*= HTTP_PROXY_HTTPGET*/ ) 
//	��  ;�����ô���������֤��ʽ
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		lpszProxyServer     : ����������ĵ�ַ(IP��ַ��������)
//		nProxyPort          : ����������Ķ˿�
//		bProxy              : �Ƿ�ʹ�ô���(��ֵΪFALSE��ʱ��ǰ����������������)
//		bProxyAuthorization : �����Ƿ���Ҫ��֤
//		lpszProxyUsername   : �����������֤���û���
//		lpszProxyPassword   : �����������֤�Ŀ���
//		nProxyType          : ���������������
//	����ֵ��void
////////////////////////////////////////////////////////////////////////////////
void CHttpDownload::SetProxy(LPCTSTR lpszProxyServer,int nProxyPort,BOOL bProxy/*=TRUE*/,BOOL bProxyAuthorization /*= FALSE*/,LPCTSTR lpszProxyUsername /*= NULL*/,LPCTSTR lpszProxyPassword /*= NULL*/,int nProxyType /*= HTTP_PROXY_HTTPGET*/)
{
	if( bProxy && nProxyType == HTTP_PROXY_USEIE )	// ʹ��IE��Proxy����
	{
		if( !GetIEProxy(m_strProxyServer,m_nProxyPort,m_nProxyType) )
		{
			m_bProxy				= FALSE;
			m_bProxyAuthorization	= FALSE;
			m_nProxyPort			= 0;
			m_nProxyType			= HTTP_PROXY_NONE;
			m_strProxyServer		= _T("");
			m_strProxyUsername		= _T("");
			m_strProxyPassword		= _T("");
		}
		else
		{
			m_bProxy				= TRUE;
			if( lpszProxyUsername != NULL )
			{
				m_bProxyAuthorization	= TRUE;
				m_strProxyUsername		= lpszProxyUsername;
				m_strProxyPassword		= lpszProxyPassword;
			}
			else
			{
				m_bProxyAuthorization	= FALSE;
				m_strProxyUsername		= _T("");
				m_strProxyPassword		= _T("");
			}
			
		}
		return;	// ����
	}

	if( bProxy && lpszProxyServer != NULL)
	{
		m_bProxy			= TRUE;
		m_strProxyServer	= lpszProxyServer;
		m_nProxyPort		= nProxyPort;
		m_nProxyType		= nProxyType;

		if( bProxyAuthorization && lpszProxyUsername != NULL)
		{
			m_bProxyAuthorization	= TRUE;
			m_strProxyUsername		= lpszProxyUsername;
			m_strProxyPassword		= lpszProxyPassword;
		}
		else
		{
			m_bProxyAuthorization	= FALSE;
			m_strProxyUsername		= _T("");
			m_strProxyPassword		= _T("");
		}
	}
	else
	{
		m_bProxy				= FALSE;
		m_bProxyAuthorization	= FALSE;
		m_nProxyPort			= 0;
		m_nProxyType			= HTTP_PROXY_NONE;
		m_strProxyServer		= _T("");
		m_strProxyUsername		= _T("");
		m_strProxyPassword		= _T("");
	}
}


//////////////////////////////////////////////////////////////////////////////////
//	��������void SetAuthorization(
//				LPCTSTR lpszUsername,
//				LPCTSTR lpszPassword,
//				BOOL bAuthorization/* = TRUE*/ ) 
//	��  ;������WWW��֤��Ϣ(���ʱ�������ҳ��ʱ��Ҫ)
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		lpszUsername   : ����ҳ����û���
//		lpszPassword   : ����
//		bAuthorization : �Ƿ���Ҫ��֤(��ֵΪFALSEʱ��ǰ����������������)
//	����ֵ��void
////////////////////////////////////////////////////////////////////////////////
void CHttpDownload::SetAuthorization(LPCTSTR lpszUsername,LPCTSTR lpszPassword,BOOL bAuthorization/* = TRUE*/ )
{
	if( bAuthorization && lpszUsername != NULL )
	{
		m_bAuthorization = TRUE;
		m_strUsername	 = lpszUsername;
		m_strPassword	 = lpszPassword;
	}
	else
	{
		m_bAuthorization = FALSE;
		m_strUsername	 = _T("");
		m_strPassword	 = _T("");
	}
}

//////////////////////////////////////////////////////////////////////////////////
//	��������void SetNotifyWnd(
//				HWND hNotifyWnd,
//				int nNotifyMsg,
//				BOOL bNotify /*= TRUE */ ) 
//	��  ;�������Ƿ���Ҫ������Ϣ�����ô���
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		hNotifyWnd : ������Ϣ�Ĵ���
//		nNotifyMsg : ��ϢID
//		bNotify    : �Ƿ�����Ϣ(��ֵΪFALSEʱ��ǰ����������������)
//	����ֵ��void
////////////////////////////////////////////////////////////////////////////////
void CHttpDownload::SetNotifyWnd(HWND hNotifyWnd, int nNotifyMsg, BOOL bNotify /*= TRUE */)
{
	if( bNotify && (hNotifyWnd != NULL) && ::IsWindow(hNotifyWnd) )
	{
		m_bNotify			= TRUE;
		m_hNotifyWnd		= hNotifyWnd;
		m_nNotifyMessage	= nNotifyMsg;
	}
	else
	{
		m_bNotify			= FALSE;
		m_hNotifyWnd		= NULL;
		m_nNotifyMessage	= 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////
//	��������void SetTimeout(
//				DWORD dwSendTimeout,
//				DWORD dwRecvTimeout,
//				DWORD dwConnTimeout ) 
//	��  ;�����ó�ʱ
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		dwSendTimeout : ���ͳ�ʱ
//		dwRecvTimeout : ���ճ�ʱ
//		dwConnTimeout : ���ӳ�ʱ
//	����ֵ��void
////////////////////////////////////////////////////////////////////////////////
void CHttpDownload::SetTimeout(DWORD dwSendTimeout, DWORD dwRecvTimeout, DWORD dwConnTimeout)
{
	if( dwSendTimeout > 0 )
		m_dwSendTimeout = dwSendTimeout;
	
	if( dwRecvTimeout > 0 )
		m_dwRecvTimeout = dwRecvTimeout;

	if( dwConnTimeout > 0 )
		m_dwConnTimeout = dwConnTimeout;


}


//////////////////////////////////////////////////////////////////////////////////
//	��������void SetUserAgent( LPCTSTR lpszUserAgent ) 
//	��  ;������UserAgent
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		lpszUserAgent : �µ�UserAgent(���磺Mozilla/4.0 (compatible; MSIE 5.0; Windows 98) )
//	����ֵ��void
////////////////////////////////////////////////////////////////////////////////
void CHttpDownload::SetUserAgent(LPCTSTR lpszUserAgent)
{
	if( lpszUserAgent == NULL )
		m_strUserAgent = DEFAULT_USERAGENT;
	else
		m_strUserAgent = lpszUserAgent;	

	if( m_strUserAgent.IsEmpty())
		m_strUserAgent = DEFAULT_USERAGENT;
	
}

//////////////////////////////////////////////////////////////////////////////////
//	��������void SetReferer( LPCTSTR lpszReferer ) 
//	��  ;������Referer(�ύ��)
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		lpszReferer :  �µ�Referer
//	����ֵ��void
////////////////////////////////////////////////////////////////////////////////
void CHttpDownload::SetReferer(LPCTSTR lpszReferer)
{
	if( lpszReferer != NULL )
		m_strReferer = lpszReferer;
	else
		m_strReferer = _T("");
}


//////////////////////////////////////////////////////////////////////////////////
//	��������void SetRetry(
//				int nRetryType,
//				int nRetryDelay,
//				int nRetryMax ) 
//	��  ;���������ԵĻ���
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		nRetryType  :  ���Է�ʽ
//			HTTP_RETRY_NONE  - ������
//			HTTP_RETRY_TIMES - ����һ������
//			HTTP_RETRY_ALWAYS- ��������(����������ѭ��)
//		nRetryDelay : �����ӳ�
//		nRetryMax   : ������Դ���(nRetryTypeΪHTTP_RETRY_TIMESʱ����ֵ��������)
//	����ֵ��void
////////////////////////////////////////////////////////////////////////////////
void CHttpDownload::SetRetry(int nRetryType, int nRetryDelay, int nRetryMax)
{
	m_nRetryType  = nRetryType;
	m_nRetryDelay = nRetryDelay;
	m_nRetryMax	  = nRetryMax;
	
	// ���һ��m_nRetryMax,���Ϊ0����Ϊȱʡֵ
	//if( (HTTP_RETRY_TIMES == m_nRetryType) && (0 == m_nRetryMax) )
	//	m_nRetryMax = DEFAULT_HTTP_RETRY_MAX;
}


//////////////////////////////////////////////////////////////////////////////////
//	��������BOOL GetDownloadFileStatus(
//				IN LPCTSTR lpszDownloadUrl,
//				OUT DWORD &dwFileSize,
//				OUT CTime &FileTime ) 
//	��  ;����ȡ�����ļ���״̬
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		lpszDownloadUrl : �ļ���URL
//		dwFileSize      : ���ڱ����ļ��Ĵ�С
//		FileTime        : ���ڱ����ļ����޸�ʱ��
//	����ֵ��BOOL
////////////////////////////////////////////////////////////////////////////////
BOOL CHttpDownload::GetDownloadFileStatus(IN LPCTSTR lpszDownloadUrl,OUT DWORD &dwFileSize, OUT CTime &FileTime)
{
	// ����Ҫ���ص�URL�Ƿ�Ϊ��
	m_strDownloadUrl = lpszDownloadUrl;
	m_strDownloadUrl.TrimLeft();
	m_strDownloadUrl.TrimRight();
	if( m_strDownloadUrl.IsEmpty() )
		return FALSE;

	// ����Ҫ���ص�URL�Ƿ���Ч
	if ( !ParseURL(m_strDownloadUrl, m_strServer, m_strObject, m_nPort))
	{
		// ��ǰ�����"http://"����
		m_strDownloadUrl = _T("http://") + m_strDownloadUrl;
		if ( !ParseURL(m_strDownloadUrl, m_strServer, m_strObject, m_nPort) )
		{
			TRACE(_T("Failed to parse the URL: %s\n"), m_strDownloadUrl);
			return FALSE;
		}
	}

	m_strSavePath		= "|";
	m_strTempSavePath	= "|";
	m_bStopDownload		= FALSE;

	if ( SendRequest() !=  HTTP_REQUEST_SUCCESS )
		return FALSE;

	dwFileSize  = m_dwDownloadSize;
	FileTime	= m_TimeLastModified;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////
//	��������BOOL ParseURL(
//				IN LPCTSTR lpszURL,
//				OUT CString &strServer,
//				OUT CString &strObject,
//				OUT int& nPort ) 
//	��  ;����URL�����ֳ�Server��Object��
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		lpszURL   : ����ֵ�URL
//		strServer : ��������ַ
//		strObject : �������ϵ�Ŀ���ļ�
//		nPort     : �������Ķ˿�
//	����ֵ��BOOL
////////////////////////////////////////////////////////////////////////////////
BOOL CHttpDownload::ParseURL(IN LPCTSTR lpszURL, OUT CString &strServer, OUT CString &strObject,OUT int& nPort)
{
	CString strURL(lpszURL);
	strURL.TrimLeft();
	strURL.TrimRight();
	
	// �������
	strServer = _T("");
	strObject = _T("");
	nPort	  = 0;

	int nPos = strURL.Find("://");
	if( nPos == -1 )
		return FALSE;

	// ��һ����֤�Ƿ�Ϊhttp://
	CString strTemp = strURL.Left( nPos+lstrlen("://") );
	strTemp.MakeLower();
	if( strTemp.Compare("http://") != 0 )
		return FALSE;

	strURL = strURL.Mid( strTemp.GetLength() );
	nPos = strURL.Find('/');
	if ( nPos == -1 )
		return FALSE;

	strObject = strURL.Mid(nPos);
	strTemp   = strURL.Left(nPos);
	
	///////////////////////////////////////////////////////////////
	/// ע�⣺��û�п���URL�����û����Ϳ�������κ������#������
	/// ���磺http://abc@def:www.yahoo.com:81/index.html#link1
	/// 
	//////////////////////////////////////////////////////////////

	// �����Ƿ��ж˿ں�
	nPos = strTemp.Find(":");
	if( nPos == -1 )
	{
		strServer = strTemp;
		nPort	  = DEFAULT_HTTP_PORT;
	}
	else
	{
		strServer = strTemp.Left( nPos );
		strTemp	  = strTemp.Mid( nPos+1 );
		nPort	  = (int)_ttoi((LPCTSTR)strTemp);
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////
//	��������int GetInfo(
//				IN LPCTSTR lpszHeader,
//				OUT DWORD& dwContentLength,
//				OUT DWORD& dwStatusCode,
//				OUT CTime& TimeLastModified,
//				OUT CString& strCookie,
//				LPTSTR lpszFtpURL ) 
//	��  ;���ӷ��ص�HTTPЭ��ͷ���ñ�Ҫ����Ϣ
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		lpszHeader       : HTTPЭ��ͷ
//		dwContentLength  : content-length�ֶε�ֵ
//		dwStatusCode     : ״̬��
//		TimeLastModified : last-modified�ֶε�ֵ
//		strCookie        : ��������Cookie
//		lpszFtpURL       : ���������ض���FTP��URL
//	����ֵ��int
//		HTTP_FAIL		: ���Ϸ���HTTPЭ��ͷ��ʧ��
//		HTTP_REDIRECT	: �ض���
//		HTTP_SUCCESS	: �ɹ�
////////////////////////////////////////////////////////////////////////////////
int CHttpDownload::GetInfo(IN LPCTSTR lpszHeader,OUT DWORD& dwContentLength,OUT DWORD& dwStatusCode,OUT CTime& TimeLastModified,OUT CString& strCookie,LPTSTR lpszFtpURL)
{
	dwContentLength = 0;
	dwStatusCode	= 0;
	TimeLastModified= CTime::GetCurrentTime();
	strCookie		= _T("");

	CString strHeader(lpszHeader);
	strHeader.MakeLower();

	//��ֳ�HTTPӦ���ͷ��Ϣ�ĵ�һ��
	int nPos = strHeader.Find("\r\n");
	if (nPos == -1)
		return HTTP_FAIL;

	// ��÷�����: Status Code
	CString strStatusLine = strHeader.Left(nPos);
	strStatusLine.TrimLeft();
	strStatusLine.TrimRight();

	nPos = strStatusLine.Find(' ');
	if( nPos == -1 )
		return HTTP_FAIL;
	strStatusLine = strStatusLine.Mid(nPos+1);
	nPos = strStatusLine.Find(' ');
	if( nPos == -1 )
		return HTTP_FAIL;
	strStatusLine = strStatusLine.Left(nPos);
	dwStatusCode = (DWORD)_ttoi((LPCTSTR)strStatusLine);

	// ��鷵����
	if( dwStatusCode >= 300 && dwStatusCode < 400 ) //���ȼ��һ�·�������Ӧ���Ƿ�Ϊ�ض���
	{
		nPos = strHeader.Find("location:");
		if (nPos == -1)
			return HTTP_FAIL;

		CString strRedirectURL = strHeader.Mid(nPos + strlen("location:"));
		nPos = strRedirectURL.Find("\r\n");
		if (nPos == -1)
			return HTTP_FAIL;

		strRedirectURL = strRedirectURL.Left(nPos);
		strRedirectURL.TrimLeft();
		strRedirectURL.TrimRight();
		
		// ����Referer
		m_strReferer = m_strDownloadUrl;

		// �ж��Ƿ��ض��������ķ�����
		// �Ӱ汾1.7��ʼ��֧���ض���FTP������
		if( strRedirectURL.Mid(0,6) == _T("ftp://") )
		{
			if( lpszFtpURL != NULL )
				memcpy( lpszFtpURL,(LPCTSTR)strRedirectURL,strRedirectURL.GetLength() );

			return HTTP_REDIRECT_FTP;
		}

		if( strRedirectURL.Mid(0,7) == _T("http://") )
		{
			m_strDownloadUrl = strRedirectURL;
			// ����Ҫ���ص�URL�Ƿ���Ч
			if ( !ParseURL(m_strDownloadUrl, m_strServer, m_strObject, m_nPort))
				return HTTP_FAIL;
			return HTTP_REDIRECT;
		}

		// �ض��򵽱��������������ط�
		strRedirectURL.Replace("\\","/");
		
		// ������ڸ�Ŀ¼
		if( strRedirectURL[0] == '/' )
		{
			m_strObject = strRedirectURL;
			return HTTP_REDIRECT;
		}
		
		// ����Ե�ǰĿ¼
		int nParentDirCount = 0;
		nPos = strRedirectURL.Find("../");
		while (nPos != -1)
		{
			strRedirectURL = strRedirectURL.Mid(nPos+3);
			nParentDirCount++;
			nPos = strRedirectURL.Find("../");
		}
		for (int i=0; i<=nParentDirCount; i++)
		{
			nPos = m_strDownloadUrl.ReverseFind('/');
			if (nPos != -1)
				m_strDownloadUrl = m_strDownloadUrl.Left(nPos);
		}
		m_strDownloadUrl = m_strDownloadUrl+"/"+strRedirectURL;

		if ( !ParseURL(m_strDownloadUrl, m_strServer, m_strObject, m_nPort))
			return HTTP_FAIL;
		return HTTP_REDIRECT;
	}

	// �������󣬲�������(�����Ƿ����������ǿͻ��˵�)
	if( dwStatusCode >=400 )
		return HTTP_FAIL;
		
	// ��ȡContentLength(���û�У��ͷ���0����ʾ����δ֪)
	nPos = strHeader.Find("content-length:");
	if( nPos != -1 )
	{
		CString strConentLen = strHeader.Mid(nPos + strlen("content-length:"));	
		nPos = strConentLen.Find("\r\n");
		if (nPos != -1)
		{
			strConentLen = strConentLen.Left(nPos);	
			strConentLen.TrimLeft();
			strConentLen.TrimRight();

			// Content-Length:
			dwContentLength = (DWORD) _ttoi( (LPCTSTR)strConentLen );
		}
	}

	// ��ȡLast-Modified:
	nPos = strHeader.Find("last-modified:");
	if (nPos != -1)
	{
		CString strTime = strHeader.Mid(nPos + strlen("last-modified:"));
		nPos = strTime.Find("\r\n");
		if (nPos != -1)
		{
			strTime = strTime.Left(nPos);
			strTime.TrimLeft();
			strTime.TrimRight();
			TimeLastModified = GetTime(strTime);
		}
	}

	// ��ȡCookie
	nPos = strHeader.Find("set-cookie:");
	if (nPos != -1)
	{
		CString strText = strHeader.Mid(nPos + strlen("set-cookie:"));
		nPos = strText.Find("\r\n");
		if (nPos != -1)
		{
			strCookie = strText.Left(nPos);
			strCookie.TrimLeft();
			strCookie.TrimRight();
			
			int nPos1 = strCookie.Find("expires=");
			int nPos2 = strCookie.Find("domain=");
			int nPos3 = strCookie.Find("path=");
			
			nPos = -1;
			if( nPos1 != -1 )
				nPos = nPos1;

			if( nPos2 != -1 )
				nPos = ( nPos == -1 )? nPos2:((nPos > nPos2 )?nPos2:nPos);

			if( nPos3 != -1 )
				nPos = ( nPos == -1 )? nPos3:((nPos > nPos3 )?nPos3:nPos);
				
			if( nPos != -1 )
				strCookie = strCookie.Left( nPos );

			// ɾ������;
			if( strCookie.Right(1) == ";" )
				strCookie = strCookie.Left( strCookie.GetLength() -1 );
		}
			
	}
	return HTTP_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	��������CTime GetTime( LPCTSTR lpszTime ) 
//	��  ;����GMT�ַ���ת����ʱ��
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		lpszTime : ʱ���ַ���
//	����ֵ��CTime
////////////////////////////////////////////////////////////////////////////////
CTime CHttpDownload::GetTime(LPCTSTR lpszTime)
{
	int nDay,nMonth,nYear,nHour,nMinute,nSecond;

	CString strTime = lpszTime;
	int nPos = strTime.Find(',');
	if (nPos != -1)
	{
		strTime = strTime.Mid(nPos+1);
		strTime.TrimLeft();

		CString strDay,strMonth,strYear,strHour,strMinute,strSecond;
		CString strAllMonth = "jan,feb,mar,apr,may,jun,jul,aug,sep,oct,nov,dec";
		strDay = strTime.Left(2);
		nDay = atoi(strDay);
		strMonth = strTime.Mid(3,3);
		strMonth.MakeLower();
		nPos = strAllMonth.Find(strMonth);
		if (nPos != -1)
		{
			strMonth.Format("%d",((nPos/4)+1));
			nMonth = atoi(strMonth);
		}
		else
			nMonth = 1;
		strTime = strTime.Mid(6);
		strTime.TrimLeft();
		nPos = strTime.FindOneOf(" \t");
		if (nPos != -1)
		{
			strYear = strTime.Left(nPos);
			nYear = atoi(strYear);
		}
		else
			nYear = 2000;
		strTime = strTime.Mid(nPos+1);
		strHour = strTime.Left(2);
		nHour = atoi(strHour);
		strMinute = strTime.Mid(3,2);
		nMinute = atoi(strMinute);
		strSecond = strTime.Mid(6,2);
		nSecond = atoi(strSecond);
	}
	
	CTime time(nYear,nMonth,nDay,nHour,nMinute,nSecond);
	return time;
}

//////////////////////////////////////////////////////////////////////////////////
//	��������void GetFileName()
//	��  ;��������URL�л�ȡ�ļ���
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ������
//	����ֵ��void
////////////////////////////////////////////////////////////////////////////////
void CHttpDownload::GetFileName()
{
	// ��ȡ���ļ���
	int nSlash = m_strObject.ReverseFind(_T('/'));
	if (nSlash == -1)
		nSlash = m_strObject.ReverseFind(_T('\\'));

	if (nSlash != -1 && m_strObject.GetLength() > 1)
		m_strFileName = m_strObject.Right(m_strObject.GetLength() - nSlash - 1);
	else
		m_strFileName = m_strObject;

	m_strFileName.TrimLeft();
	m_strFileName.TrimRight();
	// �ļ���Ϊ�ջ���Ч
	if( m_strFileName.IsEmpty() || !IsValidFileName(m_strFileName) )
		m_strFileName = DEFAULT_SAVE_FILE;
}

//////////////////////////////////////////////////////////////////////////////////
//	��������void StopDownload()
//	��  ;����ֹ���� 
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ������
//	����ֵ��void
////////////////////////////////////////////////////////////////////////////////
void CHttpDownload::StopDownload()
{
	m_bStopDownload = TRUE;

	if ( m_hStopEvent != NULL )
		SetEvent(m_hStopEvent);
}


//////////////////////////////////////////////////////////////////////////////////
//	��������int Base64Decode(
//				IN LPCTSTR lpszDecoding,
//				OUT CString &strDecoded ) 
//	��  ;��BASE64����
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		lpszDecoding : �������ַ���
//		strDecoded   : ������
//	����ֵ��int�������ĳ���
////////////////////////////////////////////////////////////////////////////////
int CHttpDownload::Base64Decode(IN LPCTSTR lpszDecoding, OUT CString &strDecoded)
{
	int nIndex =0;
	int nDigit;
    int nDecode[ 256 ];
	int nSize;
	int nNumBits = 6;

	if( lpszDecoding == NULL )
		return 0;
	
	if( ( nSize = lstrlen(lpszDecoding) ) == 0 )
		return 0;

	// Build Decode Table
	for( int i = 0; i < 256; i++ ) 
		nDecode[i] = -2; // Illegal digit
	for( i=0; i < 64; i++ )
	{
		nDecode[ m_strBase64TAB[ i ] ] = i;
		nDecode[ '=' ] = -1; 
    }

	// Clear the output buffer
	strDecoded = _T("");
	long lBitsStorage  =0;
	int nBitsRemaining = 0;
	int nScratch = 0;
	UCHAR c;
	
	// Decode the Input
	for( nIndex = 0, i = 0; nIndex < nSize; nIndex++ )
	{
		c = lpszDecoding[ nIndex ];

		// �������в��Ϸ����ַ�
		if( c> 0x7F)
			continue;

		nDigit = nDecode[c];
		if( nDigit >= 0 ) 
		{
			lBitsStorage = (lBitsStorage << nNumBits) | (nDigit & 0x3F);
			nBitsRemaining += nNumBits;
			while( nBitsRemaining > 7 ) 
			{
				nScratch = lBitsStorage >> (nBitsRemaining - 8);
				strDecoded += (nScratch & 0xFF);
				i++;
				nBitsRemaining -= 8;
			}
		}
    }	

	return strDecoded.GetLength();
}


//////////////////////////////////////////////////////////////////////////////////
//	��������int Base64Encode(
//				IN LPCTSTR lpszEncoding,
//				OUT CString &strEncoded ) 
//	��  ;��BASE64����
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		lpszEncoding : ��������ַ���
//		strEncoded   : ������
//	����ֵ��int�������ĳ���
////////////////////////////////////////////////////////////////////////////////
int CHttpDownload::Base64Encode(IN LPCTSTR lpszEncoding, OUT CString &strEncoded)
{
	int nDigit;
	int nNumBits = 6;
	int nIndex = 0;
	int nInputSize;

	strEncoded = _T( "" );
	if( lpszEncoding == NULL )
		return 0;

	if( ( nInputSize = lstrlen(lpszEncoding) ) == 0 )
		return 0;

	int nBitsRemaining = 0;
	long lBitsStorage	=0;
	long lScratch		=0;
	int nBits;
	UCHAR c;

	while( nNumBits > 0 )
	{
		while( ( nBitsRemaining < nNumBits ) &&  ( nIndex < nInputSize ) ) 
		{
			c = lpszEncoding[ nIndex++ ];
			lBitsStorage <<= 8;
			lBitsStorage |= (c & 0xff);
			nBitsRemaining += 8;
		}
		if( nBitsRemaining < nNumBits ) 
		{
			lScratch = lBitsStorage << ( nNumBits - nBitsRemaining );
			nBits    = nBitsRemaining;
			nBitsRemaining = 0;
		}	 
		else 
		{
			lScratch = lBitsStorage >> ( nBitsRemaining - nNumBits );
			nBits	 = nNumBits;
			nBitsRemaining -= nNumBits;
		}
		nDigit = (int)(lScratch & m_nBase64Mask[nNumBits]);
		nNumBits = nBits;
		if( nNumBits <=0 )
			break;
		
		strEncoded += m_strBase64TAB[ nDigit ];
	}
	// Pad with '=' as per RFC 1521
	while( strEncoded.GetLength() % 4 != 0 )
		strEncoded += '=';

	return strEncoded.GetLength();

}


//////////////////////////////////////////////////////////////////////////////////
//	��������int MakeConnection(
//				CBufSocket* pBufSocket,
//				CString strServer,
//				int nPort ) 
//	��  ;����������
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		pBufSocket : CBufSocket��ָ��
//		strServer  : �����ӷ�����
//		nPort      : �˿�
//	����ֵ��int
////////////////////////////////////////////////////////////////////////////////
int CHttpDownload::MakeConnection(CBufSocket* pBufSocket,CString strServer,int nPort)
{
	CSocksPacket	cSocks( pBufSocket );
	DWORD			dwIP;
	BYTE			byAuth,byAtyp;
	SOCKSREPPACKET	pack;
	CString			strAuth,strAddr;

	strAuth = _T("");
	strAddr = _T("");

	switch( m_nProxyType )
	{
	case HTTP_PROXY_NONE:
		Check( m_bStopDownload,HTTP_REQUEST_STOP);
		Check( !pBufSocket->Connect(strServer,nPort,m_dwConnTimeout,TRUE),HTTP_REQUEST_ERROR);
		break;
	case HTTP_PROXY_HTTPGET:
		Check( m_bStopDownload,HTTP_REQUEST_STOP);
		Check( !pBufSocket->Connect(m_strProxyServer,m_nProxyPort,m_dwConnTimeout,TRUE),HTTP_REQUEST_ERROR);
		break;
	case HTTP_PROXY_SOCKS4A:
		dwIP = CBufSocket::GetIP(strServer,TRUE);
		if( dwIP == INADDR_NONE )
		{
			Check( m_bStopDownload,HTTP_REQUEST_STOP);
			Check( !pBufSocket->Connect(m_strProxyServer,m_nProxyPort,m_dwConnTimeout,TRUE),HTTP_REQUEST_ERROR);
			Check( m_bStopDownload,HTTP_REQUEST_STOP);
			Check( !cSocks.SendSocks4aReq(CMD_CONNECT,nPort,strServer,m_strProxyUsername,m_dwSendTimeout),HTTP_REQUEST_ERROR);

			Check( m_bStopDownload,HTTP_REQUEST_STOP);
			Check( !cSocks.RecvPacket(PACKET_SOCKS4AREP,m_dwRecvTimeout),HTTP_REQUEST_ERROR);
			Check( !cSocks.IsSocksOK(),HTTP_REQUEST_FAIL);	// �����д������Կ�����û���õ�
			break;// NOTICE:��������ܹ���������������ʹ��SOCKS4 Proxy
		}
	case HTTP_PROXY_SOCKS4:
		// ����Ҫ�õ�Proxy Server��IP��ַ(����Ϊ����)
		dwIP = CBufSocket::GetIP(strServer,TRUE);
		Check( dwIP == INADDR_NONE,HTTP_REQUEST_ERROR);

		Check( m_bStopDownload,HTTP_REQUEST_STOP);
		Check( !pBufSocket->Connect(m_strProxyServer,m_nProxyPort,m_dwConnTimeout,TRUE),HTTP_REQUEST_ERROR);
		Check( m_bStopDownload,HTTP_REQUEST_STOP);
		Check( !cSocks.SendSocks4Req(CMD_CONNECT,nPort,dwIP,m_strProxyUsername,m_dwSendTimeout),HTTP_REQUEST_ERROR);

		Check( m_bStopDownload,HTTP_REQUEST_STOP);
		Check( !cSocks.RecvPacket(PACKET_SOCKS4REP,m_dwRecvTimeout),HTTP_REQUEST_ERROR);
		Check( !cSocks.IsSocksOK(),HTTP_REQUEST_FAIL); // �����д������Կ�����û���õ�
		break;

	case HTTP_PROXY_SOCKS5:

		Check( m_bStopDownload,HTTP_REQUEST_STOP);
		Check( !pBufSocket->Connect(m_strProxyServer,m_nProxyPort,m_dwConnTimeout,TRUE),HTTP_REQUEST_ERROR);

		if( m_bProxyAuthorization )
		{
			char ch	= (char)AUTH_NONE;
			strAuth += ch;
			ch 		= (char)AUTH_PASSWD;
			strAuth += ch;
		}
		else
		{
			char ch	= (char)AUTH_NONE;
			strAuth += ch;
		}
		byAuth =(BYTE)strAuth.GetLength();
			
		Check( m_bStopDownload,HTTP_REQUEST_STOP);
		Check( !cSocks.SendSocks5AuthReq(byAuth,(LPCTSTR)strAuth,m_dwSendTimeout),HTTP_REQUEST_ERROR);
		
		Check( m_bStopDownload,HTTP_REQUEST_STOP);

		ZeroMemory(&pack,sizeof(SOCKSREPPACKET));
		Check( !cSocks.RecvPacket(&pack,PACKET_SOCKS5AUTHREP,m_dwRecvTimeout),HTTP_REQUEST_ERROR);
		Check( !cSocks.IsSocksOK(&pack,PACKET_SOCKS5AUTHREP),HTTP_REQUEST_FAIL);	// �����д������Կ�����û���õ�

		switch( pack.socks5AuthRep.byAuth )
		{
		case AUTH_NONE:
			break;
		case AUTH_PASSWD:
			
			Check( !m_bProxyAuthorization,HTTP_REQUEST_FAIL);
			Check( m_bStopDownload,HTTP_REQUEST_STOP);
			Check( !cSocks.SendSocks5AuthPasswdReq(m_strProxyUsername,m_strProxyPassword,m_dwSendTimeout),HTTP_REQUEST_ERROR);
			Check( m_bStopDownload,HTTP_REQUEST_STOP);
			
			Check( !cSocks.RecvPacket(PACKET_SOCKS5AUTHPASSWDREP,m_dwRecvTimeout),HTTP_REQUEST_ERROR);
			Check( !cSocks.IsSocksOK(),HTTP_REQUEST_FAIL); // �����д������Կ�����û���õ�
			break;
		case AUTH_GSSAPI:
		case AUTH_CHAP:
		case AUTH_UNKNOWN:
		default:
			return HTTP_REQUEST_FAIL;
			break;
		}
		dwIP = CBufSocket::GetIP(strServer,TRUE);
		if( dwIP != INADDR_NONE )
		{
			byAtyp = ATYP_IPV4ADDR;
			//����ת���ֽ���
			strAddr += (char)( (dwIP    ) &0x000000ff) ; 
			strAddr += (char)( (dwIP>>8 ) &0x000000ff); 
			strAddr += (char)( (dwIP>>16) &0x000000ff); 
			strAddr += (char)( (dwIP>>24) &0x000000ff); 

		}
		else
		{
			byAtyp = ATYP_HOSTNAME;
			char ch = (char)strServer.GetLength();
			strAddr += ch;
			strAddr += strServer;
		}

		Check( m_bStopDownload,HTTP_REQUEST_STOP);
		Check( !cSocks.SendSocks5Req(CMD_CONNECT,byAtyp,strAddr,nPort,m_dwSendTimeout),HTTP_REQUEST_ERROR);
		Check( m_bStopDownload,HTTP_REQUEST_STOP);
		Check( !cSocks.RecvPacket(PACKET_SOCKS5REP,m_dwRecvTimeout),HTTP_REQUEST_ERROR);
		Check( !cSocks.IsSocksOK(),HTTP_REQUEST_FAIL); // �����д������Կ�����û���õ�
		break;
	case HTTP_PROXY_HTTPCONNECT:
	default:
		return HTTP_REQUEST_FAIL;
		break;
	}
	return HTTP_REQUEST_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	��������BOOL GetIEProxy(
//				CString &strProxyServer,
//				int &nProxyPort,
//				int &nProxyType ) 
//	��  ;����ȡIE��Proxy����
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		strProxyServer : �����������ַ
//		nProxyPort     : ����������Ķ˿�
//		nProxyType     : ���������������
//	����ֵ��BOOL
////////////////////////////////////////////////////////////////////////////////
BOOL CHttpDownload::GetIEProxy(CString &strProxyServer, int &nProxyPort, int &nProxyType)
{
	// ��ȡע���
	HKEY hRegKey;
	DWORD dwRet = RegOpenKeyEx(
		HKEY_CURRENT_USER,
		"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
		0L,
		KEY_QUERY_VALUE,&hRegKey);
	if (dwRet != ERROR_SUCCESS)
		return FALSE;
	
	TCHAR szAddr[256]	= { 0 };
	DWORD dwLen			= 256;
	dwRet = RegQueryValueEx(hRegKey,"ProxyServer",NULL,NULL,(LPBYTE)szAddr,&dwLen);
	if (dwRet != ERROR_SUCCESS)
	{
		RegCloseKey(hRegKey);
		return FALSE;
	}
    RegCloseKey(hRegKey);
    
    // ע�����ܴ������д���ͬһ���������ʱ�� http=...ftp=...��Ϣ
    //    comment by Linsuyi, 2002/01/22  23:30
    // 
    // ����Proxy������
    //http=193.168.10.1:80;socks=192.168.10.235:1080
    //193.168.10.1:1090
    //ftp=193.168.10.1:1090;gopher=193.168.10.1:1090;https=193.168.10.1:1090;socks=193.168.10.2:1080
    int nPos = -1;
    CString strProxy = szAddr;
    
    if (strProxy.IsEmpty())
        return FALSE;
    
    strProxy.MakeLower();
    nProxyType = HTTP_PROXY_HTTPGET;
    
    nPos = strProxy.Find(';');
    if (nPos != -1)
    {
        nPos = strProxy.Find("http=");        // HTTP GET Proxy
        if (nPos != -1)
            strProxy = strProxy.Mid(nPos+strlen("http="));
        else if ((nPos = strProxy.Find("socks=")) != -1) // SOCKS Proxy ( SOCKS5 )
        {
            strProxy = strProxy.Mid( nPos+strlen("socks="));
            nProxyType = HTTP_PROXY_SOCKS5;
        }
        nPos = strProxy.Find(";");
        if (nPos != -1)
    	    strProxy = strProxy.Left(nPos);
    }
    
    nPos = strProxy.Find(":");
    strProxyServer = strProxy.Left(nPos);
    nProxyPort = _ttoi(strProxy.Mid(nPos+1));
    if (nProxyPort <= 0)
        return FALSE;
    
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////
//	��������void void SetCookie(
//				LPCTSTR lpszCookie,
//				BOOL bUseIECookie /*= FALSE*/ ) 
//	��  ;������Cookie
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		lpszCookie   : Cookieֵ 
//		bUseIECookie : �Ƿ�ʹ��IE��Cookieֵ
//	����ֵ��void
////////////////////////////////////////////////////////////////////////////////
void CHttpDownload::SetCookie(LPCTSTR lpszCookie,BOOL bUseIECookie /*= FALSE*/ )
{
	if( bUseIECookie )
	{
		m_bUseIECookie = TRUE;
/*
		CString strCookie;
		if( GetIECookie(strCookie,m_strDownloadUrl) )
			m_strCookie = strCookie;
		else
			m_strCookie = _T("");
*/

	}
	else
	{
		m_bUseIECookie = FALSE;

		if( lpszCookie != NULL)
			m_strCookie = lpszCookie;
		else
			m_strCookie = _T("");
	}
}

//////////////////////////////////////////////////////////////////////////////////
//	��������BOOL GetIECookie(
//				CString& strCookie,
//				LPCTSTR lpszURL )
//	��  ;����ȡIE��Cookie����
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		strCookie  : ���ڱ���Cookie
//		lpszURL    : ������URL
//	����ֵ��BOOL
////////////////////////////////////////////////////////////////////////////////
BOOL CHttpDownload::GetIECookie(CString& strCookie,LPCTSTR lpszURL )
{
/*
	Cookies Folder:
		HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders\ 
		Cookies = ***

	The simpliest way to Get Cookie value is call :
		CInternetSession::GetCookie(***,***,strCookie);
	But this way need the Wininet.dll which i don't want to use,How can i do?
	OR: You can call :
		CInternetSession::GetCookie(***,***,strCookie);
	before you use HttpDownload,and then call SetCookie(***);
*/
	strCookie = _T("");
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//	��������BOOL IsValidFileName( LPCTSTR lpszFileName ) 
//	��  ;���ж��ļ����Ƿ�Ϸ�
//	��ȫ�ֱ�����Ӱ�죺��
//	��  ����
//		lpszFileName : �ļ���
//	����ֵ��BOOL
////////////////////////////////////////////////////////////////////////////////
BOOL CHttpDownload::IsValidFileName(LPCTSTR lpszFileName)
{
/*
	Short and long file names must not contain the following characters:
	\   ?   |   >  <   :  /  *  "
	In addition, short file names must not contain the following characters:
	+  ,  ;  =  [  ] 
	Short file names may not include a space, although a long file name may. 
	No space is allowed preceding the vertical bar (|) separator for the short 
	file name/long file name syntax. If a space exists after the separator, then 
	the long file name must have a space at the beginning of the file name. No 
	full-path syntax is allowed.
*/
	CString strFileName(lpszFileName);
	if( strFileName.FindOneOf("\\?|></\"*:") != -1 )
		return FALSE;

	return TRUE;
}

//seawind
BOOL CHttpDownload::IsUserStop()
{
	ASSERT(m_hStopEvent != NULL);

	if (WaitForSingleObject(m_hStopEvent, 0) == WAIT_OBJECT_0)
		return TRUE;
	else
		return FALSE;
}
