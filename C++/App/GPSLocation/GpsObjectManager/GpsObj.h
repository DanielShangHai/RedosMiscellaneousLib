#pragma once
#include "NetConnect.h"
//#include "n:\work\redosmiscellaneous\redosmiscellaneouslib\c++\app\gpslocation\gpslocation\netconnect.h"


class GpsObj
{
public:
	GpsObj(void);
	~GpsObj(void);

public:
	CNetConnect* m_lpNetConnect;
	//CNetConnect  m_NetConnect;
	CWinThread* m_pDataThread;
	char m_recDataBuff[1024];
	//const int DatabuffLength = 1024;
private:
public:
	long m_IDnum;
	double m_posLongitude;
	double m_posLatitude;
	double m_posHeightOnSeaLevel;
	char m_LongitudeChar;
	char m_LatitudeChar;
	double m_54X;
	double m_54Y;
	double m_54H;
	double m_54DirectorAngle;
	
	CString m_IpAddress;
	long m_nPort;
	
public:
	bool setNetParameters(const CString &ipAddress, long port,bool IsServer);

private:
	static UINT DataCollectThreadFun(LPVOID pParam);
	
public:
	bool m_threadstop;
	void stopDataThread(void);
	bool StartDataThread(int sel);
	int m_unprocessDataLen;
	double GPGGALangAndLatit(char* buff, int len);
};
