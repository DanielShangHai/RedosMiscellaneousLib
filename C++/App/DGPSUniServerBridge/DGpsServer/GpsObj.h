#pragma once


#include "winsock2.h"

class GpsObj
{
public:
	GpsObj(void);
	~GpsObj(void);
	static unsigned WINAPI DataThreadFunction(void *arg);


private:
	SOCKADDR_IN m_clntAddr;
	SOCKET m_hClntSock;
	HANDLE m_DataThread;
};
