
// BufSocket.h: interface for the CBufSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUFSOCKET_H__ABC62098_FF8C_412F_BE5B_820881B1E334__INCLUDED_)
#define AFX_BUFSOCKET_H__ABC62098_FF8C_412F_BE5B_820881B1E334__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ����ͷ�ļ�
#include "winsock2.h"

// �궨��
#define Min(a, b)               (((a) < (b)) ? (a): (b))
#define INext(i, n)             ((((i) + 1) < (n)) ? ((i) + 1): 0)

#undef	IN
#define IN
#undef	OUT
#define OUT

// ����
const int SOCKET_SUCCESS					= 0;		// SOCKET�����ɹ�����
const int SOCKET_FAIL						= (-1);		// SOCKET����ʧ��
const int SOCKET_BUFFER_TOO_SMALL			= 1;		// ������̫С
const int INVALID_CHAR						= 0x100;	// ��Ч�����ַ�(�����ַ��ǵ��ֽڵ�)

static const int DEFAULT_BUFFER_SIZE			= 4096;		// ȱʡ�Ļ�������С
static const int DEFAULT_SHUTDOWN_RECV_TIMEOUT	= 4*1000;	// ���Źر�ʱ�ӳ�4��
static const int DEFAULT_BLOCKED_SNDRCV_SLEEP	= 100;		// 100����(��������ʱ�ȴ�[˯��]ʱ��)
static const int DEFAULT_CONN_TIMEOUT			= 20*1000;	// 20�� ���ӳ�ʱ
static const int DEFAULT_SEND_TIMEOUT			= 20*1000;	// 20�� ���ͳ�ʱ
static const int DEFAULT_RECV_TIMEOUT			= 20*1000;	// 20�� ���ճ�ʱ 
static const int DEFAULT_ACCEPT_TIMEOUT			= 20*1000;	// 20�� �ȴ���ʱ 


#pragma pack(push, 1)

// ���ݽṹ(������ѭ��ʹ��)
typedef struct _tagBufSocketData
{
    SOCKET			hSocket;			// SOCKET
    int             nBufferSize;		// ���ݻ����С
    char*           pszBuffer;			// ���ݻ���
    int             nBytesInBuffer;		// �������������ݳ���(�ֽ�)
    int             nReadIndex;			// ���Զ�ȡ����һ������λ��
    int             nBufferIndex;		// ����ʹ�õ���һ������λ��	
} BSD,*PBSD;

#pragma pack(pop)


class CBufSocket  
{
public:
	CBufSocket();
	virtual ~CBufSocket();

public:
	// ��ʼ�����ͷŶ�̬���ӿ�
	static BOOL		InitLibrary( int nMajorVer = 2 ,int nMinorVer = 2 );
	static void		CleanupLibrary();
	static DWORD	GetIP(const char* pszName,BOOL bFixNtDNS  = FALSE); // Used to Fix NT DNS Problem	

public:

	//seawind
	HANDLE m_hStopEvent;

	int		GetLastError(void);	// ��ȡ����

	BOOL	Create(	int		nAddressFamily	= AF_INET, 
					int		nType			= SOCK_STREAM, 
					int		nProtocol		= 0,
					int		nBufferSize		= DEFAULT_BUFFER_SIZE,
					SOCKET	hSocket			= INVALID_SOCKET,
					DWORD	dwFlags			= WSA_FLAG_OVERLAPPED);

	void	Close(	BOOL bHardClose = FALSE );
	int		Recv( OUT char *pszBuffer, int nBufferSize, DWORD dwTimeout = DEFAULT_RECV_TIMEOUT);
	int		Send(char const * pszBuffer, int nBufferSize, DWORD dwTimeout = DEFAULT_SEND_TIMEOUT);
	
	BOOL	Connect(char const * pszServer, int nPort,DWORD dwTimeout = DEFAULT_CONN_TIMEOUT,BOOL bFixNtDNS = FALSE);
	int		BSDGetChar(DWORD dwTimeout = DEFAULT_RECV_TIMEOUT);
	int		BSDGetString(OUT char *pszBuffer, int nBufferSize,DWORD dwTimeout = DEFAULT_RECV_TIMEOUT);
	int		BSDGetData(OUT char *pszBuffer, int nBufferSize,DWORD dwTimeout = DEFAULT_RECV_TIMEOUT);
	
	SOCKET	GetSocket();
	BOOL	GetSockName(OUT char *pszIP,OUT int& nPort);

	// Add in 1.1 <
	BOOL	Listen(int nConnections = 5);
	BOOL	Bind(int nPort = 0);
	SOCKET	Accept(struct sockaddr * pSocketAddr, int *nAddrLen,DWORD dwTimeout = DEFAULT_ACCEPT_TIMEOUT);
	SOCKET	Accept(struct sockaddr * pSocketAddr, int *nAddrLen,HANDLE hEndEvent,DWORD dwTimeout = DEFAULT_ACCEPT_TIMEOUT);
	// Add in 1.1 >

	// Add in 1.2 <
	BOOL	RecvFrom(OUT char* pszFrom,OUT int& nFromPort,OUT char *pszBuffer,IN int nBufferSize,IN DWORD dwTimeout = DEFAULT_RECV_TIMEOUT);
	BOOL	SendTo(const char* pszTo,int nToPort,const char * pszBuffer, int nBufferSize, DWORD dwTimeout = DEFAULT_SEND_TIMEOUT);
	BOOL	SetSockOpt( int nOptionName, const void* lpOptionValue, int nOptionLen, int nLevel = SOL_SOCKET );
	BOOL	GetSockOpt( int nOptionName, void* lpOptionValue, int* lpOptionLen, int nLevel = SOL_SOCKET );
	// Add in 1.2 >
	

private:
	void	SetLastError(int nErrorCode);
	BOOL	SetSocketOption(SOCKET hSocket);
	BOOL	Connect(SOCKET hSocket, const struct sockaddr * pSocketAddr, int iAddrLen,DWORD dwTimeout);
	int     RecvOnce(SOCKET hSocket, OUT char *pszBuffer, int nBufferSize);
	int		RecvData(SOCKET hSocket, OUT char *pszBuffer, int nBufferSize, DWORD dwTimeout);
	int     SendOnce(SOCKET hSocket, char const * pszBuffer, int nBufferSize);
	int		SendData(SOCKET hSocket, char const * pszBuffer, int nBufferSize, DWORD dwTimeout);
	int		BSDReadData(PBSD pBSD, DWORD dwTimeout);

	// Add in 1.1 <
	BOOL	Bind(SOCKET hSocket, const struct sockaddr * pSocketAddr, int nAddrLen);
	BOOL    Block(SOCKET hSocket, BOOL bBlock);
	// Add in 1.1 >

	// Add in 1.2 <
	int		RecvFrom(SOCKET hSocket, struct sockaddr * pFrom, int nFromlen,char *pszBuffer, int nBufferSize, DWORD dwTimeout);
	int		SendTo(SOCKET hSocket, const struct sockaddr * pTo,int nToLen,char const * pszBuffer, int nBufferSize, DWORD dwTimeout);
	// Add in 1.2 >

private:
	SOCKET	m_hSocket;
	PBSD	m_pBSD;
	int		m_nErrorCode;	//�������
};

#endif // !defined(AFX_BUFSOCKET_H__ABC62098_FF8C_412F_BE5B_820881B1E334__INCLUDED_)
