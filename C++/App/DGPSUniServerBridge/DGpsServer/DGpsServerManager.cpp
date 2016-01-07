#include "DGpsServerManager.h"
#include "winsock2.h"
#include <Windows.h>
#include <process.h>

#include "GpsObj.h"


#define  DGPS_LISTEN_PORT 6011
#define  TERMINAL_LISTEN_PORT 6012

CDGpsServerManager::CDGpsServerManager(void)
: m_nPortTerminalServer(0)
, m_threadStopTerminal(true)
{
   ;
}

CDGpsServerManager::~CDGpsServerManager(void)
{
	
    WSACleanup();
}


CDGpsServerManager* CDGpsServerManager::instance()
{
	static CDGpsServerManager theInstance;

	return &theInstance;
}

bool CDGpsServerManager::initialize(void)
{
	if (WSAStartup(MAKEWORD(2,2),&wsaData)!=0)
	{
		//ErrorHandling("WSAStartup() error!\r\n");
		return false;
	}


    hDGPSServSock_listen = socket(PF_INET, SOCK_STREAM, 0);
	if (hDGPSServSock_listen == INVALID_SOCKET)
	{
		//ErrorHandling("socket() error\r\n");
	}
	memset(&DGpsServAddr, 0, sizeof(DGpsServAddr));
	DGpsServAddr.sin_family = AF_INET;
	DGpsServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	DGpsServAddr.sin_port = htons(DGPS_LISTEN_PORT);


	hTerminalServSock_listen = socket(PF_INET, SOCK_STREAM, 0);
	if (hTerminalServSock_listen == INVALID_SOCKET)
	{
		//ErrorHandling("socket() error\r\n");
	}
	memset(&TerminalServAddr, 0, sizeof(TerminalServAddr));
	TerminalServAddr.sin_family = AF_INET;
	TerminalServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	TerminalServAddr.sin_port = htons(TERMINAL_LISTEN_PORT);


	return true;
}

bool CDGpsServerManager::StartTerminalServer(int nPortNum)
{

/*
	hTerminalServSock_listen = socket(PF_INET, SOCK_STREAM, 0);
	if (hTerminalServSock_listen == INVALID_SOCKET)
	{
		//ErrorHandling("socket() error\r\n");
	}
	memset(&TerminalServAddr, 0, sizeof(TerminalServAddr));
	TerminalServAddr.sin_family = AF_INET;
	TerminalServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
*/
	m_TerminalServAddr.sin_port = htons(TERMINAL_LISTEN_PORT);
    m_nPortTerminalServer = nPortNum;
	if (bind(m_hTerminalServSock_listen, (SOCKADDR*)&m_TerminalServAddr, sizeof(m_TerminalServAddr)) == SOCKET_ERROR)
	{
		//ErrorHandling("bind() error\r\n");
		return false;
	}
	if (listen(m_hTerminalServSock_listen, 5) == SOCKET_ERROR)
	{
		//ErrorHandling("listen error \r\n");
		return false;
	}
    m_TerminalAcceptThread = (HANDLE)_beginthreadex(NULL, 0, HandleTerminalServer, (void*)this, CREATE_SUSPENDED, NULL);
    m_threadStopTerminal = false;
	ResumeThread(m_TerminalAcceptThread);
	return true;
}



unsigned WINAPI HandleTerminalServer(void *arg)
{
	CDGpsServerManager *lpServerManager = (CDGpsServerManager *)arg;
	SOCKET hLocalClntSock;
	int szClntAddr = 0;
	SOCKADDR_IN clntAddr;
	while (lpServerManager->m_threadStopTerminal!=true)
	{
		
		szClntAddr = sizeof(clntAddr);
		hLocalClntSock = accept(lpServerManager->m_hTerminalServSock_listen, (SOCKADDR*)&clntAddr, &szClntAddr);
        if (hLocalClntSock!=INVALID_SOCKET)
        {
			GpsObj *newTerminal = new GpsObj();
			newTerminal->m_DataThread = (HANDLE)_beginthreadex(NULL, 0, GpsObj::DataThreadFunction, (void*)&arg, 0, NULL);
			memcpy(&(newTerminal->m_clntAddr),&clntAddr,sizeof(clntAddr));
			printf("connected DGPS client IP: %s \n",inet_ntoa(clntAddr.sin_addr));

        }	
	}
}
bool CDGpsServerManager::StopTerminalListenThread(void)
{
	DWORD dwExitCode;
	if (m_TerminalAcceptThread)
	{
		m_threadStopTerminal  = true;
		CloseHandle(m_hTerminalServSock_listen);
		WaitForSingleObject( m_TerminalAcceptThread, INFINITE );
		GetExitCodeThread( m_TerminalAcceptThread, &dwExitCode );
		CloseHandle( m_TerminalAcceptThread );
		///如果成功传回TRUE，否则传回FALSE。如果失败，可以调用GetLastError()找出原因。如果线程已结束，那么线程的结束代码会被放在lpExitCode参数中带回来。如果线程尚未结束，lpExitCode带回来的值是STILL_ACTIVE。
		///在调用GetExitCodeThread()之前，要注意不要调用CloseHandle关闭掉线程句柄。GetExitCodeThread()可以在调用WaitForSingleObject()等待线程结束之后调用。


        ////Should add code to delete each GPS obj


	}
	return false;
}
