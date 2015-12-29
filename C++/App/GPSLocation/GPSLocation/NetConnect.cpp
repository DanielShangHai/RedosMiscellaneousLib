#include "StdAfx.h"
#include "NetConnect.h"

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
		m_socket.Close();
	}
}

void CNetConnect::setTargetIpAddress(const CString &strIpAddress)
{
   m_strIPAddress = strIpAddress;
}



void CNetConnect::setPortNum(int nPortNum)
{
   m_Port = nPortNum;
}

bool CNetConnect::initialize(void)
{
	
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
	
}

bool CNetConnect::connect(void)
{
	if (m_IsServer)
	{
		;
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
      if(!WSAEINVAL==m_socket.Connect(m_strIPAddress,m_Port))
	  {
         return false;
	  }
	  else
	  {
          m_connected = true;
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
			dwCount=m_socket.Receive(recBuff,readNum);
		}

	}
	return 0;
}
