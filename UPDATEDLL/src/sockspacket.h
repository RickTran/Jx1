
// SocksPacket.h: interface for the CSocksPacket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKSPACKET_H__04B153A0_80D4_442C_ACCE_0FA5C8ECF577__INCLUDED_)
#define AFX_SOCKSPACKET_H__04B153A0_80D4_442C_ACCE_0FA5C8ECF577__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BufSocket.h" //��ҪCBufSocket��

//��������
#undef	IN
#define IN
#undef	OUT
#define OUT

static const BYTE	BYTE_NULL		= 0x00;			// ���ֽ�
static const WORD	WORD_NULL		= 0x0000;		// ����
static const DWORD	UNKNOWN_IP		= 0x10000000;	// �����IP��ַ,ָ��ҪSOCKS4A PROXY�������� 0.0.0.1
static const BYTE	AUTH_OK			= 0x00;			// Username/Passwd��֤OK
static const DWORD	SOP_SEND_TIMEOUT= 120*1000;		//WSA_INFINITE;	
static const DWORD	SOP_RECV_TIMEOUT= 120*1000;		//WSA_INFINITE;

// SOCKS������
const int	SOCKS_TYPE_4	= 0x01;
const int	SOCKS_TYPE_4A	= 0x02;
const int	SOCKS_TYPE_5	= 0x03; 

// SOCKS�İ汾��
const BYTE	VER_SOCKS4		= 0x04;
const BYTE	VER_SOCKS4A		= 0x04;
const BYTE	VER_SOCKS5		= 0x05;
const BYTE	VER_SOCKSUNKNOWN= 0xFF;

//��֧�ֵ�����
const BYTE	CMD_CONNECT		= 0x01;
const BYTE	CMD_BIND		= 0x02;
const BYTE	CMD_UDP			= 0x03;	// (SOCKS5 ONLY)
const BYTE	CMD_UNKNOWN		= 0xFF;

/* SOCKS4(A)����(����ֵ) */
const BYTE	REP4_SUCCESS	= 90;
const BYTE	REP4_RESULT		= 90;
const BYTE	REP4_FAIL		= 91;
const BYTE	REP4_NOIDENTD	= 92;
const BYTE	REP4_BADID		= 93;

/* SOCKS5����(����ֵ) */
const BYTE	REP5_SUCCESS	= 0x00;
const BYTE	REP5_RESULT		= 0x00;
const BYTE	REP5_FAIL		= 0x01;
const BYTE	REP5_NOTALLOW	= 0x02;
const BYTE	REP5_NETUNREACH	= 0x03;
const BYTE	REP5_HOSTUNREACH= 0x04;
const BYTE	REP5_CONNREF	= 0x05;
const BYTE	REP5_TTLEXP		= 0x06;
const BYTE	REP5_BADCMD		= 0x07;
const BYTE	REP5_BADADDR	= 0x08;

// ��֤��ʽ(SOCKS5 ONLY)
const BYTE	AUTH_NONE		= 0x00;
const BYTE	AUTH_GSSAPI		= 0x01;
const BYTE	AUTH_PASSWD		= 0x02;
const BYTE	AUTH_CHAP		= 0x03;
const BYTE	AUTH_UNKNOWN	= 0xFF;

// ��ַ��ʽ(SOCKS5 ONLY)
const BYTE	ATYP_IPV4ADDR	= 0x01;
const BYTE	ATYP_HOSTNAME	= 0x03;
const BYTE	ATYP_IPV6ADDR	= 0x04;

// ��������
const int	PACKET_SOCKS4REQ			= 0x01;
const int	PACKET_SOCKS4AREQ			= 0x02;
const int	PACKET_SOCKS5AUTHREQ		= 0x03;
const int	PACKET_SOCKS5AUTHPASSWDREQ	= 0x04;
const int	PACKET_SOCKS5REQ			= 0x05;

const int	PACKET_SOCKS4REP			= 0x21;
const int	PACKET_SOCKS4AREP			= 0x22;
const int	PACKET_SOCKS5AUTHREP		= 0x23;
const int	PACKET_SOCKS5AUTHPASSWDREP	= 0x24;
const int	PACKET_SOCKS5REP			= 0x25;

const int	PACKET_SOCKS5UDP			= 0x41;


#pragma pack(push, 1)

///////////////////////////////////////////////////////
//
//	SOCK4 Proxy
//
//////////////////////////////////////////////////////
typedef struct _tagSocks4Req
{
	BYTE	byVer;		// �汾�� 0x04
	BYTE	byCmd;		// ����
	WORD	wDestPort;	// Ŀ��˿�
	DWORD	dwDestIP;	// Ŀ��IP��ַ
	LPCTSTR	pszUserID;	// �û�ID
	BYTE	byNULL;		// 0x00
} SOCKS4REQ,*PSOCKS4REQ;

typedef struct _tagSock4Rep
{
	BYTE	byVer;
	BYTE	byRep;		// 0x90-0x93
	WORD	wDestPort;
	DWORD	dwDestIP;
} SOCK4REP,SOCKS4AREP,*PSOCK4REP,*PSOCK4AREP;

///////////////////////////////////////////////////////
//
//	SOCK4A Proxy
//
//////////////////////////////////////////////////////

typedef struct _tagSocks4AReq
{
	BYTE	byVer;			// �汾�� 0x04
	BYTE	byCmd;			// ����
	WORD	wDestPort;		// Ŀ��˿�
	DWORD	dwDestIP;		// Ŀ�����IP��ַ 0x000000XX
	LPCTSTR	pszUserID;		// �û�ID
	BYTE	byNULL1;		// �ָ���
	LPCTSTR	pszDestHostName;// Ŀ���������
	BYTE	byNULL2;		// ����0x00
} SOCKS4AREQ,*PSOCKS4AREQ;

///////////////////////////////////////////////////////
//
//	SOCK5 Proxy
//
//////////////////////////////////////////////////////
// Auth-Request
typedef struct _tagSocks5AuthReq
{
	BYTE	byVer;
	BYTE	byNAuth;
	LPCTSTR	pszAuth;
} SOCKS5AUTHREQ,*PSOCKS5AUTHREQ;

// Auth-Reply
typedef struct _tagSocks5AuthRep
{
	BYTE	byVer;
	BYTE	byAuth;
} SOCKS5AUTHREP,*PSOCKS5AUTHREP;

// USERNAME/PASSWORD��֤ - Request
typedef struct _tagSocks5AuthPasswdReq
{
	BYTE	byVer;
	BYTE	byUserNameLen;
	LPCTSTR	pszUserName;
	BYTE	byPasswdLen;
	LPCTSTR	pszPasswd;
} SOCKS5AUTHPASSWDREQ,*PSOCKS5AUTHPASSWDREQ;

// USERNAME/PASSWORD��֤ - Reply
typedef struct _tagSocks5AuthPasswdRep
{
	BYTE	byVer;
	BYTE	byStatus;
} SOCKS5AUTHPASSWDREP,*PSOCKS5AUTHPASSWDREP;
 
// Request
typedef struct _tagSocks5Req
{
	BYTE	byVer;
	BYTE	byCmd;
	BYTE	byRsv;
	BYTE	byAtyp;
	LPCTSTR	pszDestAddr;
	WORD	wDestPort;
} SOCKS5REQ,*PSOCKS5REQ;

// Reply
typedef struct _tagSocks5Rep
{
	BYTE	byVer;
	BYTE	byRep;
	BYTE	byRsv;
	BYTE	byAtyp;
	LPSTR	pszBindAddr;
	WORD	wBindPort;
} SOCKS5REP,*PSOCKS5REP;


// UDP (Socks5 ONLY)
typedef struct _tagSocks5UDP
{
	WORD	wRsv;
	BYTE	byFrag;
	BYTE	byAtyp;
	LPSTR	pszDestAddr;
	WORD	wDestPort;
	LPSTR	pszData;
} SOCKS5UDP,*PSOCKS5UDP;

// ��������
// Request-Packet (TCP ONLY)
typedef union _tagSocksReqPacket
{
	SOCKS4REQ				socks4Req;
	SOCKS4AREQ				socks4aReq;
	SOCKS5AUTHREQ			socks5AuthReq;
	SOCKS5AUTHPASSWDREQ		socks5AuthPasswdReq;
	SOCKS5REQ				socks5Req;
} SOCKSREQPACKET,*PSOCKSREQPACKET;

// Reply-Packet (TCP ONLY)
typedef union _tagSocksRepPacket
{
	SOCK4REP				socks4Rep;
	SOCKS4AREP				socks4aRep;
	SOCKS5AUTHREP			socks5AuthRep;
	SOCKS5AUTHPASSWDREP		socks5AuthPasswdRep;
	SOCKS5REP				socks5Rep;
} SOCKSREPPACKET,*PSOCKSREPPACKET;

// Request-Reply UDP Packet (UDP ONLY)
typedef union _tagSocksUDPPacket
{
	SOCKS5UDP				socks5UDP;
} SOCKSUDPPACKET,*PSOCKSUDPPACKET;

#pragma pack(pop)


class CSocksPacket  
{
public:
	CSocksPacket( CBufSocket* pBufSocket );
	virtual ~CSocksPacket();

public:
	BOOL	SendSocks4Req(BYTE byCmd,WORD wDestPort,DWORD dwDestIP,const char * pszUserID  = NULL ,DWORD dwTimeout  = SOP_SEND_TIMEOUT );
	BOOL	SendSocks4aReq(BYTE byCmd,WORD wDestPort,const char* pszDestHostName,const char * pszUserID  = NULL ,DWORD dwTimeout  = SOP_SEND_TIMEOUT );
	BOOL	SendSocks5AuthReq(BYTE byNAuth,const char* pszAuth,DWORD dwTimeout  = SOP_SEND_TIMEOUT );
	BOOL	SendSocks5AuthPasswdReq(const char* pszUserName,const char * pszPassword,DWORD dwTimeout  = SOP_SEND_TIMEOUT );
	BOOL	SendSocks5Req(BYTE byCmd,BYTE byAtyp,const char * pszDestAddr,WORD wDestPort,DWORD dwTimeout  = SOP_SEND_TIMEOUT );
	BOOL	RecvPacket(OUT PSOCKSREPPACKET pPacket,int nPacketType,DWORD dwTimeout = SOP_RECV_TIMEOUT);
	BOOL	IsSocksOK( PSOCKSREPPACKET pPacket,int nPacketType );
	BOOL	SendPacket(PSOCKSREQPACKET pPacket,int nPacketType,DWORD dwTimeout = SOP_SEND_TIMEOUT);

	// ADD IN 1.1 <
	BOOL	RecvPacket(int nPacketType,DWORD dwTimeout = SOP_RECV_TIMEOUT);
	BOOL	IsSocksOK();
	// ADD IN 1.1 >

private:
	BOOL	Pack(OUT char *pszDest,PSOCKSREQPACKET pPacket,int nPacketType);
	int		PacketSize(PSOCKSREQPACKET pPacket,int nPacketType);

private:
	CBufSocket*		m_pBufSocket;

	// ADD IN 1.1 <
	SOCKSREPPACKET	m_stRepPacket;
	int				m_nRepPacketType;
	// ADD IN 1.1 >
};

#endif // !defined(AFX_SOCKSPACKET_H__04B153A0_80D4_442C_ACCE_0FA5C8ECF577__INCLUDED_)
