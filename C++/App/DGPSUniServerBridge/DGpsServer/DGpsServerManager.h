#pragma once

class CDGpsServerManager
{

private:
	// Singleton implementation, constructors are private
	CDGpsServerManager(void);
	virtual ~CDGpsServerManager(void);
	CDGpsServerManager(CDGpsServerManager const&);
	CDGpsServerManager const& operator=(CDGpsServerManager const&);

public:
	static CDGpsServerManager* instance(); // return pointer to singleton object
	void initialize(void);
private:
	WSADATA wsaData;
	SOCKET hDGPSServSock_listen;
	SOCKADDR_IN DGpsServAddr;

	SOCKET m_hTerminalServSock_listen;
	SOCKADDR_IN m_TerminalServAddr;
public:
	bool StartTerminalServer(int nPortNum);
private:
	int m_nPortTerminalServer;
	HANDLE m_TerminalAcceptThread;
    static unsigned WINAPI HandleTerminalServer(void *arg);


	bool m_threadStopTerminal;
public:
	bool StopTerminalListenThread(void);


private:
	typedef map<long ,GpsObj* > MapIDObj;
	typedef MapIDObj::iterator MapIDObj_it;
	MapIDObj m_GpsObjMap;

};
