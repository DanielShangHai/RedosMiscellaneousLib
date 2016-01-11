#include "DGpsServerManager.h"
#include "winsock2.h"
#include <Windows.h>
#include <process.h>

#include "GpsObj.h"


#define  DGPS_LISTEN_PORT 6011
#define  TERMINAL_LISTEN_PORT 6012
#define  TERMINAL_LISTEN_PORT2 6013

#define  VIEW_1_PORT 6014
#define  VIEW_2_PORT 6015


CDGpsServerManager::CDGpsServerManager(void)
: m_nPortTerminalServer(0)
, m_threadStopTerminal(true)
, m_NextGPSId(1)
, m_threadStopView(true)
, m_threadStopDGps(true)
, m_threadStopDGpsData(true)
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
	memset(&m_DGpsServAddr, 0, sizeof(m_DGpsServAddr));
	m_DGpsServAddr.sin_family = AF_INET;
	m_DGpsServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
 	m_DGpsServAddr.sin_port = htons(DGPS_LISTEN_PORT);
    StartDGpsServer(DGPS_LISTEN_PORT);
 


	m_hTerminalServSock_listen = socket(PF_INET, SOCK_STREAM, 0);
	if (m_hTerminalServSock_listen == INVALID_SOCKET)
	{
		//ErrorHandling("socket() error\r\n");
	}
	memset(&m_TerminalServAddr, 0, sizeof(m_TerminalServAddr));
	m_TerminalServAddr.sin_family = AF_INET;
	m_TerminalServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_TerminalServAddr.sin_port = htons(TERMINAL_LISTEN_PORT);
	StartTerminalServer(TERMINAL_LISTEN_PORT);


	m_hTerminalServSock_listen2 = socket(PF_INET, SOCK_STREAM, 0);
	if (m_hTerminalServSock_listen2 == INVALID_SOCKET)
	{
		//ErrorHandling("socket() error\r\n");
	}
	memset(&m_TerminalServAddr2, 0, sizeof(m_TerminalServAddr2));
	m_TerminalServAddr2.sin_family = AF_INET;
	m_TerminalServAddr2.sin_addr.s_addr = htonl(INADDR_ANY);
	m_TerminalServAddr2.sin_port = htons(TERMINAL_LISTEN_PORT2);
	StartTerminalServer2(TERMINAL_LISTEN_PORT2);






	m_hViewServSock_listen_1 = socket(PF_INET, SOCK_STREAM, 0);
	if (m_hViewServSock_listen_1 == INVALID_SOCKET)
	{
		//ErrorHandling("socket() error\r\n");
	}
	memset(&m_ViewServAddr_1, 0, sizeof(m_ViewServAddr_1));
	m_ViewServAddr_1.sin_family = AF_INET;
	m_ViewServAddr_1.sin_addr.s_addr = htonl(INADDR_ANY);
	m_ViewServAddr_1.sin_port = htons(TERMINAL_LISTEN_PORT);
	StartViewServer(VIEW_1_PORT);

	m_hViewServSock_listen_2 = socket(PF_INET, SOCK_STREAM, 0);
	if (m_hViewServSock_listen_2 == INVALID_SOCKET)
	{
		//ErrorHandling("socket() error\r\n");
	}
	memset(&m_ViewServAddr_2, 0, sizeof(m_ViewServAddr_2));
	m_ViewServAddr_2.sin_family = AF_INET;
	m_ViewServAddr_2.sin_addr.s_addr = htonl(INADDR_ANY);
	m_ViewServAddr_2.sin_port = htons(TERMINAL_LISTEN_PORT);
	StartView2Server(VIEW_2_PORT);




	hMutexP1 = CreateMutex(NULL,FALSE,NULL);
	hMutexP2 = CreateMutex(NULL,FALSE,NULL);







   m_hclientSockForTest = socket(PF_INET,SOCK_STREAM,0);
   memset(&m_AimServAddr,0,sizeof(m_AimServAddr));
   m_AimServAddr.sin_family = AF_INET;
   m_AimServAddr.sin_addr.s_addr = inet_addr("222.73.198.140");
   m_AimServAddr.sin_port = htons(6042);
   connect(m_hclientSockForTest,(SOCKADDR*)&m_AimServAddr,sizeof(m_AimServAddr));
   m_ClientTestThread = (HANDLE)_beginthreadex(NULL, 0, HandleTestClient, (void*)this, CREATE_SUSPENDED, NULL);

   m_ThreadClientStop = false;
   ResumeThread(m_ClientTestThread);
	return true;
}





bool CDGpsServerManager::StartDGpsServer(int nPortNum)
{

	m_DGpsServAddr.sin_port = htons(DGPS_LISTEN_PORT);
    m_nPortDGPSServer = nPortNum;
	if (bind(hDGPSServSock_listen, (SOCKADDR*)&m_DGpsServAddr, sizeof(m_DGpsServAddr)) == SOCKET_ERROR)
	{
		//ErrorHandling("bind() error\r\n");
		return false;
	}
	if (listen(hDGPSServSock_listen, 5) == SOCKET_ERROR)
	{
		//ErrorHandling("listen error \r\n");
		return false;
	}
    m_DGPSServerThread = (HANDLE)_beginthreadex(NULL, 0, HandleDGPSServer, (void*)this, CREATE_SUSPENDED, NULL);
    m_threadStopDGps = false;
	ResumeThread(m_DGPSServerThread);
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


bool CDGpsServerManager::StartTerminalServer2(int nPortNum)
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
	m_TerminalServAddr2.sin_port = htons(TERMINAL_LISTEN_PORT2);
    m_nPortTerminalServer2 = nPortNum;
	if (bind(m_hTerminalServSock_listen2, (SOCKADDR*)&m_TerminalServAddr2, sizeof(m_TerminalServAddr2)) == SOCKET_ERROR)
	{
		//ErrorHandling("bind() error\r\n");
		return false;
	}
	if (listen(m_hTerminalServSock_listen2, 5) == SOCKET_ERROR)
	{
		//ErrorHandling("listen error \r\n");
		return false;
	}
    m_TerminalAcceptThread2 = (HANDLE)_beginthreadex(NULL, 0, HandleTerminalServer2, (void*)this, CREATE_SUSPENDED, NULL);
    m_threadStopTerminal2 = false;
	ResumeThread(m_TerminalAcceptThread2);
	return true;
}




unsigned WINAPI CDGpsServerManager:: HandleDGPSServer(void *arg)
{
	CDGpsServerManager *lpServerManager = (CDGpsServerManager *)arg;
	SOCKET hLocalClntSock;
	int szClntAddr = 0;
	SOCKADDR_IN clntAddr;
	HANDLE localThreadHand;
	while (lpServerManager->m_threadStopDGps!=true)
	{

		szClntAddr = sizeof(clntAddr);
		hLocalClntSock = accept(lpServerManager->hDGPSServSock_listen, (SOCKADDR*)&clntAddr, &szClntAddr);
		if (hLocalClntSock!=INVALID_SOCKET)
		{			
			lpServerManager->hDGPSServSock_Data = hLocalClntSock;
			localThreadHand = (HANDLE)_beginthreadex(NULL, 0,CDGpsServerManager:: HandleDGPSServer_DataThread, arg, CREATE_SUSPENDED, NULL);
			if (lpServerManager->m_DGPSServerDataThread)//如果已经有 则删除
			{
				lpServerManager->m_threadStopDGpsData = true;
				closesocket(lpServerManager->hDGPSServSock_Data);
				WaitForSingleObject( lpServerManager->m_DGPSServerDataThread, 2000 );
				;//GetExitCodeThread( m_TerminalAcceptThread, &dwExitCode );
				CloseHandle( lpServerManager->m_DGPSServerDataThread );
                lpServerManager->m_DGPSServerDataThread  = NULL;

			}
			lpServerManager->m_threadStopDGpsData = false;
			lpServerManager->m_DGPSServerDataThread = localThreadHand;
			ResumeThread(localThreadHand);
			printf("connected DGPS client IP: %s \n",inet_ntoa(clntAddr.sin_addr));
		}	
	}
	return 0L;
}



unsigned WINAPI CDGpsServerManager:: HandleDGPSServer_DataThread(void *arg)
{
	CDGpsServerManager *lpServerManager = (CDGpsServerManager *)arg;
	SOCKET hLocalClntSock;
	int szClntAddr = 0;
	SOCKADDR_IN clntAddr;
	char buff[256];
	while (lpServerManager->m_threadStopDGpsData!=true)
	{
		int lc_strlen = recv(lpServerManager->hDGPSServSock_Data,buff,256,0);
		if (lc_strlen)
		{

			lpServerManager->SendDataToAllGPS(buff,lc_strlen);
		}
		else
		{
			closesocket(lpServerManager->hDGPSServSock_Data);

		}
	}
	return 0L;
}




unsigned WINAPI CDGpsServerManager::HandleTerminalServer(void *arg)
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
			newTerminal->m_DataThread = (HANDLE)_beginthreadex(NULL, 0, GpsObj::DataThreadFunction, newTerminal, CREATE_SUSPENDED, NULL);
			memcpy(&(newTerminal->m_clntAddr),&clntAddr,sizeof(clntAddr));
			newTerminal->m_hClntSock = hLocalClntSock;
			newTerminal->m_ManagerHandle = arg;
			newTerminal->m_LinkedPort = VIEW_1_PORT;
			lpServerManager->AddGpsObj(lpServerManager->m_NextGPSId++,newTerminal);
			newTerminal->StartDataThread();
			printf("connected DGPS client IP: %s \n",inet_ntoa(clntAddr.sin_addr));
        }	
	}
	return 0L;
}


unsigned WINAPI CDGpsServerManager::HandleTerminalServer2(void *arg)
{
	CDGpsServerManager *lpServerManager = (CDGpsServerManager *)arg;
	SOCKET hLocalClntSock;
	int szClntAddr = 0;
	SOCKADDR_IN clntAddr;
	while (lpServerManager->m_threadStopTerminal2!=true)
	{

		szClntAddr = sizeof(clntAddr);
		hLocalClntSock = accept(lpServerManager->m_hTerminalServSock_listen2, (SOCKADDR*)&clntAddr, &szClntAddr);
		if (hLocalClntSock!=INVALID_SOCKET)
		{
			GpsObj *newTerminal = new GpsObj();
			newTerminal->m_DataThread = (HANDLE)_beginthreadex(NULL, 0, GpsObj::DataThreadFunction, newTerminal, CREATE_SUSPENDED, NULL);
			memcpy(&(newTerminal->m_clntAddr),&clntAddr,sizeof(clntAddr));
			newTerminal->m_hClntSock = hLocalClntSock;
			newTerminal->m_ManagerHandle = arg;
			newTerminal->m_LinkedPort = VIEW_2_PORT;
			lpServerManager->AddGpsObj2(lpServerManager->m_NextGPSId++,newTerminal);
			newTerminal->StartDataThread();
			printf("connected DGPS client IP: %s \n",inet_ntoa(clntAddr.sin_addr));
		}	
	}
	return 0L;
}



bool CDGpsServerManager::StopTerminalListenThread(void)
{
	DWORD dwExitCode;
	if (m_TerminalAcceptThread)
	{
		m_threadStopTerminal  = true;
		closesocket(m_hTerminalServSock_listen);
		WaitForSingleObject( m_TerminalAcceptThread, INFINITE );
		GetExitCodeThread( m_TerminalAcceptThread, &dwExitCode );
		CloseHandle( m_TerminalAcceptThread );
		///如果成功传回TRUE，否则传回FALSE。如果失败，可以调用GetLastError()找出原因。如果线程已结束，那么线程的结束代码会被放在lpExitCode参数中带回来。如果线程尚未结束，lpExitCode带回来的值是STILL_ACTIVE。
		///在调用GetExitCodeThread()之前，要注意不要调用CloseHandle关闭掉线程句柄。GetExitCodeThread()可以在调用WaitForSingleObject()等待线程结束之后调用。
        ////Should add code to delete each GPS obj


		MapIDObj_it it = m_GpsObjMap.begin();
		while(it!=m_GpsObjMap.end())
		{
			MapIDObj_it local = it;
			it++;
			local->second->StopDataThread();
			closesocket(local->second->m_hClntSock);
			delete (GpsObj *)(local->second);            
            m_GpsObjMap.erase(local);
			
		}
	}
	return false;
}

bool CDGpsServerManager::AddGpsObj(int nID,GpsObj* lpNewGPS)
{

	int IdNum = m_GpsObjMap.count(nID);
	if (IdNum>0)
	{
		return false;
	}
	//GpsObj* lpNewGPS = new GpsObj;
	m_GpsObjMap.insert(pair<long,GpsObj*>(nID,lpNewGPS));
	//m_GpsObjMap(Id) = lpNewGPS;
	return true;
}

bool CDGpsServerManager::AddGpsObj2(int nID,GpsObj* lpNewGPS)
{

	int IdNum = m_GpsObjMap2.count(nID);
	if (IdNum>0)
	{
		return false;
	}
	//GpsObj* lpNewGPS = new GpsObj;
	m_GpsObjMap2.insert(pair<long,GpsObj*>(nID,lpNewGPS));
	//m_GpsObjMap(Id) = lpNewGPS;
	return true;
}


bool CDGpsServerManager::StartViewServer(int nPort)
{
	m_ViewServAddr_1.sin_port = htons(nPort);
	m_nPortViewServer = nPort;
	if (bind(m_hViewServSock_listen_1, (SOCKADDR*)&m_ViewServAddr_1, sizeof(m_ViewServAddr_1)) == SOCKET_ERROR)
	{
		//ErrorHandling("bind() error\r\n");
		return false;
	}
	if (listen(m_hViewServSock_listen_1, 5) == SOCKET_ERROR)
	{
		//ErrorHandling("listen error \r\n");
		return false;
	}

	m_ViewServerThread = (HANDLE)_beginthreadex(NULL, 0, HandleViewServer, (void*)this, CREATE_SUSPENDED, NULL);
    m_threadStopView = false;
	ResumeThread(m_ViewServerThread);
	return true;
}

bool CDGpsServerManager::StartView2Server(int nPort)
{
	m_ViewServAddr_2.sin_port = htons(nPort);
	m_nPortViewServer2 = nPort;
	if (bind(m_hViewServSock_listen_2, (SOCKADDR*)&m_ViewServAddr_2, sizeof(m_ViewServAddr_2)) == SOCKET_ERROR)
	{
		//ErrorHandling("bind() error\r\n");
		return false;
	}
	if (listen(m_hViewServSock_listen_2, 5) == SOCKET_ERROR)
	{
		//ErrorHandling("listen error \r\n");
		return false;
	}

	m_View2ServerThread = (HANDLE)_beginthreadex(NULL, 0, HandleView2Server, (void*)this, CREATE_SUSPENDED, NULL);
    m_threadStopView2 = false;
	ResumeThread(m_View2ServerThread);
	return true;
}




unsigned WINAPI CDGpsServerManager:: HandleViewServer(void *arg)
{
	CDGpsServerManager *lpServerManager = (CDGpsServerManager *)arg;
	SOCKET hLocalClntSock;
	int szClntAddr = 0;
	SOCKADDR_IN clntAddr;
	while (lpServerManager->m_threadStopView!=true)
	{

		szClntAddr = sizeof(clntAddr);
		hLocalClntSock = accept(lpServerManager->m_hViewServSock_listen_1, (SOCKADDR*)&clntAddr, &szClntAddr);
		if (hLocalClntSock!=INVALID_SOCKET)
		{
			WaitForSingleObject(lpServerManager->hMutexP1,INFINITE);
			lpServerManager->m_SocketVec_P1.push_back(hLocalClntSock);
			ReleaseMutex(lpServerManager->hMutexP1);
			/*
			GpsObj *newTerminal = new GpsObj();
			newTerminal->m_DataThread = (HANDLE)_beginthreadex(NULL, 0, GpsObj::DataThreadFunction, newTerminal, CREATE_SUSPENDED, NULL);
			memcpy(&(newTerminal->m_clntAddr),&clntAddr,sizeof(clntAddr));
			newTerminal->m_hClntSock = hLocalClntSock;
			newTerminal->m_ManagerHandle = arg;
			lpServerManager->AddGpsObj(lpServerManager->m_NextGPSId++,newTerminal);
			newTerminal->StartDataThread();
			printf("connected DGPS client IP: %s \n",inet_ntoa(clntAddr.sin_addr));
			*/
		}	
	}
	return 0L;
}



unsigned WINAPI  CDGpsServerManager::HandleView2Server(void *arg)
{
	CDGpsServerManager *lpServerManager = (CDGpsServerManager *)arg;
	SOCKET hLocalClntSock;
	int szClntAddr = 0;
	SOCKADDR_IN clntAddr;
	while (lpServerManager->m_threadStopView2!=true)
	{

		szClntAddr = sizeof(clntAddr);
		hLocalClntSock = accept(lpServerManager->m_hViewServSock_listen_2, (SOCKADDR*)&clntAddr, &szClntAddr);
		if (hLocalClntSock!=INVALID_SOCKET)
		{
			WaitForSingleObject(lpServerManager->hMutexP2,INFINITE);
			lpServerManager->m_SocketVec_P2.push_back(hLocalClntSock);
			ReleaseMutex(lpServerManager->hMutexP2);
			/*
			GpsObj *newTerminal = new GpsObj();
			newTerminal->m_DataThread = (HANDLE)_beginthreadex(NULL, 0, GpsObj::DataThreadFunction, newTerminal, CREATE_SUSPENDED, NULL);
			memcpy(&(newTerminal->m_clntAddr),&clntAddr,sizeof(clntAddr));
			newTerminal->m_hClntSock = hLocalClntSock;
			newTerminal->m_ManagerHandle = arg;
			lpServerManager->AddGpsObj(lpServerManager->m_NextGPSId++,newTerminal);
			newTerminal->StartDataThread();
			printf("connected DGPS client IP: %s \n",inet_ntoa(clntAddr.sin_addr));
			*/
		}	
	}
	return 0L;
}


bool CDGpsServerManager::SendDataToAllGPS(char* dataBuff, int len)
{


	int Maplen = m_GpsObjMap.size();
	MapIDObj_it it = m_GpsObjMap.begin();
	while(it!=m_GpsObjMap.end())
	{
	   int ret = it->second->SendDataToGpsTerminal(dataBuff,len);
       if (ret==0)
       {
		 
         MapIDObj_it local = it;
		 it++;
		 local->second->StopDataThread();
		 closesocket(local->second->m_hClntSock);
		 delete (GpsObj *)(local->second);
		 m_GpsObjMap.erase(local);
       }	
	   else
	   {
	      it++;
	   }
	}
	it = m_GpsObjMap2.begin();
	while(it!=m_GpsObjMap2.end())
	{
		int ret = it->second->SendDataToGpsTerminal(dataBuff,len);
		if (ret==0)
		{

			MapIDObj_it local = it;
			it++;
			local->second->StopDataThread();
			closesocket(local->second->m_hClntSock);
			delete (GpsObj *)(local->second);
			m_GpsObjMap2.erase(local);
		}	
		else
		{
			it++;
		}
	}

	return true;
}


void CDGpsServerManager::SetGUIHandle(HWND hd)
{
	m_GUIHandle = hd;
}


unsigned WINAPI  CDGpsServerManager::HandleTestClient(void *arg)
{
	CDGpsServerManager *lpServerManager = (CDGpsServerManager *)arg;
	SOCKET hLocalClntSock;
	int szClntAddr = 0;
	SOCKADDR_IN clntAddr;
	char buff[256];
	while (lpServerManager->m_ThreadClientStop!=true)
	{
		int lc_strlen = recv(lpServerManager->m_hclientSockForTest,buff,256,0);

		if (lc_strlen)
		{
              lpServerManager->SendDataToAllGPS(buff,lc_strlen);
		}
		
	}
	return 0L;
}