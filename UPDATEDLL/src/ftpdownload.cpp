
///////////////////////////////////////////////////////////////////////////////
// FtpDownload.cpp: implementation of the CFtpDownload class.
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SocksPacket.h"
#include "FtpDownload.h"
#include "Resource.h"

#define READ_BUFFER_SIZE 10240
#define DEFAULT_SAVE_DIR _T("C:\\")
#define Check(a,b) if((a)) return (b);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFtpDownload::CFtpDownload()
{
	m_strDownloadUrl	= _T("");
	m_strSavePath		= _T("");		// ������ȫ·���򱣴�Ŀ¼	
	m_strTempSavePath	= _T("");		//��ʱ�ļ���·��

	// ֹͣ����
	m_bStopDownload		= FALSE;
	m_hStopEvent		= NULL;

	// ǿ����������(�������е��ļ��Ƿ���Զ���ļ���ͬ)
	m_bForceDownload	= FALSE;

	// �Ƿ�֧�ֶϵ�����
	m_bSupportResume	= FALSE;

	// PASV��ʽ
	m_bPasv				= TRUE;

	// �ļ��Լ����ش�С
	m_dwFileSize			= 0;		// �ļ��ܵĴ�С	
	m_dwFileDownloadedSize	= 0;		// �ļ��ܹ��Ѿ����صĴ�С

	m_dwDownloadSize	= 0;			// ������Ҫ���صĴ�С
	m_dwDownloadedSize	= 0;			// �����Ѿ����صĴ�С


	// ��ʱTIMEOUT	���ӳ�ʱ�����ͳ�ʱ�����ճ�ʱ(��λ������)
	m_dwConnTimeout	= FTP_CONN_TIMEOUT;	
	m_dwRecvTimeout	= FTP_RECV_TIMEOUT;
	m_dwSendTimeout		= FTP_SEND_TIMEOUT;

	// ���Ի���
	m_nRetryType		= FTP_RETRY_NONE;	//��������(0:������ 1:����һ������ 2:��������)
	m_nRetryTimes		= 0;				//���Դ���
	m_nRetryDelay		= 0;				//�����ӳ�(��λ������)
	m_nRetryMax			= 0;				//���Ե�������

	// ������
	m_nErrorCount		= 0;				//������� (��ʱû������)
	m_strError			= _T("");			//������Ϣ (��ʱû������)


	// ���������ڷ�����Ϣ
	m_bNotify			= FALSE;			// �Ƿ����ⷢ��֪ͨ��Ϣ	
	m_hNotifyWnd		= NULL;				// ��֪ͨ�Ĵ���
	m_nNotifyMessage	= 0;				// ��֪ͨ����Ϣ
    
	// �Ƿ�ʹ�ô��� 
	m_bProxy			= FALSE;
	m_strProxyServer	= _T("");
	m_nProxyPort		= 0;
	m_nProxyType		= FTP_PROXY_NONE;
	
	// �����Ƿ���Ҫ��֤
	m_bProxyAuthorization	= FALSE;
	m_strProxyUsername		= _T("");
	m_strProxyPassword		= _T("");

	// FTP�û��Ϳ���
	m_bAuthorization		= FALSE;
	m_strUsername			= _T("anonymous");
	m_strPassword			= _T("mail@unknow.com");

	// ���ع��������õı���
	m_strServer			= _T("");
	m_strObject			= _T("");
	m_strFileName		= _T("");
	m_nPort				= DEFAULT_FTP_PORT;

}

CFtpDownload::~CFtpDownload()
{
	CloseControlSocket();
	CloseDataSocket();
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
BOOL CFtpDownload::GetIEProxy(CString &strProxyServer, int &nProxyPort, int &nProxyType)
{
    // ��ȡע���
    HKEY hRegKey;
    DWORD dwRet = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
        0L,
        KEY_QUERY_VALUE,&hRegKey
    );
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
    nProxyType = FTP_PROXY_NONE;
       
    nPos = strProxy.Find("socks=");
    if (-1 == nPos)
        return FALSE;
    
    strProxy = strProxy.Mid( nPos+strlen("socks="));
    nProxyType = FTP_PROXY_SOCKS5;
    
    nPos = strProxy.Find(";");
    if (nPos != -1)
    	strProxy = strProxy.Left(nPos);
    
    nPos = strProxy.Find(":");
    strProxyServer = strProxy.Left(nPos);
    nProxyPort = _ttoi(strProxy.Mid(nPos+1));
    if (nProxyPort <= 0)
        return FALSE;
    
    return TRUE;
}

// ���ô���
void CFtpDownload::SetProxy(LPCTSTR lpszProxyServer,int nProxyPort, BOOL bProxy/*=TRUE*/, BOOL bProxyAuthorization /*=FALSE*/, LPCTSTR lpszProxyUsername /*=NULL*/, LPCTSTR lpszProxyPassword /*=NULL*/, int nProxyType /*=FTP_PROXY_NONE*/)
{
    if (bProxy && (FTP_PROXY_USEIE == nProxyType))
    {
        if (!GetIEProxy(m_strProxyServer,m_nProxyPort,m_nProxyType))
        {
            m_bProxy                = FALSE;
            m_bProxyAuthorization   = FALSE;
            m_nProxyPort            = 0;
            m_nProxyType            = FTP_PROXY_NONE;
            m_strProxyServer        = _T("");
            m_strProxyUsername      = _T("");
            m_strProxyPassword      = _T("");
        }
        else
        {
            m_bProxy                = TRUE;
            
            if (lpszProxyUsername != NULL)
            {
                m_bProxyAuthorization   = TRUE;
                m_strProxyUsername      = lpszProxyUsername;
                m_strProxyPassword      = lpszProxyPassword;
            }
            else
            {
                m_bProxyAuthorization   = FALSE;
                m_strProxyUsername      = _T("");
                m_strProxyPassword      = _T("");
            }
        }
    }
    else if (bProxy && (lpszProxyServer != NULL) && (nProxyPort != 0))
    {
        m_bProxy			= TRUE;
        m_strProxyServer	= lpszProxyServer;
        m_nProxyPort		= nProxyPort;
        m_nProxyType		= nProxyType;
        
        if (bProxyAuthorization && lpszProxyUsername != NULL)
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
        
        m_nProxyType			= FTP_PROXY_NONE;
        m_strProxyServer		= _T("");
        m_strProxyUsername		= _T("");
        m_strProxyPassword		= _T("");
    }
}

// �����Ƿ���Ҫ������Ϣ�����ô���
void CFtpDownload::SetNotifyWnd(HWND hNotifyWnd, int nNotifyMsg, BOOL bNotify)
{
    if (bNotify && (hNotifyWnd != NULL) && ::IsWindow(hNotifyWnd) )
    {
        m_bNotify	 = TRUE;
        m_hNotifyWnd = hNotifyWnd;
        m_nNotifyMessage = nNotifyMsg;
    }
    else
    {
        m_bNotify	 = FALSE;
        m_hNotifyWnd = NULL;
        m_nNotifyMessage = 0;
    }
}

// ���ó�ʱ
void CFtpDownload::SetTimeout(DWORD dwSendTimeout, DWORD dwRecvTimeout, DWORD dwConnTimeout)
{
	if( dwSendTimeout > 0 )
		m_dwSendTimeout = dwSendTimeout;
	
	if( dwRecvTimeout > 0 )
		m_dwRecvTimeout = dwRecvTimeout;

	if( dwConnTimeout > 0 )
		m_dwConnTimeout = dwConnTimeout;


}
// �������ԵĻ���
// nRetryType = 0  ������					FTP_RETRY_NONE
// nRetryType = 1  ����һ������				FTP_RETRY_TIMES
// nRetryType = 2  ��Զ����(����������ѭ��)	FTP_RETRY_ALWAYS
void CFtpDownload::SetRetry(int nRetryType, int nRetryDelay, int nRetryMax)
{
	m_nRetryType  = nRetryType;
	m_nRetryDelay = nRetryDelay;
	m_nRetryMax	  = nRetryMax;
	
	// ���һ��m_nRetryMax,���Ϊ0����Ϊȱʡֵ
	if( (FTP_RETRY_TIMES == m_nRetryType) && (0 == m_nRetryMax) )
		m_nRetryMax = DEFAULT_FTP_RETRY_MAX;
}

// �����û����Ϳں�
void CFtpDownload::SetAuthorization(LPCTSTR lpszUsername, LPCTSTR lpszPassword, BOOL byAuthorization)
{
	if( byAuthorization && lpszUsername != NULL )
	{
		m_bAuthorization = TRUE;
		m_strUsername	 = lpszUsername;
		m_strPassword	 = lpszPassword;
	}
	else
	{
		m_bAuthorization = FALSE;
		m_strUsername	 = _T("anonymous");
		m_strPassword	 = _T("hello@dont.know.com");
	}
}

// ����PASVģʽ
void CFtpDownload::SetPasv(BOOL bPasv /*= TRUE*/)
{
	m_bPasv = bPasv;
}

// ֹͣ����
void CFtpDownload::StopDownload()
{
	m_bStopDownload = TRUE;

	if ( m_hStopEvent != NULL )
		SetEvent(m_hStopEvent);
}


// ��������SOCKET
BOOL CFtpDownload::CreateControlSocket()
{
	CloseControlSocket();
	return m_cControlSocket.Create(AF_INET,SOCK_STREAM,0,READ_BUFFER_SIZE);
}

// ��������SOCKET
BOOL CFtpDownload::CreateDataSocket( SOCKET hSocket /*= INVALID_SOCKET */)
{
	CloseDataSocket();
	return m_cDataSocket.Create(AF_INET,SOCK_STREAM,0,READ_BUFFER_SIZE,hSocket);
}


// �رտ���SOCKET
void CFtpDownload::CloseControlSocket()
{
	m_cControlSocket.Close(TRUE);
}

// �ر�����SOCKET
void CFtpDownload::CloseDataSocket()
{
	m_cDataSocket.Close(TRUE);
}


// ��URL�����ֳ�Server��Object��
BOOL CFtpDownload::ParseURL(LPCTSTR lpszURL, CString &strServer, CString &strObject,int& nPort)
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

	// ��һ����֤�Ƿ�ΪFtp://
	CString strTemp = strURL.Left( nPos+lstrlen("://") );
	//strTemp.MakeLower();
	//if( strTemp.Compare("ftp://") != 0 )
    if(strTemp.CompareNoCase("ftp://") != 0)
		return FALSE;

	strURL = strURL.Mid( strTemp.GetLength() );
	nPos = strURL.Find('/');
	if ( nPos == -1 )
		return FALSE;

	strObject = strURL.Mid(nPos);
	strTemp   = strURL.Left(nPos);
	
	// �����Ƿ��ж˿ں�
	nPos = strTemp.Find(":");
	if( nPos == -1 )
	{
		strServer = strTemp;
		nPort	  = DEFAULT_FTP_PORT;
	}
	else
	{
		strServer = strTemp.Left( nPos );
		strTemp	  = strTemp.Mid( nPos+1 );
		nPort	  = (int)_ttoi((LPCTSTR)strTemp);
	}
	return TRUE;
}


// ������URL�л�ȡ�ļ���
void CFtpDownload::GetFileName()
{
	// ��ȡ���ļ���
	int nSlash = m_strObject.ReverseFind(_T('/'));
	if (nSlash == -1)
		nSlash = m_strObject.ReverseFind(_T('\\'));

	if (nSlash != -1 && m_strObject.GetLength() > 1)
		m_strFileName = m_strObject.Right(m_strObject.GetLength() - nSlash - 1);
	else
		m_strFileName = m_strObject;
}


// ���ַ���ת����ʱ��
CTime CFtpDownload::GetTime(LPCTSTR lpszTime)
{
	CString strTime(lpszTime);
	CTime	dTime = CTime::GetCurrentTime();

	int nPos = strTime.FindOneOf(" \t");
	if( nPos == -1 )
		return dTime;

	CString strAllMonth = "jan,feb,mar,apr,may,jun,jul,aug,sep,oct,nov,dec";
	CString strDay,strMonth,strYear,strHour,strMinute;
	int		nDay,nMonth,nYear,nHour,nMinute,nSecond;


	strMonth = strTime.Left( nPos );
	strTime = strTime.Mid( nPos +1 );
	strTime.TrimLeft();

	nPos = strAllMonth.Find(strMonth);
	if( nPos == -1 )
		return dTime;
	strMonth.Format("%d",((nPos/4)+1));	
	nMonth = atoi(strMonth);

	nPos = strTime.FindOneOf(" \t");
	if( nPos == -1 )
		return dTime;
	strDay = strTime.Left( nPos );
	nDay = atoi(strDay);

	strTime = strTime.Mid( nPos+1 );
	strTime.TrimLeft();

	nPos = strTime.FindOneOf(":");
	if (nPos != -1)
	{
		strHour = strTime.Left( nPos );
		nHour = atoi(strHour);
	
		strTime = strTime.Mid( nPos+1 );
		strTime.TrimLeft();

		nPos = strTime.FindOneOf(":");
		if( nPos != -1 )
		{
			strMinute = strTime.Left( nPos );
			nMinute = atoi(strMinute);

			strTime = strTime.Mid( nPos+1 );
			strTime.TrimLeft();

			nSecond = atoi(strTime);

		}
		else
		{
			nMinute = atoi(strTime);
			nSecond = 0;
		}


	}
	else
	{
		nHour	= 0;
		nMinute = 0;
		nSecond = 0;
	}
	nYear = dTime.GetYear();

	dTime = CTime(nYear,nMonth,nDay,nHour,nMinute,nSecond);
	return	dTime;
}

// ��÷�����
BOOL CFtpDownload::GetReplyCode(LPCTSTR lpszResponse,DWORD& dwReplyCode,BOOL& bMultiLine)
{
	CString strSource(lpszResponse);

	dwReplyCode = 0;
	bMultiLine = FALSE;
	
	if( strSource.GetLength() < 3 )
		return FALSE;

	dwReplyCode = (DWORD)_ttoi(strSource.Left(3));

	if( strSource.GetLength() >= 4 )
		bMultiLine = (strSource.GetAt(3) == '-' );
	else
		bMultiLine = FALSE;

	return TRUE;
}

// ��ȡ����ͨ������Ϣ
BOOL CFtpDownload::GetDataReply(CString& strReply)
{
	char	szReadBuf[1025];
	strReply = _T("");

	// ������Ϣ
	ZeroMemory(szReadBuf,1025);
	if( m_cDataSocket.BSDGetString(szReadBuf,1024,m_dwRecvTimeout) != SOCKET_SUCCESS )
		return FALSE;

	strReply += szReadBuf;

	return TRUE;
}



// ���Reply��Ϣ
BOOL CFtpDownload::GetReply(CString& strReply,DWORD& dwReplyCode)
{
	char	szReadBuf[1025];
	CString strResponse;
	DWORD	dwStartCode;
	BOOL	bMultiLine;
	
	strReply = _T("");
	dwReplyCode = 0;

	// �Ƚ���һ��
	ZeroMemory(szReadBuf,1025);
	if( m_cControlSocket.BSDGetString(szReadBuf,1024,m_dwRecvTimeout) != SOCKET_SUCCESS )
		return FALSE;

	strReply += szReadBuf;

	if( !GetReplyCode(szReadBuf,dwReplyCode,bMultiLine) )
		return FALSE;

	if( !bMultiLine )
		return TRUE;

	dwStartCode = dwReplyCode;
	while(TRUE)
	{
		ZeroMemory(szReadBuf,1025);
		if( m_cControlSocket.BSDGetString(szReadBuf,1024,m_dwRecvTimeout) != SOCKET_SUCCESS )
			return FALSE;

		strReply += szReadBuf;		
		strReply += "\r\n";

		if( !GetReplyCode(szReadBuf,dwReplyCode,bMultiLine) )
			return FALSE;

		if( dwStartCode == dwReplyCode && !bMultiLine )
			break;
	}
	return TRUE;
}


// ����FTP����
BOOL CFtpDownload::SendCommand(LPCTSTR lpszCmd,LPCTSTR lpszParameter)
{
	CString strCmd(lpszCmd);
	
	if( lpszParameter != NULL && lstrlen(lpszParameter)>0 )
	{
		strCmd += " ";
		strCmd += lpszParameter;
	}
	strCmd += "\r\n";
 
	if( m_cControlSocket.Send(strCmd,strCmd.GetLength(),m_dwSendTimeout) != strCmd.GetLength() )
		return FALSE;

	return TRUE;

}

// ��Pasv�ķ�����Ϣ�л��IP�Ͷ˿�
BOOL CFtpDownload::GetPasvIP(LPCTSTR lpszReply,CString& strPasvIP,int& nPasvPort)
{
	CString strReply(lpszReply);
	CString strPort1,strPort2;
	
	strPasvIP = _T("");
	nPasvPort = 0;

	int nPos = strReply.Find('(');
	if( nPos == -1 )
		return FALSE;

	strPasvIP = strReply.Mid( nPos + 1 );
	nPos = strPasvIP.Find(')');
	if( nPos == -1 )
		return FALSE;

	strPasvIP = strPasvIP.Left(nPos);
	
	nPos = strPasvIP.ReverseFind(',');
	if( nPos == -1)
		return FALSE;
	
	strPort2 = strPasvIP.Right(strPasvIP.GetLength() - nPos -1 );
	strPasvIP = strPasvIP.Left(nPos);

	nPos = strPasvIP.ReverseFind(',');
	if( nPos == -1 )
		return FALSE;

	strPort1 = strPasvIP.Right(strPasvIP.GetLength() - nPos -1 );
	strPasvIP = strPasvIP.Left(nPos);

	strPasvIP.Replace(',','.');
	nPasvPort = (int) ( atoi(strPort1)*256+atoi(strPort2) );
	return TRUE;
}

// ��ȡ�ļ�ʱ��ʹ�С
BOOL CFtpDownload::GetInfo(LPCTSTR lpszReply,DWORD& dwSize,CTime& TimeLastModified)
{
	CString strReply(lpszReply);
	
	dwSize = 0;
	TimeLastModified = CTime::GetCurrentTime();
	
	//-rwxr-xr-x   1 ftpuser  ftpusers  14712179 Sep 13 09:55 csmproxy50nt.exe
	//-r--r--r--  1 root  wheel  454454 Oct  5 10:54 /pub/socks/dante-1.1.4.tar.gz
	//-r-xr-xr-x   1 0        0         986400 Jun 16  1998 /pub/Networks/Socket/Winsock/ws2setup.exe

	strReply.TrimLeft();
	strReply.TrimRight();
	strReply.MakeLower();
	
	int nCount = 0;
	int nPos ;
	while( nCount < 4 )
	{	
		nPos = strReply.FindOneOf(" \t");
		if( nPos == -1 )
			return FALSE;

		strReply = strReply.Mid(nPos+1);
		strReply.TrimLeft();
	
		nCount ++;
	}

	nPos = strReply.FindOneOf(" \t");
	if( nPos == -1 )
		return FALSE;
	
	CString strSize = strReply.Left(nPos);
	dwSize = (DWORD)_ttol(strSize);

	strReply = strReply.Mid(nPos+1);
	strReply.TrimLeft();

	int nPos1 = strReply.ReverseFind(' ');
	int nPos2 = strReply.ReverseFind('\t');
	
	if( nPos1 != -1 )
	{
		if( nPos2 != -1 )
		{
			nPos = ( nPos1 <= nPos2 )? nPos1:nPos2;
		}
		else
		{
			nPos = nPos1;
		}
	}
	else
	{
		if( nPos2 != -1 )
		{
			nPos = nPos2;
		}
		else
		{
			return FALSE;
		}
	}

	strReply = strReply.Left(nPos);
	strReply.TrimRight();

	TimeLastModified = GetTime(strReply);
	return TRUE;
}


// ��������
int CFtpDownload::SendRequest(BOOL bPasv /*= TRUE*/)
{
	
	while (TRUE)
	{
		CString			strSend, strReply;
		DWORD			dwSize, dwReplyCode;
		
		// ���ڱ���PASV��PORT�Ĳ���
		CString			strIP;
		int				nPort;
		CString         sMes;

        strIP = _T("");
		nPort = 0;


		// ��PASV 
		CBufSocket	cListenSocket;

		m_dwFileDownloadedSize = 0;
		m_dwDownloadSize	   = 0;

		// ���������׽���
		Check( !CreateControlSocket(), FTP_REQUEST_FAIL);
		
		// ��������ͨ��
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		int nRet = MakeConnection(&m_cControlSocket,m_strServer,m_nPort);
		Check( ( nRet != FTP_REQUEST_SUCCESS ), nRet );
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);// �Ȼ�ȡ��������Greeting��Ϣ
		//seawind
		Check( m_bStopDownload,FTP_REQUEST_STOP);
        
        //Removed by linsuyi 2001/01/31 16:32
        // sMes.LoadString(IDS_CREATE_CHANNELS_SUCCESS);
        // sMes += strReply;
        // TRACE1("%s ", sMes);
        
		// ��ʼ��������
		// USER
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !SendCommand(FTP_CMD_USER,m_strUsername) ,FTP_REQUEST_ERROR );
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR );
		//seawind
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		
        //Removed by linsuyi 2001/01/31 16:32
        // sMes.Empty();
        // sMes.LoadString(IDS_USER_COMMAND_SUCCESS);
        // sMes += strReply;
		// TRACE1("%s ", sMes);

		switch(dwReplyCode)
		{
		case 230:
			break;
		case 331: // Need Password
			// PASS
			Check( m_bStopDownload,FTP_REQUEST_STOP);
			Check( !SendCommand(FTP_CMD_PASS,m_strPassword),FTP_REQUEST_ERROR);
			Check( m_bStopDownload,FTP_REQUEST_STOP);
			Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
			//seawind
			Check( m_bStopDownload,FTP_REQUEST_STOP);
			Check( dwReplyCode != 230 ,FTP_REQUEST_FAIL);
			
            //Removed by linsuyi 2001/01/31 16:32
            // sMes.Empty();
            // sMes.LoadString(IDS_PASS_COMMAND_SUCCESS);
            // sMes += strReply;
			// TRACE1("%s ", sMes);

			break;
		default:
			return FTP_REQUEST_FAIL;
			break;
		}

		// �����Ƿ�֧�ֶϵ�����
		// REST 10
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !SendCommand(FTP_CMD_REST,_T("10")),FTP_REQUEST_ERROR);
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
		//seawind
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		if( dwReplyCode == 350 )
			m_bSupportResume = TRUE;
		else
			m_bSupportResume = FALSE;
        
        //Removed by linsuyi 2001/01/31 16:32
        // sMes.LoadString(IDS_REST10_COMMAND_SUCCESS);
        // sMes += strReply;
        // TRACE1("%s ", sMes);
        
		// �ַ�ģʽ
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !SendCommand(FTP_CMD_TYPE,_T("A")),FTP_REQUEST_ERROR);
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
		//seawind
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( dwReplyCode != 200 ,FTP_REQUEST_FAIL);	// ��������֧������ģʽ

        //Removed by linsuyi 2001/01/31 16:32
        // sMes.Empty();
        // sMes.LoadString(IDS_TYPEA_COMMAND_SUCCESS);
        // sMes += strReply;
        // TRACE1("%s ", sMes);

		// �Ƿ�֧��PASV
		if( bPasv )
		{
			// PASV
			Check( m_bStopDownload,FTP_REQUEST_STOP);
			Check( !SendCommand(FTP_CMD_PASV,NULL),FTP_REQUEST_ERROR);
			Check( m_bStopDownload,FTP_REQUEST_STOP);
			Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
			//seawind
            Check( m_bStopDownload,FTP_REQUEST_STOP);
	        
            //Removed by linsuyi 2001/01/31 16:32
            // sMes.Empty();
            // sMes.LoadString(IDS_PASV_COMMAND_SUCCESS);
            // sMes += strReply;
            // TRACE1("%s ", sMes);
			
			if( dwReplyCode == 227 )
			{
				if( GetPasvIP(strReply,strIP,nPort) )
				{
					m_bPasv = TRUE;
					// ��������ͨ��
					Check( m_bStopDownload,FTP_REQUEST_STOP);
					Check( !CreateDataSocket(),FTP_REQUEST_FAIL);
					nRet = MakeConnection(&m_cDataSocket,strIP,nPort);
					Check( nRet != FTP_REQUEST_SUCCESS,nRet);
				}
				else
					m_bPasv = FALSE;
			}
			else
				m_bPasv = FALSE;
		}

		dwSize = 0;
		m_TimeLastModified = CTime::GetCurrentTime();

        // �����Ƿ����PASV�����кܴ�����
        if (bPasv && m_bPasv)   // PASV
        {
            // ִ��LIST����
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( !SendCommand(FTP_CMD_LIST, m_strObject ),FTP_REQUEST_ERROR);
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
            //seawind
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            
            //Removed by linsuyi 2001/01/31 16:32
            // sMes.Empty();
            // sMes.LoadString(IDS_LIST_COMMAND_SUCCESS);
            // sMes += strReply;
            // TRACE1("%s ", sMes);
            
            if ((dwReplyCode == 125) || ( dwReplyCode == 150 ))
            {
                //������ͨ����ȡ��Ϣ
                Check(m_bStopDownload, FTP_REQUEST_STOP);
                if (GetDataReply(strReply))
                {
                    //Removed by linsuyi 2001/01/31 16:32
                    // sMes.Empty();
                    // sMes.LoadString(IDS_LIST_DATA_CHANNELS);
                    // sMes += strReply;
                    // TRACE1("%s ", sMes);
                    
                    if (!GetInfo(strReply,dwSize,m_TimeLastModified))
                    {
                        // TODO:
                    }
                }
                CloseDataSocket();
                
                //�ӿ���ͨ����ȡ��Ϣ
                Check( m_bStopDownload,FTP_REQUEST_STOP);
                Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
                //seawind
                Check( m_bStopDownload,FTP_REQUEST_STOP);
                Check( dwReplyCode != 250 && dwReplyCode != 226,FTP_REQUEST_ERROR);
                
                //Removed by linsuyi 2001/01/31 16:32
                // sMes.Empty();
                // sMes.LoadString(IDS_LIST_CONTROL_CHANNELS);
                // sMes += strReply;
                // TRACE1("%s ", sMes);
                
            }   //125.150 -- LIST COMMAND [PASV]
            
            //Binģʽ
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( !SendCommand(FTP_CMD_TYPE,_T("I")),FTP_REQUEST_ERROR);
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
            //seawind
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            
            //Removed by linsuyi 2001/01/31 16:32
            // sMes.Empty();
            // sMes.LoadString(IDS_TYPEI_COMMAND_SUCCESS);
            // sMes += strReply;
            // TRACE1("%s ", sMes);
                        
            if( dwReplyCode != 200 )
            {
                // TODO:
            }
            
            // PASV����
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( !SendCommand(FTP_CMD_PASV,NULL ),FTP_REQUEST_ERROR);
            Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
            //seawind
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( dwReplyCode != 227,FTP_REQUEST_ERROR);
            Check( !GetPasvIP(strReply,strIP,nPort),FTP_REQUEST_ERROR);
            
            //Removed by linsuyi 2001/01/31 16:32
            // sMes.Empty();
            // sMes.LoadString(IDS_PASV_PASV_COMMAND_SUCCESS);
            // sMes += strReply;
            // TRACE1("%s ", sMes);
            
            // ��������ͨ��
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( !CreateDataSocket(),FTP_REQUEST_FAIL);
            nRet = MakeConnection(&m_cDataSocket,strIP,nPort);
            Check( nRet != FTP_REQUEST_SUCCESS,nRet);
            
            //Removed by linsuyi 2001/01/31 16:32
            // sMes.Empty();
            // sMes.LoadString(IDS_PASV_CREATE_DATA_CHANNELS_SUCCESS);
            // sMes += strReply;
            // TRACE1("%s ", sMes);
        }
        else    // ������PASV
        {
            
            char		szListenIP[128],szTemp[128];
            int			nListenPort;
            DWORD		dwListenIP;
            CString		strListenIP;
            
            // �Ƿ�ΪSOCKS����
            if( m_nProxyType == FTP_PROXY_SOCKS4 ||
                m_nProxyType == FTP_PROXY_SOCKS4A||
                m_nProxyType == FTP_PROXY_SOCKS5 )
            {
                if( !CreateDataSocket() )
                    return FTP_REQUEST_FAIL;
                
                Check( m_bStopDownload,FTP_REQUEST_STOP);
                nRet = MakeConnection( &m_cDataSocket,m_strServer,m_nPort,TRUE,&dwListenIP,&nListenPort);
                Check( nRet != FTP_REQUEST_SUCCESS,nRet);
                strListenIP.Format("%d,%d,%d,%d",(dwListenIP>>24)&0xFF,(dwListenIP>>16)&0xFF,(dwListenIP>>8)&0xFF,(dwListenIP)&0xFF);
            }
            else
            {
                //û�д���
                Check( m_bStopDownload,FTP_REQUEST_STOP);
                Check( !cListenSocket.Create(),FTP_REQUEST_FAIL);
                Check( !cListenSocket.Bind(0), FTP_REQUEST_ERROR);
                Check( !cListenSocket.Listen(1),FTP_REQUEST_ERROR);
                
                ZeroMemory(szListenIP,128);
                Check( !m_cControlSocket.GetSockName(szListenIP,nListenPort),FTP_REQUEST_ERROR);
                
                ZeroMemory(szTemp,128);
                Check( !cListenSocket.GetSockName(szTemp,nListenPort),FTP_REQUEST_ERROR);
                
                strListenIP =  _T("");
                strListenIP += szListenIP;
                strListenIP.Replace('.',',');
            }
            
            strSend.Format(",%d,%d",nListenPort/256,nListenPort%256);
            strSend = strListenIP+strSend;
            
            // PORT
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( !SendCommand(FTP_CMD_PORT,strSend ),FTP_REQUEST_ERROR);
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
            //seawind
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( dwReplyCode != 200, FTP_REQUEST_FAIL );	// ����֧�ָ�����
            
            //Removed by linsuyi 2001/01/31 16:32
            // sMes.Empty();
            // sMes.LoadString(IDS_NOPASV_PORT_COMMAND_SUCCESS);
            // sMes += strReply;
            // TRACE1("%s ", sMes);
            
            // ִ��LIST����
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( !SendCommand(FTP_CMD_LIST, m_strObject),FTP_REQUEST_ERROR);
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
            //seawind
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            
            //Removed by linsuyi 2001/01/31 16:32
            // sMes.Empty();
            // sMes.LoadString(IDS_NOPASV_LIST_COMMAND_SUCCESS);
            // sMes += strReply;
            // TRACE1("%s ", sMes);
            
            if( (dwReplyCode == 125) || ( dwReplyCode == 150 ) )
            {
                if( m_nProxyType == FTP_PROXY_SOCKS4 ||
                    m_nProxyType == FTP_PROXY_SOCKS4A||
                    m_nProxyType == FTP_PROXY_SOCKS5 )
                {
                    CSocksPacket cSocks( &m_cDataSocket );
                    
                    switch ( m_nProxyType )
                    {
                    case FTP_PROXY_SOCKS4A:
                        Check( !cSocks.RecvPacket(PACKET_SOCKS4AREP,m_dwRecvTimeout),FTP_REQUEST_ERROR);
                        Check( !cSocks.IsSocksOK(),FTP_REQUEST_FAIL);
                        break;
                    case FTP_PROXY_SOCKS4:
                        Check( !cSocks.RecvPacket(PACKET_SOCKS4REP,m_dwRecvTimeout),FTP_REQUEST_ERROR);
                        Check( !cSocks.IsSocksOK(),FTP_REQUEST_FAIL);
                        break;
                    case FTP_PROXY_SOCKS5:
                        Check( !cSocks.RecvPacket(PACKET_SOCKS5REP,m_dwRecvTimeout),FTP_REQUEST_ERROR);
                        Check( !cSocks.IsSocksOK(),FTP_REQUEST_FAIL);
                        break;
                    }
                }
                else
                {
                    // ACCEPT
                    SOCKET hSocket = cListenSocket.Accept(NULL,0,m_hStopEvent,WSA_INFINITE);
                    Check( hSocket == INVALID_SOCKET,FTP_REQUEST_ERROR);	
                    Check( !CreateDataSocket(hSocket),FTP_REQUEST_FAIL);	
                }
                
                // ������ͨ����ȡ��Ϣ
                Check( m_bStopDownload,FTP_REQUEST_STOP);
                if (GetDataReply(strReply))
                {
                    //Removed by linsuyi 2001/01/31 16:32
                    // sMes.Empty();
                    // sMes.LoadString(IDS_NOPASV_LIST_DATA_CHANNELS);
                    // sMes += strReply;
                    // TRACE1("%s ", sMes);
                    
                    if( !GetInfo(strReply,dwSize,m_TimeLastModified) )
                    {
                        // TODO:
                    }
                }
                CloseDataSocket();
                
                // �ӿ���ͨ����ȡ��Ϣ
                Check( m_bStopDownload,FTP_REQUEST_STOP);
                Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
                //seawind
                Check( m_bStopDownload,FTP_REQUEST_STOP);
                Check( dwReplyCode != 250 && dwReplyCode != 226,FTP_REQUEST_ERROR);
                
                //Removed by linsuyi 2001/01/31 16:32
                // sMes.Empty();
                // sMes.LoadString(IDS_NOPASV_LIST_CONTROL_CHANNELS);
                // sMes += strReply;
                // TRACE1("%s ", sMes);
            } // LIST
            
            // Binģʽ
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( !SendCommand(FTP_CMD_TYPE,_T("I")),FTP_REQUEST_ERROR);
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
            //seawind
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            
            //Removed by linsuyi 2001/01/31 16:32
            // sMes.Empty();
            // sMes.LoadString(IDS_NOPASV_TYPEI_COMMAND_SUCCESS);
            // sMes += strReply;
            // TRACE1("%s ", sMes);
            
            if (dwReplyCode != 200)
            {
                // TODO:
            }
            
            // PORT����
            if (
                (m_nProxyType == FTP_PROXY_SOCKS4) ||
                (m_nProxyType == FTP_PROXY_SOCKS4A) ||
                (m_nProxyType == FTP_PROXY_SOCKS5)
            )
            {
                Check( m_bStopDownload,FTP_REQUEST_STOP);
                Check( !CreateDataSocket(),FTP_REQUEST_FAIL);
                nRet = MakeConnection( &m_cDataSocket,m_strServer,m_nPort,TRUE,&dwListenIP,&nListenPort);
                Check( nRet != FTP_REQUEST_SUCCESS,nRet);
                strListenIP.Format("%d,%d,%d,%d",(dwListenIP>>24)&0xFF,(dwListenIP>>16)&0xFF,(dwListenIP>>8)&0xFF,(dwListenIP)&0xFF);
            }
            else
            {
                Check( m_bStopDownload,FTP_REQUEST_STOP);
                Check( !cListenSocket.Create(),FTP_REQUEST_FAIL);
                Check( !cListenSocket.Bind(0),FTP_REQUEST_ERROR);
                Check( !cListenSocket.Listen(1),FTP_REQUEST_ERROR);
                
                ZeroMemory(szListenIP,128);
                Check( !m_cControlSocket.GetSockName(szListenIP,nListenPort),FTP_REQUEST_ERROR);
                
                ZeroMemory(szTemp,128);
                Check( !cListenSocket.GetSockName(szTemp,nListenPort),FTP_REQUEST_ERROR);
                
                strListenIP = _T("");
                strListenIP += szListenIP;
                strListenIP.Replace('.',',');
            }
            
            strSend.Format(",%d,%d",nListenPort/256,nListenPort%256);
            strSend = strListenIP+strSend;
            
            // PORT
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( !SendCommand(FTP_CMD_PORT,strSend ),FTP_REQUEST_ERROR);
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
            //seawind
            Check( m_bStopDownload,FTP_REQUEST_STOP);
            Check( dwReplyCode != 200, FTP_REQUEST_FAIL);// ����֧�ָ�����
            
            //Removed by linsuyi 2001/01/31 16:32
            // sMes.Empty();
            // sMes.LoadString(IDS_NOPASV_PORT_COMMAND_SUCCESS);
            // sMes += strReply;
            // TRACE1("%s ", sMes);
            
        }   // NO PASV
        
        // ׼������
        if (dwSize == 0)
        {
			// ��һ��SIZE����
			Check( m_bStopDownload,FTP_REQUEST_STOP);
			Check( !SendCommand(FTP_CMD_SIZE, m_strObject ),FTP_REQUEST_ERROR);
			Check( m_bStopDownload,FTP_REQUEST_STOP);
			Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
			//seawind
			Check( m_bStopDownload,FTP_REQUEST_STOP);
			
            //Removed by linsuyi 2001/01/31 16:32
            // sMes.Empty();
            // sMes.LoadString(IDS_SIZE_COMMAND_SUCCESS);
            // sMes += strReply;
            // TRACE1("%s ", sMes);
            
			if( dwReplyCode == 213 )
			{
				strReply.TrimLeft();
				strReply.TrimRight();
				int nPos = strReply.Find(' ');
				if( nPos != -1 )
				{
					strReply = strReply.Mid( nPos + 1 );
					strReply.TrimLeft();
					dwSize = (DWORD)_ttol(strReply);
				}
			}
		}
		
		// �ж�m_strSavePath�Ƿ�Ϊ·��
		GetFileName();
		if( m_strSavePath.Right(1) == '\\' )
		{
			m_strTempSavePath = m_strSavePath;
			m_strTempSavePath += m_strFileName;
			m_strTempSavePath += ".tmp";
		}

		CString		strRange;
		m_dwFileDownloadedSize = 0;
		if( m_bSupportResume )
		{
			// �鿴�ļ��Ѿ����صĳ���
			CFileStatus fileDownStatus;
			if (CFile::GetStatus(m_strTempSavePath,fileDownStatus) && !m_bForceDownload )
				m_dwFileDownloadedSize = fileDownStatus.m_size;

			strRange.Format(_T("%d"),m_dwFileDownloadedSize );

			// REST COMMAND
			Check( m_bStopDownload,FTP_REQUEST_STOP);
			Check( !SendCommand(FTP_CMD_REST,strRange ),FTP_REQUEST_ERROR);
			Check( m_bStopDownload,FTP_REQUEST_STOP);
			Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
			//seawind
			Check( m_bStopDownload,FTP_REQUEST_STOP);
			Check( dwReplyCode != 350,FTP_REQUEST_ERROR);

            //Removed by linsuyi 2001/01/31 16:32
            // sMes.Empty();
            // sMes.LoadString(IDS_REST_COMMAND_SUCCESS);
            // sMes += strReply;
            // TRACE1("%s ", sMes);
		}

		// ע��
		m_dwFileSize	 = dwSize;
		m_dwDownloadSize = dwSize - m_dwFileDownloadedSize;
		Check( m_dwDownloadSize < 0,FTP_REQUEST_FAIL);
		
		// ����RETR����
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !SendCommand(FTP_CMD_RETR,m_strObject ),FTP_REQUEST_ERROR);
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !GetReply(strReply,dwReplyCode),FTP_REQUEST_ERROR);
		//seawind
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( (dwReplyCode != 125 && dwReplyCode != 150 && dwReplyCode != 110),FTP_REQUEST_FAIL);
        
        //Removed by linsuyi 2001/01/31 16:32
        // sMes.Empty();
        // sMes.LoadString(IDS_RETR_COMMAND_SUCCESS);
        // sMes += strReply;
        // TRACE1("%s ", sMes);
        
        // ��PASV��ʽҪ��������ͨ��
		if( !bPasv || !m_bPasv )
		{
			if( m_nProxyType == FTP_PROXY_SOCKS4 ||
				m_nProxyType == FTP_PROXY_SOCKS4A||
				m_nProxyType == FTP_PROXY_SOCKS5 )
			{
				CSocksPacket cSocks( &m_cDataSocket );
				switch ( m_nProxyType )
				{
				case FTP_PROXY_SOCKS4A:
					Check( m_bStopDownload,FTP_REQUEST_STOP);
					Check( !cSocks.RecvPacket(PACKET_SOCKS4AREP,m_dwRecvTimeout),FTP_REQUEST_ERROR);
					Check( !cSocks.IsSocksOK(),FTP_REQUEST_FAIL);
					break;
				case FTP_PROXY_SOCKS4:
					Check( m_bStopDownload,FTP_REQUEST_STOP);
					Check( !cSocks.RecvPacket(PACKET_SOCKS4REP,m_dwRecvTimeout),FTP_REQUEST_ERROR);
					Check( !cSocks.IsSocksOK(),FTP_REQUEST_FAIL);
					break;
				case FTP_PROXY_SOCKS5:
					Check( m_bStopDownload,FTP_REQUEST_STOP);
					Check( !cSocks.RecvPacket(PACKET_SOCKS5REP,m_dwRecvTimeout),FTP_REQUEST_ERROR);
					Check( !cSocks.IsSocksOK(),FTP_REQUEST_FAIL);
					break;
				}
			}
			else
			{
				Check( m_bStopDownload,FTP_REQUEST_STOP);
				SOCKET hSocket = cListenSocket.Accept(NULL,0,m_hStopEvent,WSA_INFINITE);
				Check( hSocket == INVALID_SOCKET,FTP_REQUEST_ERROR);	
				Check( !CreateDataSocket(hSocket),FTP_REQUEST_FAIL);	
			}
		} // NO PASV ��������ͨ��

		return FTP_REQUEST_SUCCESS;
	}// WHILE LOOP

	return FTP_REQUEST_SUCCESS;
}


// ��ʼ����
int CFtpDownload::Download(LPCTSTR lpszDownloadUrl,LPCTSTR lpszSavePath,BOOL bForceDownload /*= FALSE */)
{
	m_bStopDownload	  = FALSE;
	m_bForceDownload  = bForceDownload;
	m_nRetryTimes	  = 0;
	m_nErrorCount	  = 0;
	m_strTempSavePath = _T("");
	
	// ����Ҫ���ص�URL�Ƿ�Ϊ��
	m_strDownloadUrl = lpszDownloadUrl;
	m_strDownloadUrl.TrimLeft();
	m_strDownloadUrl.TrimRight();
	if( m_strDownloadUrl.IsEmpty() )
		return FTP_RESULT_FAIL;

	// ����Ҫ���ص�URL�Ƿ���Ч
	if ( !ParseURL(m_strDownloadUrl, m_strServer, m_strObject, m_nPort))
	{
		// ��ǰ�����"ftp://"����
		m_strDownloadUrl = _T("ftp://") + m_strDownloadUrl;
		if ( !ParseURL(m_strDownloadUrl,m_strServer, m_strObject, m_nPort) )
		{
			TRACE(_T("Failed to parse the URL: %s\n"), m_strDownloadUrl);
			return FTP_RESULT_FAIL;
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
			return FTP_RESULT_FAIL;

		//seawind
		m_cControlSocket.m_hStopEvent = m_hStopEvent;
		m_cDataSocket.m_hStopEvent = m_hStopEvent;
	}
	ResetEvent( m_hStopEvent );

	// ������������
	m_dwDownloadedSize		= 0;
	m_dwFileDownloadedSize	= 0;
	m_dwFileSize			= 0;
	m_dwDownloadSize		= 0;

	BOOL bSendOnce = TRUE;		// ���ڿ�����hWndNotify���ڷ�����Ϣ
	
ReDownload:
	int nRequestRet = SendRequest( m_bPasv ) ;

	//seawind
	//if (WaitForSingleObject(m_hStopEvent, 0) == WAIT_OBJECT_0)
	//	return HTTP_RESULT_STOP;
	Check( m_bStopDownload,FTP_RESULT_STOP);

	switch(nRequestRet)
	{
	case FTP_REQUEST_SUCCESS:
		TRACE0("\n\n***FTP_REQUEST_SUCCESS");
		break;
	case FTP_REQUEST_STOP:
		TRACE0("\n\n***FTP_REQUEST_STOP");
		return FTP_RESULT_STOP;
		break;
	case FTP_REQUEST_FAIL:
		TRACE0("\n\n***FTP_REQUEST_FAIL");
		return FTP_RESULT_FAIL;
		break;
	case FTP_REQUEST_ERROR:
		TRACE0("\n\n***FTP_REQUEST_ERROR");
		// ֹͣ����?
		Check( m_bStopDownload,FTP_RESULT_STOP);

		switch( m_nRetryType )
		{
		case FTP_RETRY_NONE:
			return FTP_RESULT_FAIL;
			break;
		case FTP_RETRY_ALWAYS:
			if( m_nRetryDelay > 0 )
			{
				//Ϊ���õȴ�ʱҲ���˳�������ʹ��Sleep����
				if( WaitForSingleObject(m_hStopEvent,m_nRetryDelay) == WAIT_OBJECT_0 )
					return FTP_RESULT_STOP;
			}
			goto ReDownload;
			break;
		case FTP_RETRY_TIMES:
			if( m_nRetryTimes > m_nRetryMax )
				return FTP_RESULT_FAIL;
			m_nRetryTimes++;
		
			if( m_nRetryDelay > 0 )
			{
				//Ϊ���õȴ�ʱҲ���˳�������ʹ��Sleep����
				if( WaitForSingleObject(m_hStopEvent,m_nRetryDelay) == WAIT_OBJECT_0 )
					return FTP_RESULT_STOP;
			}
			
			goto ReDownload;
			break;
		default:
			return FTP_RESULT_FAIL;
			break;
		}
		break;
	default:
		return FTP_RESULT_FAIL;
		break;
	}

	
	// Now Support none content-length header
	//if (m_dwDownloadSize == 0)
	//	return FTP_RESULT_FAIL;


	if( m_strSavePath.Right(1) == '\\' )
		m_strSavePath += m_strFileName;

	if( !m_bForceDownload ) // ��ǿ�����أ����Last-Modified
	{
		CFileStatus fileStatus;
		if (CFile::GetStatus(m_strSavePath,fileStatus))
		{
			// ���ܻ����1������
			if (( fileStatus.m_mtime - m_TimeLastModified <=2 && m_TimeLastModified-fileStatus.m_mtime<=2 ) && (DWORD)fileStatus.m_size == m_dwFileSize )
				return FTP_RESULT_SAMEAS;
		}
	}
	CFile fileDown;
	if(! fileDown.Open(m_strTempSavePath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite|CFile::shareDenyWrite) )
	{
		TRACE0("\n\n*** FILE OPEN FAILED!");
		return FTP_RESULT_FAIL;	
	}

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
		  return FTP_RESULT_FAIL;
		}	
	}
	
	if( bSendOnce && m_bNotify )
	{
		FTPDOWNLOADSTATUS DownloadStatus;
		
		DownloadStatus.dwFileSize  = m_dwFileSize;
		DownloadStatus.strFileName = m_strFileName;
		DownloadStatus.dwFileDownloadedSize  = m_dwFileDownloadedSize;

		DownloadStatus.nStatusType = FTP_STATUS_FILESIZE;
		if (WaitForSingleObject(m_hStopEvent, 0) != WAIT_OBJECT_0)
			::SendMessage(m_hNotifyWnd,m_nNotifyMessage,MSG_FTPDOWNLOAD_STATUS,(LPARAM)&DownloadStatus);	

		DownloadStatus.nStatusType = FTP_STATUS_FILENAME;
		if (WaitForSingleObject(m_hStopEvent, 0) != WAIT_OBJECT_0)
			::SendMessage(m_hNotifyWnd,m_nNotifyMessage,MSG_FTPDOWNLOAD_STATUS,(LPARAM)&DownloadStatus);	
	
		DownloadStatus.nStatusType = FTP_STATUS_FILEDOWNLOADEDSIZE;
		if (WaitForSingleObject(m_hStopEvent, 0) != WAIT_OBJECT_0)
			::SendMessage(m_hNotifyWnd,m_nNotifyMessage,MSG_FTPDOWNLOAD_STATUS,(LPARAM)&DownloadStatus);	
		
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

	do
	{
		// ֹͣ����?
		Check( m_bStopDownload,FTP_RESULT_STOP);
		
		ZeroMemory(szReadBuf,READ_BUFFER_SIZE+1);
		int nRet = m_cDataSocket.BSDGetData(szReadBuf,READ_BUFFER_SIZE,m_dwRecvTimeout);

		//seawind
		Check( m_bStopDownload, FTP_RESULT_STOP);		

		if (nRet <= 0)
		{
			////////////////////////////////////////
			if( m_dwDownloadSize == 0xFFFFFFFF )
				break;
			///////////////////////////////////////

			fileDown.Close();
			m_nErrorCount++;
			goto ReDownload; //�ٴη�������
		}

		// ������д���ļ�
		try
		{
			fileDown.Write(szReadBuf,nRet);
		}
		catch(CFileException* e)
		{
			e->Delete();
			fileDown.Close();
			goto ReDownload;
		}

		m_dwDownloadedSize		+= nRet;
		m_dwFileDownloadedSize	+= nRet;

		// ֪ͨ��Ϣ
		if (m_bNotify)
		{
			FTPDOWNLOADSTATUS DownloadStatus;
            
			DownloadStatus.nStatusType			= FTP_STATUS_FILEDOWNLOADEDSIZE;
			DownloadStatus.dwFileDownloadedSize = m_dwFileDownloadedSize;
			DownloadStatus.dwFileSize			= m_dwFileSize;
			DownloadStatus.strFileName			= m_strFileName;

			if (WaitForSingleObject(m_hStopEvent, 0) != WAIT_OBJECT_0)
				::SendMessage(m_hNotifyWnd,m_nNotifyMessage,MSG_FTPDOWNLOAD_STATUS,(LPARAM)&DownloadStatus);	
		}
    } while(m_dwDownloadedSize < m_dwDownloadSize);
    
	TRACE0("\n\n***FILE DOWNLOAD SUCCESS!");

	// �ر��ļ�
	fileDown.Close();
	
	//�ر�SOCKET
	CloseControlSocket();
	CloseDataSocket();

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
    
    // ���ٽ�����������
    return FTP_RESULT_SUCCESS;
}


// ��������
int CFtpDownload::MakeConnection(CBufSocket* pBufSocket,CString strServer,int nPort,BOOL bBind /*= FALSE*/ ,LPDWORD	lpdwIP /*= NULL*/,LPINT lpnPort /*= NULL*/)
{
	CSocksPacket	cSocks( pBufSocket );
	DWORD			dwIP;
	BYTE			byAuth,byAtyp,byCmd;
	SOCKSREPPACKET	pack;
	CString			strAuth,strAddr,strIP,strDigit;

	strAuth = _T("");
	strAddr = _T("");

	// ����Ƿ�ΪBIND����
	if( bBind && (	m_nProxyType == FTP_PROXY_SOCKS4  || 
					m_nProxyType == FTP_PROXY_SOCKS4A || 
					m_nProxyType == FTP_PROXY_SOCKS5 ) )
	{
		byCmd = CMD_BIND;
		if( lpdwIP == NULL || lpnPort == NULL )
			return FTP_REQUEST_ERROR;
	}
	else
	{
		bBind = FALSE;
		byCmd = CMD_CONNECT;
	}

	switch( m_nProxyType )
	{
	case FTP_PROXY_NONE:
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !pBufSocket->Connect(strServer,nPort,m_dwConnTimeout,TRUE),FTP_REQUEST_ERROR);
		break;
	case FTP_PROXY_SOCKS4A:
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		dwIP = CBufSocket::GetIP(strServer,TRUE);
		if( dwIP == INADDR_NONE )
		{
			Check( m_bStopDownload,FTP_REQUEST_STOP);
			Check( !pBufSocket->Connect(m_strProxyServer,m_nProxyPort,m_dwConnTimeout,TRUE),FTP_REQUEST_ERROR);

			Check( m_bStopDownload,FTP_REQUEST_STOP);
			Check( !cSocks.SendSocks4aReq(byCmd,nPort,strServer,m_strProxyUsername,m_dwSendTimeout),FTP_REQUEST_ERROR);
			Check( m_bStopDownload,FTP_REQUEST_STOP);

			ZeroMemory(&pack,sizeof(SOCKSREPPACKET));
			Check( !cSocks.RecvPacket(&pack,PACKET_SOCKS4AREP,m_dwRecvTimeout),FTP_REQUEST_ERROR);
			Check( !cSocks.IsSocksOK(&pack,PACKET_SOCKS4AREP),FTP_REQUEST_FAIL);// �����д������Կ�����û���õ�

			// �����BIND����
			if( bBind )
			{
				if( pack.socks4aRep.dwDestIP == 0 )
					*lpdwIP = ntohl(CBufSocket::GetIP( m_strProxyServer ));
				else
					*lpdwIP = pack.socks4aRep.dwDestIP;

				*lpnPort= (int)pack.socks4aRep.wDestPort;
			}
			
			break;// NOTICE:��������ܹ���������������ʹ��SOCKS4 Proxy
		}
	case FTP_PROXY_SOCKS4:
		// ����Ҫ�õ�Proxy Server��IP��ַ(����Ϊ����)
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		dwIP = CBufSocket::GetIP(strServer,TRUE);
		Check( dwIP == INADDR_NONE ,FTP_REQUEST_ERROR);

		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !pBufSocket->Connect(m_strProxyServer,m_nProxyPort,m_dwConnTimeout,TRUE),FTP_REQUEST_ERROR);
		
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !cSocks.SendSocks4Req( byCmd,nPort,dwIP,m_strProxyUsername,m_dwSendTimeout),FTP_REQUEST_ERROR);

		ZeroMemory(&pack,sizeof(SOCKSREPPACKET));
		Check( !cSocks.RecvPacket(&pack,PACKET_SOCKS4REP,m_dwRecvTimeout),FTP_REQUEST_ERROR);
		Check( !cSocks.IsSocksOK(&pack,PACKET_SOCKS4REP),FTP_REQUEST_FAIL); // �����д������Կ�����û���õ�

		// �����BIND����
		if( bBind )
		{
			if( pack.socks4Rep.dwDestIP == 0 )
				*lpdwIP = ntohl(CBufSocket::GetIP( m_strProxyServer) );
			else
				*lpdwIP = pack.socks4Rep.dwDestIP;
			*lpnPort= (int)pack.socks4Rep.wDestPort;
		}
		break;
	case FTP_PROXY_SOCKS5:

		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !pBufSocket->Connect(m_strProxyServer,m_nProxyPort,m_dwConnTimeout,TRUE),FTP_REQUEST_ERROR);

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
			
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !cSocks.SendSocks5AuthReq(byAuth,(LPCTSTR)strAuth,m_dwSendTimeout),FTP_REQUEST_ERROR);
		Check( m_bStopDownload,FTP_REQUEST_STOP);

		ZeroMemory(&pack,sizeof(SOCKSREPPACKET));
		Check( !cSocks.RecvPacket(&pack,PACKET_SOCKS5AUTHREP,m_dwRecvTimeout),FTP_REQUEST_ERROR);
		Check( !cSocks.IsSocksOK(&pack,PACKET_SOCKS5AUTHREP),FTP_REQUEST_FAIL);	// �����д������Կ�����û���õ�

		switch( pack.socks5AuthRep.byAuth )
		{
		case AUTH_NONE:
			break;
		case AUTH_PASSWD:

			Check( !m_bProxyAuthorization ,FTP_REQUEST_FAIL);
			Check( m_bStopDownload,FTP_REQUEST_STOP);
			Check( !cSocks.SendSocks5AuthPasswdReq(m_strProxyUsername,m_strProxyPassword,m_dwSendTimeout),FTP_REQUEST_ERROR);
			Check( m_bStopDownload,FTP_REQUEST_STOP);
			
			ZeroMemory(&pack,sizeof(SOCKSREPPACKET));
			Check( !cSocks.RecvPacket(&pack,PACKET_SOCKS5AUTHPASSWDREP,m_dwRecvTimeout),FTP_REQUEST_ERROR);
			Check( !cSocks.IsSocksOK(&pack,PACKET_SOCKS5AUTHPASSWDREP) ,FTP_REQUEST_FAIL); // �����д������Կ�����û���õ�
			break;
		case AUTH_GSSAPI:
		case AUTH_CHAP:
		case AUTH_UNKNOWN:
		default:
			return FTP_REQUEST_FAIL;
			break;
		}
		
		Check( m_bStopDownload,FTP_REQUEST_STOP);
		dwIP = CBufSocket::GetIP(strServer,TRUE);
		if( dwIP != INADDR_NONE )
		{
			byAtyp = ATYP_IPV4ADDR;
			// ����Ҫת���ֽ���(�Ѿ��������ֽ���)
			strAddr += (char)( (dwIP    ) &0x000000ff); 
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

		Check( m_bStopDownload,FTP_REQUEST_STOP);
		Check( !cSocks.SendSocks5Req(byCmd,byAtyp,strAddr,nPort,m_dwSendTimeout),FTP_REQUEST_ERROR);
		Check( m_bStopDownload,FTP_REQUEST_STOP);

		ZeroMemory(&pack,sizeof(SOCKSREPPACKET));
		Check( !cSocks.RecvPacket(&pack,PACKET_SOCKS5REP,m_dwRecvTimeout),FTP_REQUEST_ERROR);
		Check( !cSocks.IsSocksOK(&pack,PACKET_SOCKS5REP),FTP_REQUEST_FAIL); // �����д������Կ�����û���õ�

		// �����BIND����
		if( bBind )
		{
			Check( pack.socks5Rep.byAtyp != ATYP_IPV4ADDR,FTP_REQUEST_ERROR);

			strIP  = _T("");
	
			strDigit.Format("%d",(BYTE)pack.socks5Rep.pszBindAddr[0]);
			strIP += strDigit;
			strIP += ".";

			strDigit.Format("%d",(BYTE)pack.socks5Rep.pszBindAddr[1]);
			strIP += strDigit;
			strIP += ".";

			strDigit.Format("%d",(BYTE)pack.socks5Rep.pszBindAddr[2]);
			strIP += strDigit;
			strIP += ".";

			strDigit.Format("%d",(BYTE)pack.socks5Rep.pszBindAddr[3]);
			strIP += strDigit;

			*lpdwIP = ntohl( inet_addr(strIP) );
				
			if( *lpdwIP == 0 )
				*lpdwIP = ntohl( CBufSocket::GetIP( m_strProxyServer ) );

			*lpnPort= (int)pack.socks5Rep.wBindPort;
		}
		
		break;
	default:
		return FTP_REQUEST_FAIL;
		break;
	}
	return FTP_REQUEST_SUCCESS;
}

//seawind
BOOL CFtpDownload::IsUserStop()
{
	ASSERT(m_hStopEvent != NULL);

	if (WaitForSingleObject(m_hStopEvent, 0) == WAIT_OBJECT_0)
		return TRUE;
	else
		return FALSE;
}
