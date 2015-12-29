#include "GpsObj.h"

GpsObj::GpsObj()
: m_IDnum(0)
, m_posLongitude(0)
, m_posLatitude(0)
, m_posHeightOnSeaLevel(0)
, m_LongitudeChar(69)  //'E'
, m_LatitudeChar(78)  //'N'
, m_54X(0)
, m_54Y(0)
, m_54H(0)
{
	;
}

GpsObj::~GpsObj(void)
{
	if(m_lpNetConnect)
	{
		delete m_lpNetConnect;
	}
}
