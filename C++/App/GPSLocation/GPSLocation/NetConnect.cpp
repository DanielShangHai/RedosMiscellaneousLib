#include "StdAfx.h"
#include "NetConnect.h"
#include <iostream>

using namespace std;

CNetConnect::CNetConnect(bool IsServer)
: m_IsServer(IsServer)
, m_strIPAddress(_T(""))
, m_Port(0)
, m_bInitialized(false)
, m_connected(false)
{
}

CNetConnect::~CNetConnect(void)
{
	if (m_bInitialized)
	{
		//m_socket.Close();

		closesocket(m_DataClientSocket);
		//WSACleanup();
	}
}

void CNetConnect::setTargetIpAddress(const CString &strIpAddress,long portnum)
{
   m_strIPAddress = strIpAddress;

   ServerAddr.sin_addr.s_addr = inet_addr("192.168.1.103");
   ServerAddr.sin_port = htons(portnum);
   memset(ServerAddr.sin_zero, 0x00, 8);
}



void CNetConnect::setPortNum(int nPortNum)
{
   m_Port = nPortNum;
}

bool CNetConnect::initialize(void)
{
/*
	if (m_IsServer)
	{
		return false;
	}
	else
	{
		if (!m_bInitialized)
		{
			if (m_socket.Create())
			{
				m_bInitialized = true;
				return true;
			}
		}
	}
*/

	//Create Socket




	m_DataClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( m_DataClientSocket == INVALID_SOCKET )
	{
		cout<<"Create Socket Failed::"<<GetLastError()<<endl;
		return false;
	}	
	ServerAddr.sin_family = AF_INET;
	m_bInitialized = true;

    // linux 用fcntl 可以設置阻塞非阻塞模式，windows 下可以用setsockopt
//	flags = fcntl(sockfd, F_GETFL, 0);                        //获取文件的flags值。
//	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);   //设置成非阻塞模式；


}

bool CNetConnect::NetConnect(void)
{
	if (m_IsServer)
	{
		return false;
	}
	bool Ret = connect(m_DataClientSocket,(struct sockaddr*)&ServerAddr, sizeof(ServerAddr));
	if ( Ret == SOCKET_ERROR )
	{
		cout<<"Connect Error::"<<GetLastError()<<endl;
		return false;
	}
	else
	{
		cout<<"连接成功!"<<endl;
		return true;	
	}
	return false;
}

bool CNetConnect::start(void)
{
	if (!m_bInitialized)
	{
		return false;
	}
	if (m_IsServer)
	{
		return false;
	}
	else
	{
/*
      if(!WSAEINVAL==m_socket.Connect(m_strIPAddress,m_Port))
	  {
         return false;
	  }
	  else
	  {
          m_connected = true;
	  }
*/

		bool Ret = connect(m_DataClientSocket,(struct sockaddr*)&ServerAddr, sizeof(ServerAddr));
		if ( Ret == SOCKET_ERROR )
		{
		  cout<<"Connect Error::"<<GetLastError()<<endl;
		  return false;
		}
		else
		{
		   cout<<_T("连接成功!")<<endl;
		   m_connected = true;
		   return true;	
		}
	}

	return true;	
}

int CNetConnect::NetReceive(char* recBuff,int readNum)
{
	if (m_connected)
	{
		if (!m_IsServer)
		{
			int dwCount;
			//dwCount=m_socket.Receive(recBuff,readNum);
			//return dwCount;

			memset(recBuff, 0x00, readNum);
			int Ret = recv(m_DataClientSocket, recBuff, readNum, 0);
			if ( Ret == 0 || Ret == SOCKET_ERROR ) 
			{
			   cout<<_T("客户端退出!")<<endl;
			   return 0;
			}
			cout<<_T("接收到客户信息为:")<<recBuff<<endl;
			return Ret;

		}

	}
	return 0;
}

int CNetConnect::NetSend(char* sendBuff, int WriteNum)
{
	if (m_connected)
	{
		if (!m_IsServer)
		{
			int dwCount;
			//return m_socket.Send(sendBuff,WriteNum);
			//dwCount=m_socket.Receive(recBuff,readNum);

			return send(m_DataClientSocket, sendBuff, WriteNum, 0);
			
		}

	}
	return 0;
}
