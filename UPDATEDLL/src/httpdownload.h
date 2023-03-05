
// HttpDownload.h: interface for the CHttpDownload class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPDOWNLOAD_H__4E55FDB1_0DE8_4ACD_94F4_CD097B5EAED0__INCLUDED_)
#define AFX_HTTPDOWNLOAD_H__4E55FDB1_0DE8_4ACD_94F4_CD097B5EAED0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BufSocket.h"

////////////////////////////////////////////////////////////////////////////////
// �ṹ����

#pragma pack(push, 1)

typedef struct _tagHttpDownloadStatus
{
	int		nStatusType;				// ״̬����
	DWORD	dwFileSize;					// �ļ���С
	DWORD	dwFileDownloadedSize;		// �Ѿ����ش�С
	CString	strFileName;				// �ļ���
	int		nErrorCount;				// �������
	CString strError;					// ����ԭ��
	DWORD	dwErrorCode;				// �������	
} HTTPDOWNLOADSTATUS,*PHTTPDOWNLOADSTATUS;

#pragma pack(pop)


////////////////////////////////////////////////////////////////////////////////
//					��������
////////////////////////////////////////////////////////////////////////////////
#undef	IN	
#define IN	// �������
#undef	OUT
#define OUT	// �������

////////////////////////////////////////////////////////////////////////////////
//					�ⲿ����
////////////////////////////////////////////////////////////////////////////////
// ���ؽ��
const int	HTTP_RESULT_SUCCESS			= 0;	// �ɹ�
const int	HTTP_RESULT_SAMEAS			= 1;	// Ҫ���ص��ļ��Ѿ����ڲ�����Զ���ļ�һ�£���������
const int	HTTP_RESULT_STOP			= 2;	// ��;ֹͣ(�û��ж�)
const int	HTTP_RESULT_FAIL			= 3;	// ����ʧ��
const int	HTTP_RESULT_REDIRECT_FTP	= 4;	// �ض���FTP

// Added by Linsuyi  2002/02/01
const int   HTTP_RESULT_REDIRECT_HTTP   = 5;    // �ض���HTTP


// ��Ϣ
const WPARAM	MSG_HTTPDOWNLOAD_NETWORK	= (WPARAM)1; // ����״̬
const WPARAM	MSG_HTTPDOWNLOAD_STATUS		= (WPARAM)2; // ����״̬
const WPARAM	MSG_HTTPDOWNLOAD_MAX		= (WPARAM)32; //�������ɹ�����


// �������
const int	HTTP_RETRY_NONE			= 0;	// ������
const int	HTTP_RETRY_TIMES		= 1;	// ����һ������
const int	HTTP_RETRY_ALWAYS		= 2;	// ��������(�п�����ѭ��)
const int	DEFAULT_HTTP_RETRY_MAX	= 10;	//ȱʡ�����Դ���

// PROXY������
const int	HTTP_PROXY_NONE			= 0;	// û�д���
const int	HTTP_PROXY_HTTPGET		= 1;	// ͨ����HTTP��GET�ʹ���
const int	HTTP_PROXY_HTTPCONNECT	= 2;	// HTTP CONNECT����
const int	HTTP_PROXY_SOCKS4		= 3;	// SOCKS4 ����	
const int	HTTP_PROXY_SOCKS4A		= 4;	// SOCKS4A����
const int	HTTP_PROXY_SOCKS5		= 5;	// SOCKS 5����
const int	HTTP_PROXY_USEIE		= 6;	// ʹ��IE���õĴ������������

//����״̬����
const int	HTTP_STATUS_FILENAME			= 1;	// �ļ���
const int	HTTP_STATUS_FILESIZE			= 2;	// �ļ���С
const int	HTTP_STATUS_FILEDOWNLOADEDSIZE	= 3;	// �ļ��Ѿ����ش�С
const int	HTTP_STATUS_ERRORCOUNT			= 4;	// �����������
const int	HTTP_STATUS_ERRORCODE			= 5;	// �������
const int	HTTP_STATUS_ERRORSTRING			= 6;	// ����ԭ��

// HTTP֧�ֵ�����
const int	HTTP_VERB_MIN		= 0;
const int	HTTP_VERB_POST      = 0;
const int	HTTP_VERB_GET       = 1;
const int	HTTP_VERB_HEAD      = 2;
const int	HTTP_VERB_PUT       = 3;
const int	HTTP_VERB_OPTIONS   = 4;
const int	HTTP_VERB_DELETE    = 5;
const int	HTTP_VERB_TRACE	    = 6;
const int	HTTP_VERB_CONNECT   = 7;
const int	HTTP_VERB_MAX		= 7;

////////////////////////////////////////////////////////////////////////////////
//							�ڲ�˽��
////////////////////////////////////////////////////////////////////////////////
// ȱʡ��ʱ����
static const DWORD HTTP_CONN_TIMEOUT	= 60*1000;	// 60��
static const DWORD HTTP_SEND_TIMEOUT	= 60*1000;	// 60��
static const DWORD HTTP_RECV_TIMEOUT	= 60*1000;	// 60��

// ��������Ľ��
static const int	HTTP_REQUEST_SUCCESS		= 0;	// �ɹ�
static const int	HTTP_REQUEST_ERROR			= 1;	// һ��������󣬿�������
static const int	HTTP_REQUEST_STOP			= 2;	// ��;ֹͣ(�û��ж�) (��������)
static const int	HTTP_REQUEST_FAIL			= 3;	// ʧ�� (��������)
static const int	HTTP_REQUEST_REDIRECT_FTP	= 4;	// �ض���FTP

// HTTP STATUS CODE����
static const int	HTTP_SUCCESS		= 0;	// �ɹ�		
static const int	HTTP_REDIRECT		= 1;	// �ض���
static const int	HTTP_FAIL			= 2;	// ʧ��
static const int	HTTP_REDIRECT_FTP	= 3;	// �ض���FTP

// ȱʡ�˿ں�
static const int	DEFAULT_HTTP_PORT	= 80;

// HTTP����
static const char*	HTTP_VERB_STR[] = 
{
	_T("POST "),
	_T("GET "),
	_T("HEAD "),
	_T("PUT "),
	_T("OPTIONS "),
	_T("DELETE "),
	_T("TRACE "),
	_T("CONNECT ")
};


////////////////////////////////////////////////////////////////////////////////
//						�ඨ��
////////////////////////////////////////////////////////////////////////////////
class CHttpDownload  
{
public:
	CHttpDownload();
	virtual ~CHttpDownload();

public:
	BOOL IsUserStop();
	// ��̬����������64���롢����
	static int Base64Encode(IN LPCTSTR lpszEncoding,OUT CString& strEncoded );
	static int Base64Decode(IN LPCTSTR lpszDecoding,OUT CString& strDecoded );

	// ���غ���
	void SetCookie(LPCTSTR lpszCookie,BOOL bUseIECookie = FALSE);
	void SetAuthorization(LPCTSTR lpszUsername,LPCTSTR lpszPassword,BOOL bAuthorization = TRUE );
	void SetReferer(LPCTSTR lpszReferer);
	void SetUserAgent(LPCTSTR lpszUserAgent);
	void SetTimeout(DWORD dwSendTimeout,DWORD dwRecvTimeout,DWORD dwConnTimeout);
	void SetRetry(int nRetryType,int nRetryDelay=0,int nRetryMax = 0);
	void SetNotifyWnd(HWND hNotifyWnd,int nNotifyMsg,BOOL bNotify = TRUE );
	void SetProxy(LPCTSTR lpszProxyServer,int nProxyPort,BOOL bProxy=TRUE,BOOL bProxyAuthorization = FALSE,LPCTSTR lpszProxyUsername = NULL,LPCTSTR lpszProxyPassword = NULL,int nProxyType = HTTP_PROXY_HTTPGET);
	void StopDownload();
	BOOL ParseURL(IN LPCTSTR lpszURL,OUT CString& strServer,OUT CString& strObject,OUT int& nPort);
	BOOL GetDownloadFileStatus(IN LPCTSTR lpszDownloadUrl,OUT DWORD &dwFileSize,OUT CTime &FileTime);
	int  Download(LPCTSTR lpszDownloadUrl,LPCTSTR lpszSavePath = NULL,BOOL bForceDownload = FALSE,BOOL bSaveResponse = TRUE,int nVerb = HTTP_VERB_GET,LPCTSTR lpszData = NULL,LPTSTR lpszFtpURL = NULL );

private:
	BOOL	CreateSocket();
	void	CloseSocket();
	int		SendRequest(int nVerb = HTTP_VERB_GET,LPTSTR lpszFtpURL = NULL);
	int		GetInfo(IN LPCTSTR lpszHeader,OUT DWORD& dwContentLength,OUT DWORD& dwStatusCode,OUT CTime& TimeLastModified,OUT CString& strCookie,LPTSTR lpszFtpURL);
	CTime	GetTime(LPCTSTR lpszTime);
	void	GetFileName();
	int		MakeConnection(CBufSocket* pBufSocket,CString strServer,int nPort);
	BOOL	GetIEProxy(CString& strProxyServer,int& nProxyPort,int& nProxyType);
	BOOL	GetIECookie(CString& strCookie,LPCTSTR lpszURL);
	BOOL	IsValidFileName(LPCTSTR lpszFileName);
	
private:	
	// ���ز���
	// ������URL�ͱ��ر���·��
	CString		m_strDownloadUrl;
	CString		m_strSavePath;		// ������ȫ·���򱣴�Ŀ¼	
	CString		m_strTempSavePath;	//��ʱ�ļ���·��
	BOOL		m_bSaveResponse;

	// ֹͣ����
	BOOL		m_bStopDownload;	// ֹͣ����	
	HANDLE		m_hStopEvent;		// ֹͣ�����¼�

	// ǿ����������(�������е��ļ��Ƿ���Զ���ļ���ͬ)
	BOOL		m_bForceDownload;

	// �Ƿ�֧�ֶϵ�����
	BOOL		m_bSupportResume;

	// �ļ��Լ����ش�С
	DWORD		m_dwFileSize;				// �ļ��ܵĴ�С	
	DWORD		m_dwFileDownloadedSize;		// �ļ��ܹ��Ѿ����صĴ�С

	DWORD		m_dwDownloadSize;			// ������Ҫ���صĴ�С
	DWORD		m_dwDownloadedSize;			// �����Ѿ����صĴ�С

	DWORD		m_dwHeaderSize;				// ����ͷ�Ĵ�С (��ʱû������)
	CString		m_strHeader;				// ����ͷ����Ϣ (��ʱû������)

	// �ļ�����(Զ���ļ�����Ϣ)
	CTime		m_TimeLastModified;

	// Referer
	CString		m_strReferer;
	
	// UserAgent
	CString		m_strUserAgent;

	// ��ʱTIMEOUT	���ӳ�ʱ�����ͳ�ʱ�����ճ�ʱ(��λ������)
	DWORD		m_dwConnTimeout;	
	DWORD		m_dwRecvTimeout;
	DWORD		m_dwSendTimeout;

	// ���Ի���
	int			m_nRetryType;	//��������(0:������ 1:����һ������ 2:��������)
	int			m_nRetryTimes;	//���Դ���
	int			m_nRetryDelay;	//�����ӳ�(��λ������)
	int			m_nRetryMax;    //���Ե�������

	// ������
	int			m_nErrorCount;	//������� 
	CString		m_strError;		//������Ϣ 
	DWORD		m_dwErrorCode;	//�������

	// ���������ڷ�����Ϣ
	BOOL		m_bNotify;			// �Ƿ����ⷢ��֪ͨ��Ϣ	
	HWND		m_hNotifyWnd;		// ��֪ͨ�Ĵ���
	int			m_nNotifyMessage;	// ��֪ͨ����Ϣ

	// �Ƿ������֤ : Request-Header: Authorization
	BOOL		m_bAuthorization;
	CString		m_strUsername;
	CString		m_strPassword;

	// �Ƿ�ʹ�ô��� 
	BOOL		m_bProxy;
	CString		m_strProxyServer;
	int			m_nProxyPort;
	int			m_nProxyType;
	
	// �����Ƿ���Ҫ��֤: Request-Header: Proxy-Authorization
	BOOL		m_bProxyAuthorization;
	CString		m_strProxyUsername;
	CString		m_strProxyPassword;

	// ���ع��������õı���
	CString		m_strServer;
	CString		m_strObject;
	CString		m_strFileName;
	int			m_nPort;
	int			m_nVerb;
	CString		m_strData;// ����ʱ���͵�����

	// �Ƿ�ʹ��Cookie
	CString		m_strCookie;
	BOOL		m_bUseIECookie;

	CBufSocket	m_cBufSocket;// �������ӵ�SOCKET
	
	// ����BASE64���롢����
	static int		m_nBase64Mask[];
	static CString	m_strBase64TAB;
};

#endif // !defined(AFX_HTTPDOWNLOAD_H__4E55FDB1_0DE8_4ACD_94F4_CD097B5EAED0__INCLUDED_)
