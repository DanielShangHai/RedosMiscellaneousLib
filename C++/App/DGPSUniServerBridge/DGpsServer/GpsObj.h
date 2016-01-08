#pragma once

#include "winsock2.h"
#include "DGpsServerManager.h"
#include "windows.h"

class GpsObj
{
	friend::CDGpsServerManager;
public:
	GpsObj(void);
	~GpsObj(void);
	static unsigned WINAPI DataThreadFunction(void *arg);
    void *m_ManagerHandle;
    bool m_running;
private:
	SOCKADDR_IN m_clntAddr;
	SOCKET m_hClntSock;
	HANDLE m_DataThread;
	
public:
	double m_posLongitude;
	double m_posLatitude;
	double m_posHeightOnSeaLevel;
	char m_LongitudeChar;
	char m_LatitudeChar;
	double m_54X;
	double m_54Y;
	double m_54H;
	double m_54DirectorAngle;
	int StartDataThread(void);
    double GPGGALangAndLatit(char* buff, int len);
	
	int StopDataThread(void);
	long m_LinkedPort;
	void setLinkedPort(long nPort);
	unsigned char m_recDataBuff[1024];
	unsigned int m_unprocessDataLen;
};
