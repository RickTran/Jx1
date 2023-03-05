
// FtpDownload.h: interface for the CFtpDownload class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FTPDOWNLOAD_H__0866041E_80F8_4547_A298_A5B387FC77CB__INCLUDED_)
#define AFX_FTPDOWNLOAD_H__0866041E_80F8_4547_A298_A5B387FC77CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BufSocket.h"


// ��������
// ���ؽ��
const int	FTP_RESULT_SUCCESS	= 0;	// �ɹ�
const int	FTP_RESULT_SAMEAS	= 1;	// Ҫ���ص��ļ��Ѿ����ڲ�����Զ���ļ�һ�£���������
const int	FTP_RESULT_STOP		= 2;	// ��;ֹͣ(�û��ж�)
const int	FTP_RESULT_FAIL		= 3;	// ����ʧ��

// ��Ϣ
const WPARAM	MSG_FTPDOWNLOAD_NETWORK	= (WPARAM)33;
const WPARAM	MSG_FTPDOWNLOAD_STATUS	= (WPARAM)34;
const WPARAM	MSG_FTPDOWNLOAD_RESULT	= (WPARAM)35;
const WPARAM	MSG_FTPDOWNLOAD_MAX		= (WPARAM)64; //�������ɹ�����

//����״̬
const int		FTP_STATUS_FILENAME				= 1;
const int		FTP_STATUS_FILESIZE				= 2;
const int		FTP_STATUS_FILEDOWNLOADEDSIZE	= 3;
/*
const int		FTP_STATUS_ERRORCOUNT			= 4;
const int		FTP_STATUS_ERRORCODE			= 5;
const int		FTP_STATUS_ERRORSTRING			= 6;
*/

// �������
const int 		FTP_RETRY_NONE			= 0;
const int 		FTP_RETRY_TIMES			= 1;
const int 		FTP_RETRY_ALWAYS		= 2;
const int 		DEFAULT_FTP_RETRY_MAX	= 10; //ȱʡ�����Դ���

// PROXY������
const int		FTP_PROXY_NONE			= 0;
const int		FTP_PROXY_SOCKS4		= 1;
const int		FTP_PROXY_SOCKS4A		= 2;
const int		FTP_PROXY_SOCKS5		= 3;

//For use IE proxy setting
//  Added by Linsuyi
//  2001/02/01  01:20
const int       FTP_PROXY_USEIE         = 4;

// ȱʡ�˿ں�
static const int	 	DEFAULT_FTP_PORT= 21;

// ȱʡ��ʱ����
static const DWORD FTP_CONN_TIMEOUT	= 120*1000;// 120��
static const DWORD FTP_SEND_TIMEOUT	= 120*1000;// 120��
static const DWORD FTP_RECV_TIMEOUT	= 120*1000;// 120��

// ��������
static const int	FTP_REQUEST_SUCCESS	= 0; // �ɹ�
static const int	FTP_REQUEST_ERROR	= 1; // һ��������󣬿�������
static const int	FTP_REQUEST_STOP	= 2; // ��;ֹͣ(�û��ж�) (��������)
static const int	FTP_REQUEST_FAIL	= 3; // ʧ�� (��������)	 

// FTP����
static const char FTP_CMD_USER[] = _T("USER");
static const char FTP_CMD_PASS[] = _T("PASS");
static const char FTP_CMD_ACCT[] = _T("ACCT");
static const char FTP_CMD_QUIT[] = _T("QUIT");
static const char FTP_CMD_PORT[] = _T("PORT");
static const char FTP_CMD_PASV[] = _T("PASV");
static const char FTP_CMD_TYPE[] = _T("TYPE");
static const char FTP_CMD_RETR[] = _T("RETR");
static const char FTP_CMD_REST[] = _T("REST");
static const char FTP_CMD_LIST[] = _T("LIST");
static const char FTP_CMD_STAT[] = _T("STAT");
static const char FTP_CMD_SITE[] = _T("SITE");
static const char FTP_CMD_ABOR[] = _T("ABOR");
static const char FTP_CMD_SIZE[] = _T("SIZE");

#pragma pack(push, 1)

// �ṹ����
typedef struct _tagFtpDownloadStatus
{
	int		nStatusType;
	DWORD	dwFileSize;
	DWORD	dwFileDownloadedSize;
	CString	strFileName;
	/*
	int		nErrorCount;
	CString strError;
	DWORD	dwErrorCode;
	*/
} FTPDOWNLOADSTATUS,*PFTPDOWNLOADSTATUS;

#pragma pack(pop)


class CFtpDownload  
{
public:
    CFtpDownload();
    virtual ~CFtpDownload();
    
public:
    BOOL IsUserStop();
    void SetPasv(BOOL bPasv = TRUE);
    void SetAuthorization(LPCTSTR lpszUsername,LPCTSTR lpszPassword,BOOL bAuthorization = TRUE );
    void SetProxy(LPCTSTR lpszProxyServer,int nProxyPort,BOOL bProxy=TRUE,BOOL bProxyAuthorization = FALSE,LPCTSTR lpszProxyUsername = NULL,LPCTSTR lpszProxyPassword = NULL,int nProxyType = FTP_PROXY_NONE);
    void SetTimeout(DWORD dwSendTimeout,DWORD dwRecvTimeout,DWORD dwConnTimeout);
    void SetRetry(int nRetryType,int nRetryDelay=0,int nRetryMax = 0);
    void SetNotifyWnd(HWND hNotifyWnd,int nNotifyMsg,BOOL bNotify = TRUE );
    void StopDownload();
    int  Download(LPCTSTR lpszDownloadUrl,LPCTSTR lpszSavePath = NULL,BOOL bForceDownload = FALSE);
    
private:
    BOOL    GetIEProxy(CString &strProxyServer, int &nProxyPort, int &nProxyType);
    BOOL    SendCommand(LPCTSTR lpszCmd,LPCTSTR lpszParameter);
    BOOL    GetDataReply(CString& strReply);
    BOOL    GetInfo(LPCTSTR lpszReply,DWORD& dwSize,CTime& TimeLastModified);
    BOOL    GetPasvIP(LPCTSTR lpszReply,CString& strPasvIP,int& nPasvPort);
    BOOL    GetReplyCode(LPCTSTR lpszResponse,DWORD& dwReplyCode,BOOL& bMultiLine);
    BOOL    GetReply(CString& strReply,DWORD& dwReplyCode);
    int     SendRequest(BOOL bPasv = TRUE);
    BOOL    CreateControlSocket();
    void    CloseControlSocket();
    BOOL    CreateDataSocket( SOCKET hSocket = INVALID_SOCKET );
    void    CloseDataSocket();
    BOOL    ParseURL(LPCTSTR lpszURL,CString& strServer,CString& strObject,int& nPort);
    void    GetFileName();
    CTime   GetTime(LPCTSTR lpszTime);
    int     MakeConnection(CBufSocket* pBufSocket,CString strServer,int nPort,BOOL bBind = FALSE ,LPDWORD lpdwIP = NULL,LPINT lpnPort = NULL);
    
private:
    // ������URL�ͱ��ر���·��
    CString		m_strDownloadUrl;
    CString		m_strSavePath;	  // ������ȫ·���򱣴�Ŀ¼	
    CString		m_strTempSavePath;//��ʱ�ļ���·��
    
    // ֹͣ����
    BOOL		m_bStopDownload;
    HANDLE		m_hStopEvent;
    
    // ǿ����������(�������е��ļ��Ƿ���Զ���ļ���ͬ)
    BOOL		m_bForceDownload;
    
    // �ļ�����(Զ���ļ�����Ϣ)
    CTime		m_TimeLastModified;
    
    // �Ƿ�֧�ֶϵ�����
    BOOL		m_bSupportResume;
    
    // �Ƿ�ʹ��PASV��ʽ
    BOOL		m_bPasv;
    
    // �ļ��Լ����ش�С
    DWORD		m_dwFileSize;				// �ļ��ܵĴ�С	
    DWORD		m_dwFileDownloadedSize;		// �ļ��ܹ��Ѿ����صĴ�С
    
    DWORD		m_dwDownloadSize;			// ������Ҫ���صĴ�С
    DWORD		m_dwDownloadedSize;			// �����Ѿ����صĴ�С
    
    
    // ��ʱTIMEOUT	���ӳ�ʱ�����ͳ�ʱ�����ճ�ʱ(��λ������)
    DWORD		m_dwConnTimeout;	
    DWORD		m_dwRecvTimeout;
    DWORD		m_dwSendTimeout;
    
    // ���Ի���
    int		m_nRetryType;	//��������(0:������ 1:����һ������ 2:��������)
    int		m_nRetryTimes;	//���Դ���
    int		m_nRetryDelay;	//�����ӳ�(��λ������)
    int		m_nRetryMax;    //���Ե�������
    
    // ������
    int		m_nErrorCount;	//������� (��ʱû������)
    CString	m_strError;		//������Ϣ (��ʱû������)
    
    
    // ���������ڷ�����Ϣ
    BOOL		m_bNotify;			// �Ƿ����ⷢ��֪ͨ��Ϣ	
    HWND		m_hNotifyWnd;		// ��֪ͨ�Ĵ���
    int			m_nNotifyMessage;	// ��֪ͨ����Ϣ
    
    // �Ƿ�ʹ�ô��� 
    BOOL		m_bProxy;
    CString		m_strProxyServer;
    int			m_nProxyPort;
    int			m_nProxyType;
    
    // �����Ƿ���Ҫ��֤
    BOOL		m_bProxyAuthorization;
    CString		m_strProxyUsername;
    CString		m_strProxyPassword;
    
    // FTP�û��Ϳ���
    BOOL		m_bAuthorization;
    CString		m_strUsername;
    CString		m_strPassword;
    
    
    // ���ع��������õı���
    CString		m_strServer;
    CString		m_strObject;
    CString		m_strFileName;
    int			m_nPort;
    
    CBufSocket	m_cControlSocket;	// �������ӵ�SOCKET
    CBufSocket	m_cDataSocket;		// �������ӵ�SOCKET
};


#endif // !defined(AFX_FTPDOWNLOAD_H__0866041E_80F8_4547_A298_A5B387FC77CB__INCLUDED_)
