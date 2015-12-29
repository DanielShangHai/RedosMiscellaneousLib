#ifndef __PREEDEFINE_NETCONNECT__REDOS
#define __PREEDEFINE_NETCONNECT__REDOS

#include "afxsock.h"
class CNetConnect
{
public:
	CNetConnect(bool IsServer);
	~CNetConnect(void);
private:
	bool m_IsServer;
	CSocket m_socket;
	CString m_strIPAddress;
	UINT m_Port;
	bool m_bInitialized;
	bool m_connected;


public:
	void setTargetIpAddress(const CString &strIpAddress);
	void setPortNum(int nPortNum);
	bool initialize(void);
	bool connect(void);
	bool start(void);
	int NetReceive(char* recBuff,int readNum);

	
};


#endif
