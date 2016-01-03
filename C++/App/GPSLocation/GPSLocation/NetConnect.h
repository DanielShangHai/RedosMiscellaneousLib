#ifndef __PREEDEFINE_NETCONNECT__REDOS
#define __PREEDEFINE_NETCONNECT__REDOS

#include "afxsock.h"
#include <winsock2.h>


class CNetConnect
{
public:
	CNetConnect(bool IsServer);
	~CNetConnect(void);
private:
	bool m_IsServer;
	
	CString m_strIPAddress;
	UINT m_Port;
	bool m_bInitialized;
	bool m_connected;


public:
	void setTargetIpAddress(const CString &strIpAddress,long portnum);
	void setPortNum(int nPortNum);
	bool initialize(void);
	bool NetConnect(void);
	bool start(void);
	int NetReceive(char* recBuff,int readNum);

	CSocket m_socket;
	int NetSend(char* sendBuff, int WriteNum);
	SOCKET m_socketForrAttach;

	WSADATA Ws;
	SOCKET m_DataClientSocket;
	struct sockaddr_in ServerAddr;
};


#endif
