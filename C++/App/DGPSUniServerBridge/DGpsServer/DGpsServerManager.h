#pragma once


#include <map>
#include <vector>
#include "GpsObj.h"


using namespace std;

class GpsObj;
typedef map<long ,GpsObj* > MapIDObj;
typedef MapIDObj::iterator MapIDObj_it;

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
	bool initialize(void);

	WSADATA wsaData;

	SOCKET hDGPSServSock_listen;
	SOCKET hDGPSServSock_Data;
	SOCKADDR_IN m_DGpsServAddr;
	HANDLE m_DGPSServerThread;
	HANDLE m_DGPSServerDataThread;
	bool m_threadStopDGps;
	bool m_threadStopDGpsData;
    int m_nPortDGPSServer;


	SOCKET m_hTerminalServSock_listen;
	SOCKADDR_IN m_TerminalServAddr;
	int m_nPortTerminalServer;
    bool m_threadStopTerminal;
    HANDLE m_TerminalAcceptThread;


	SOCKET m_hTerminalServSock_listen2;
	SOCKADDR_IN m_TerminalServAddr2;
	int m_nPortTerminalServer2;
    bool m_threadStopTerminal2;
    HANDLE m_TerminalAcceptThread2;





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
	bool StartTerminalServer2(int nPortNum);

	
	int m_nPortViewServer;
	int m_nPortViewServer2;

	
	static unsigned WINAPI HandleDGPSServer_DataThread(void *arg);
	static unsigned WINAPI HandleDGPSServer(void *arg);
    static unsigned WINAPI HandleTerminalServer(void *arg);
	static unsigned WINAPI HandleTerminalServer2(void *arg);
    static unsigned WINAPI HandleViewServer(void *arg);
	static unsigned WINAPI HandleView2Server(void *arg);

	static unsigned WINAPI HandleTestClient(void *arg);
public:
	bool StopTerminalListenThread(void);


	
	MapIDObj m_GpsObjMap;
	MapIDObj m_GpsObjMap2;

	bool AddGpsObj(int nID,GpsObj* lpNewGPS);
	bool AddGpsObj2(int nID,GpsObj* lpNewGPS);
	int m_NextGPSId;
    HANDLE hMutexP1;
	HANDLE hMutexP2;
	vector<SOCKET> m_SocketVec_P1;
	vector<SOCKET> m_SocketVec_P2;
public:
	bool StartViewServer(int nPort);
	bool StartView2Server(int nPort);
	bool StartDGpsServer(int nPortNum);
	bool SendDataToAllGPS(char* dataBuff, int len);
	HWND m_GUIHandle;
	void SetGUIHandle(HWND hd);






	SOCKET m_hclientSockForTest;
	SOCKADDR_IN m_AimServAddr;
	//int m_nPortTerminalServer;
	bool m_ThreadClientStop;
	HANDLE m_ClientTestThread;
    


};
