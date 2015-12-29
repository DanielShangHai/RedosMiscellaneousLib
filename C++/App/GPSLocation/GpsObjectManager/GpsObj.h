#pragma once
#include "NetConnect.h"
//#include "n:\work\redosmiscellaneous\redosmiscellaneouslib\c++\app\gpslocation\gpslocation\netconnect.h"


class GpsObj
{
public:
	GpsObj(void);
	~GpsObj(void);
private:
	CNetConnect* m_lpNetConnect;
	long m_IDnum;
	double m_posLongitude;
	double m_posLatitude;
	double m_posHeightOnSeaLevel;
	char m_LongitudeChar;
	char m_LatitudeChar;
	double m_54X;
	double m_54Y;
	double m_54H;
	CWinThread* m_pDataThread;
	CString m_IpAddress;
	long m_nPort;
	char m_recDataBuff;
};
