
#include "stdio.h"
#include "stdlib.h"
#include <winsock2.h>
#include <process.h>

unsigned WINAPI ThreadFunForSocket1(void *arg);



SOCKET m_hclientSock1;
SOCKADDR_IN m_AimServAddr1;
//int m_nPortTerminalServer;
bool m_ThreadClientStop;
HANDLE m_ClientTestThread;


SOCKET m_hclientSock2;
SOCKADDR_IN m_AimServAddr2;
bool m_ThreadClientStop2;
HANDLE m_ClientTestThread2;


int main(int argc,char* argv[])
{

	WSADATA wsaData;

	WSAStartup(MAKEWORD(2,2),&wsaData);
	m_hclientSock1  = socket(PF_INET,SOCK_STREAM,0);
	memset(&m_AimServAddr1,0,sizeof(m_AimServAddr1));
	m_AimServAddr1.sin_family = AF_INET;
	m_AimServAddr1.sin_addr.s_addr = inet_addr("222.73.198.140");
	m_AimServAddr1.sin_port = htons(6042);
	connect(m_hclientSock1,(SOCKADDR*)&m_AimServAddr1,sizeof(m_AimServAddr1));
	m_ClientTestThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunForSocket1, (void *)m_hclientSock1, CREATE_SUSPENDED, NULL);

	m_ThreadClientStop = false;
	ResumeThread(m_ClientTestThread);


	m_hclientSock2  = socket(PF_INET,SOCK_STREAM,0);
	memset(&m_AimServAddr2,0,sizeof(m_AimServAddr2));
	m_AimServAddr2.sin_family = AF_INET;
	m_AimServAddr2.sin_addr.s_addr = inet_addr("127.0.0.1");
	m_AimServAddr2.sin_port = htons(6012);
	connect(m_hclientSock2,(SOCKADDR*)&m_AimServAddr2,sizeof(m_AimServAddr2));
	//m_ClientTestThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunForSocket1, (void *)m_hclientSock1, CREATE_SUSPENDED, NULL);

	//m_ThreadClientStop = false;
	//ResumeThread(m_ClientTestThread);

    while(1);
	closesocket(m_hclientSock1);
	closesocket(m_hclientSock2);
	CloseHandle(m_ClientTestThread);
    WSACleanup();
	return 0;
}



#define  BUF_SIZE 256
unsigned WINAPI ThreadFunForSocket1(void *arg)
{
	SOCKET clientSocket = (SOCKET)arg;
	char buf[BUF_SIZE];
	while(m_ThreadClientStop!=true)
	{
		int lc_ren = recv(clientSocket,buf,BUF_SIZE,0);
		if (lc_ren>0)
		{
			send(m_hclientSock2,buf,lc_ren,0);
		}

	}
	return 0L;
}
