#include "GpsObj.h"
#include "GPSParser.h"
#include "GpsCoordTransfer.h"

using namespace gpscord;



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
, m_pDataThread(NULL)
, m_lpNetConnect(NULL)
, m_IpAddress(_T(""))
, m_nPort(0)
, m_threadstop(true)
, m_unprocessDataLen(0)
, m_PosQuality(0)
{
	m_pDataThread = AfxBeginThread(DataCollectThreadFun, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
}

GpsObj::~GpsObj(void)
{

    HANDLE hp;
	// ½áÊøÏß³Ì
	if (m_pDataThread)
	{

		stopDataThread();
		hp=m_pDataThread->m_hThread;
		if (hp) 
		{
			if (WaitForSingleObject(hp, 1) != WAIT_OBJECT_0)
			{
				TerminateThread(hp,0);
			}
			CloseHandle(hp);
		}
	}

	if(m_lpNetConnect)
	{
		delete m_lpNetConnect;
	}
}

bool GpsObj::setNetParameters(const CString &ipAddress, long port,bool IsServer)
{
	m_IpAddress = ipAddress;
	m_nPort = port;


	if (m_lpNetConnect)
	{
		return false;
	}
    
	m_lpNetConnect = new CNetConnect(false);
	m_lpNetConnect->initialize();
    m_lpNetConnect->setTargetIpAddress(ipAddress,port);
    m_lpNetConnect->setPortNum(port);

	
	return true;
}




UINT GpsObj::DataCollectThreadFun(LPVOID pParam)
{
	GpsObj* lpLocalGps = (GpsObj*)pParam;
	
    int ObjSel = 0;
   // CSocket localSocket;
   // localSocket.Attach(lpLocalGps->m_lpNetConnect->m_socketForrAttach);
	char buff[2];
    buff[0] = 0x7e;
	CGPSParser GPSParserObj;    
	GPSParserObj.SetProtocolHead(buff,1);
	GPSParserObj.SetMessageInfoLength(12);
	char GetDataBuff[256];
	buff[0] = 0xA5;
	buff[1] = 0x5A;
    while(!(lpLocalGps->m_threadstop))
    {
		int getDataLength = lpLocalGps->m_lpNetConnect->NetReceive(GetDataBuff,256);
       // lpLocalGps->m_lpNetConnect->NetSend(buff,2); lpLocalGps->m_recDataBuff
       memcpy(&(lpLocalGps->m_recDataBuff[lpLocalGps->m_unprocessDataLen]),GetDataBuff,getDataLength);
	   lpLocalGps->m_unprocessDataLen+=getDataLength;
       lpLocalGps->m_unprocessDataLen = GPSParserObj.PreProcessData((unsigned char*)(lpLocalGps->m_recDataBuff),lpLocalGps->m_unprocessDataLen);
	   do 
	   {
	   	   int pos  = GPSParserObj.findPrococolStringHead(lpLocalGps->m_recDataBuff,lpLocalGps->m_unprocessDataLen);
		   int pos2 = GPSParserObj.findPrococolStringHead(&lpLocalGps->m_recDataBuff[1],lpLocalGps->m_unprocessDataLen-1)+1;
		   if (pos == lpLocalGps->m_unprocessDataLen) //not found
		   {
			   if (lpLocalGps->m_unprocessDataLen>=GPSParserObj.getProtocolHeadLength())
			   {
				   memcpy(lpLocalGps->m_recDataBuff,&(lpLocalGps->m_recDataBuff[lpLocalGps->m_unprocessDataLen-GPSParserObj.getProtocolHeadLength()+1]),GPSParserObj.getProtocolHeadLength());
				   lpLocalGps->m_unprocessDataLen = GPSParserObj.getProtocolHeadLength()-1;
			   }		
			   break;
		   }
		   else
		   {
			   if (pos2<lpLocalGps->m_unprocessDataLen-1) //found
			   {
				   if (pos2-pos==GPSParserObj.getProtocolHeadLength())
				   {
					   pos = pos2;//the pos2 is really start pos is previous end
				   }

			   }
			   memcpy(lpLocalGps->m_recDataBuff,&(lpLocalGps->m_recDataBuff[pos]),lpLocalGps->m_unprocessDataLen - pos);
			   lpLocalGps->m_unprocessDataLen = lpLocalGps->m_unprocessDataLen - pos;
		   }
		   
		   int messageLen = GPSParserObj.ProcessProtocolData(lpLocalGps->m_recDataBuff,lpLocalGps->m_unprocessDataLen);
		   if (messageLen>0)
		   {
			   int wholeMessageLen = GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+messageLen+1+GPSParserObj.getProtocolHeadLength();
			   if (wholeMessageLen>lpLocalGps->m_unprocessDataLen)
			   {
				   break;
			   }
			   char databuff[32];
               memcpy(databuff,&(lpLocalGps->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+4]),6);
			   if(databuff[5]&0x01!=0)
			   {
			     ObjSel = 0;
			   }
			   else
			   {
                  ObjSel = 1;
			   }

			   memcpy(databuff,&(lpLocalGps->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15]),16);
               double longitude = lpLocalGps->GPGGALangAndLatit(databuff,16);
			   if (ObjSel)
			   {
				   lpLocalGps->m_posLongitude2 = longitude;
			   }
			   else
			   {
                   lpLocalGps->m_posLongitude = longitude;
			   }
              
			   memcpy(databuff,&(lpLocalGps->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15+16]),16);
			   double latitude = lpLocalGps->GPGGALangAndLatit(databuff,16);
               
			   if (ObjSel)
			   {
				   lpLocalGps->m_posLatitude2 = latitude;
			   }
			   else
			   {
				   lpLocalGps->m_posLatitude = latitude;
			   }


			   memcpy(databuff,&(lpLocalGps->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15+16+16]),16);			  
			   databuff[16] = 0x00;
               double height1 = atof(databuff);
               
			   if (ObjSel)
			   {
				   lpLocalGps->m_posHeightOnSeaLevel2 = height1;
			   }
			   else
			   {
				   lpLocalGps->m_posHeightOnSeaLevel = height1;
			   }


			   memcpy(databuff,&(lpLocalGps->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15+16+16+16]),8);
			   databuff[8] = 0x00;
			   int tempDegree = atof(databuff);
			   
			   if (ObjSel)
			   {
				   lpLocalGps->m_54DirectorAngle2 = 3.14*((double)tempDegree)/(180.0);
			   }
			   else
			   {
				   lpLocalGps->m_54DirectorAngle = 3.14*((double)tempDegree)/(180.0);
			   }

			   char QualityByte = lpLocalGps->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+99];
			   char byteBit = 0x01;
			   for (int i=0;i<6;i++)
			   {
				   if (QualityByte&byteBit)
				   {
					   
					   if (ObjSel)
					   {
						   lpLocalGps->m_PosQuality2 = i;
					   }
					   else
					   {
						   lpLocalGps->m_PosQuality = i;
					   }
					   break;
				   }
				   byteBit<<1;
			   }

               //memcpy(databuff,&(lpLocalGps->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15+16+16+16+24]),2);
			   //int tempDegree = (int)(databuff[0])<<8+databuff[1];
               //lpLocalGps->m_54DirectorAngle = 3.14*((double)tempDegree)/(100.0*180.0);
               GpsCoordTransfer cordTransfer;
			   cordTransfer.setCoordSystemSel(C_54_SYSTEM);
			   cordTransfer.setTransferParameter(-1277560,39731830,0,1);
			   double flat_x,flat_y,flat_h;
               flat_x = 0.0;
			   flat_y = 0.0;
			   flat_h = 0.0;
			   cordTransfer.BLH2XYZ(latitude, longitude, height1, flat_x, flat_y, flat_h);
                           //.BLH2XYZ(latitude, longitude, height1, out flat_X, out flat_Y, out flat_H);
             
			   if (ObjSel)
			   {
				   lpLocalGps->m_54X2 = flat_x;
				   lpLocalGps->m_54Y2 = flat_y;
				   lpLocalGps->m_54H2 = flat_h; 
			   }
			   else
			   {
				   lpLocalGps->m_54X = flat_x;
				   lpLocalGps->m_54Y = flat_y;
				   lpLocalGps->m_54H = flat_h; 
			   }



			   memcpy(lpLocalGps->m_recDataBuff,&(lpLocalGps->m_recDataBuff[wholeMessageLen]),lpLocalGps->m_unprocessDataLen-wholeMessageLen);
			   lpLocalGps->m_unprocessDataLen-=wholeMessageLen;
		   }
		   else  //message not complete
		   {
              break;		  
		   }
       } while (1);

	   Sleep(500);
    }
    //??
	DWORD ExitCode=0;
	GetExitCodeThread( lpLocalGps->m_pDataThread->m_hThread,&ExitCode);
	if(ExitCode>0 )
		AfxEndThread(ExitCode,true);

	return 0L;
}

void GpsObj::stopDataThread(void)
{
	m_threadstop = false;
}



bool GpsObj::StartDataThread(int sel)
{

    bool connected =  m_lpNetConnect->start();
	if (!connected)
	{
		return false;
	}
	m_threadstop = false;
   // m_lpNetConnect->m_socketForrAttach = m_lpNetConnect->m_socket.Detach();
	m_pDataThread->ResumeThread();
	return true;
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
