#pragma once

#include <vector>

using namespace std;

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

	SOCKET m_hViewServSock_listen_1;
	SOCKADDR_IN m_ViewServAddr_1;
    HANDLE m_ViewServerThread;
    bool m_threadStopView;


	SOCKET m_hViewServSock_listen_2;
	SOCKADDR_IN m_ViewServAddr_2;
	HANDLE m_View2ServerThread;
	bool m_threadStopView2;

public:
	bool StartTerminalServer(int nPortNum);
private:
	int m_nPortTerminalServer;
	int m_nPortViewServer;

	HANDLE m_TerminalAcceptThread;
    static unsigned WINAPI HandleTerminalServer(void *arg);
    static unsigned WINAPI HandleViewServer(void *arg);

	bool m_threadStopTerminal;
public:
	bool StopTerminalListenThread(void);


private:
	typedef map<long ,GpsObj* > MapIDObj;
	typedef MapIDObj::iterator MapIDObj_it;
	MapIDObj m_GpsObjMap;

	bool AddGpsObj(int nID,GpsObj* lpNewGPS);
	int m_NextGPSId;
    HANDLE hMutexP1;
	HANDLE hMutexP2;
	vector<SOCKET> m_SocketVec_P1;
	vector<SOCKET> m_SocketVec_P2;
public:
	bool StartViewServer(int nPort);
};
