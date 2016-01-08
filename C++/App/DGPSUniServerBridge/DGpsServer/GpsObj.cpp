#include "GpsObj.h"
#include <Windows.h>
#include <process.h>
#include "GPSParser.h"
#include "GpsCoordTransfer.h"


GpsObj::GpsObj(void)
:m_DataThread(0),
 m_hClntSock(0)
 , m_running(false)
 , m_ManagerHandle(NULL)
 , m_LinkedPort(0)
 , m_unprocessDataLen(0)
{
}

GpsObj::~GpsObj(void)
{
}



unsigned WINAPI DataThreadFunction(void *arg)
{
    GpsObj *lpObject = (GpsObj *)arg;
	unsigned char buff[256];
	char buff[2];
	buff[0] = 0x7e;
	CGPSParser GPSParserObj;    
	GPSParserObj.SetProtocolHead(buff,1);
	GPSParserObj.SetMessageInfoLength(12);
	while(arg->m_running)
	{
		int lc_strlen = recv(lpObject->m_hClntSock,buff,256,0);
		if (lc_strlen)
		{
			memcpy(&(lpObject->m_recDataBuff[lpObject->m_unprocessDataLen]),buff,lc_strlen);
			lpObject->m_unprocessDataLen+=lc_strlen;
			lpObject->m_unprocessDataLen = GPSParserObj.PreProcessData((unsigned char*)(lpObject->m_recDataBuff),lpObject->m_unprocessDataLen);
			do 
			{
				int pos  = GPSParserObj.findPrococolStringHead(lpObject->m_recDataBuff,lpObject->m_unprocessDataLen);
				int pos2 = GPSParserObj.findPrococolStringHead(&lpObject->m_recDataBuff[1],lpObject->m_unprocessDataLen-1);
				if (pos == lpObject->m_unprocessDataLen) //not found
				{
					if (lpObject->m_unprocessDataLen>=GPSParserObj.getProtocolHeadLength())
					{
						memcpy(lpObject->m_recDataBuff,&(lpObject->m_recDataBuff[lpObject->m_unprocessDataLen-GPSParserObj.getProtocolHeadLength()+1]),GPSParserObj.getProtocolHeadLength());
						lpObject->m_unprocessDataLen = GPSParserObj.getProtocolHeadLength()-1;
					}		
					break;
				}
				else
				{
					if (pos2<lpObject->m_unprocessDataLen-1) //found
					{
						if (pos2-pos==GPSParserObj.getProtocolHeadLength())
						{
							pos = pos2;//the pos2 is really start pos is previous end
						}

					}
					memcpy(lpObject->m_recDataBuff,&(lpObject->m_recDataBuff[pos]),lpObject->m_unprocessDataLen - pos);
					lpObject->m_unprocessDataLen = lpObject->m_unprocessDataLen - pos;
				}

				int messageLen = GPSParserObj.ProcessProtocolData(lpObject->m_recDataBuff,lpObject->m_unprocessDataLen);
				if (messageLen>0)
				{
					int wholeMessageLen = GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+messageLen+1+GPSParserObj.getProtocolHeadLength();
					if (wholeMessageLen>lpObject->m_unprocessDataLen)
					{
						break;
					}
					char databuff[32];
					memcpy(databuff,&(lpObject->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15]),16);
					double longitude = lpObject->GPGGALangAndLatit(databuff,16);
					lpObject->m_posLongitude = longitude;
					memcpy(databuff,&(lpObject->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15+16]),16);
					double latitude = lpObject->GPGGALangAndLatit(databuff,16);
					lpObject->m_posLatitude = latitude;
					memcpy(databuff,&(lpObject->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15+16+16]),16);			  
					databuff[16] = 0x00;
					double height1 = atof(databuff);
					lpObject->m_posHeightOnSeaLevel = height1;


					memcpy(databuff,&(lpObject->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15+16+16+16]),8);
					databuff[8] = 0x00;
					int tempDegree = atof(databuff);
					lpObject->m_54DirectorAngle = 3.14*((double)tempDegree)/(180.0);

					//memcpy(databuff,&(lpLocalGps->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15+16+16+16+24]),2);
					//int tempDegree = (int)(databuff[0])<<8+databuff[1];
					//lpLocalGps->m_54DirectorAngle = 3.14*((double)tempDegree)/(100.0*180.0);
					GpsCoordTransfer cordTransfer;
					cordTransfer.setCoordSystemSel(C_54_SYSTEM);
					double flat_x,flat_y,flat_h;
					flat_x = 0.0;
					flat_y = 0.0;
					flat_h = 0.0;
					cordTransfer.BLH2XYZ(latitude, longitude, height1, flat_x, flat_y, flat_h);
					//.BLH2XYZ(latitude, longitude, height1, out flat_X, out flat_Y, out flat_H);
					lpObject->m_54X = flat_x;
					lpObject->m_54Y = flat_y;
					lpObject->m_54H = flat_h;              

					memcpy(lpObject->m_recDataBuff,&(lpObject->m_recDataBuff[wholeMessageLen]),wholeMessageLen);
				}
				else  //message not complete
				{
					break;		  
				}
			} while (1);
		}
		else
		{
            arg->m_running = false;
		}
       ;
	}
	return 0L;
}
int GpsObj::StartDataThread(void)
{
	if (m_DataThread)
	{
		m_running = true;
		ResumeThread(m_DataThread);
	}
	return 0;
}

int GpsObj::StopDataThread(void)
{
	m_running = false;
	if (m_DataThread)
	{
		WaitForSingleObject( m_DataThread, INFINITE );
	}	
	return 0;
}

void GpsObj::setLinkedPort(long nPort)
{
	m_LinkedPort = nPort;
}

#define  _PIVALVE  (3.1415926535897932384626433832795)

double GpsObj::GPGGALangAndLatit(char* buff, int len)
{
	char localBuff[100];
	if (len<100)
	{
		memcpy(localBuff,buff,len);
		localBuff[len] = 0x00;
	}
	else
	{
		return 0;
	}
	double result = atof(localBuff);

	double mm = result - ((int)(result/100))*100;
	double dd = (double)((int)(result/100));
	dd = dd + mm/60;
	dd = _PIVALVE*dd/180.0;
	return dd;
}
