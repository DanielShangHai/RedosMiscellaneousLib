#include "GpsObj.h"
#include <Windows.h>
#include <process.h>
#include "GPSParser.h"
#include "GpsCoordTransfer.h"


#define  VIEW_1_PORT 6014
#define  VIEW_2_PORT 6015
#define WM_GET_TERMINAL_DATA             (WM_USER + 5)  // 
#define WM_GET_TERMINAL_DATA_2           (WM_USER + 6)  // 


using namespace gpscord;

GpsObj::GpsObj(void)
:m_DataThread(0),
 m_hClntSock(0)
 , m_running(false)
 , m_ManagerHandle(NULL)
 , m_LinkedPort(0)
 , m_unprocessDataLen(0)
 , m_PosQuality(0)
{
	m_hSocketMutex = CreateMutex(NULL,FALSE,NULL);
}

GpsObj::~GpsObj(void)
{
	CloseHandle(m_hSocketMutex);
	m_hSocketMutex = NULL;
}



unsigned WINAPI GpsObj::DataThreadFunction(void *arg)
{
    GpsObj *lpObject = (GpsObj *)arg;
	 char buff[256];
	//char buff[2];
	buff[0] = 0x7e;
	CGPSParser GPSParserObj;    
	GPSParserObj.SetProtocolHead((char *)buff,1);
	GPSParserObj.SetMessageInfoLength(12);
	CDGpsServerManager *lpManager = (CDGpsServerManager *)lpObject->m_ManagerHandle;
	char FormStringBuff[256];
	while(lpObject->m_running)
	{
		int lc_strlen = recv(lpObject->m_hClntSock,buff,256,0);

		if (lc_strlen>0)
		{

           
			if (lpObject->m_LinkedPort==VIEW_1_PORT)
			{
				WaitForSingleObject(lpManager->hMutexP1,INFINITE);
				vector<SOCKET>::iterator iter = lpManager->m_SocketVec_P1.begin();
				for( ;iter!= lpManager->m_SocketVec_P1.end();)
				{
					SOCKET localSocket = *iter;
					int ret = send(localSocket,buff,lc_strlen,0);
					vector<SOCKET>::iterator iterTemp = iter;
					iter++;
					if (ret == 0)
					{
                        closesocket(localSocket);
                        lpManager->m_SocketVec_P1.erase(iterTemp);
					}
				}
				ReleaseMutex(lpManager->hMutexP1);
				//SendMessage(lpManager->m_GUIHandle,WM_GET_TERMINAL_DATA,(WPARAM)buff,lc_strlen);
			}
			else if (lpObject->m_LinkedPort==VIEW_2_PORT)
			{
				WaitForSingleObject(lpManager->hMutexP2,INFINITE);
				vector<SOCKET>::iterator iter = lpManager->m_SocketVec_P2.begin();
				for( ;iter!= lpManager->m_SocketVec_P2.end();)
				{
					SOCKET localSocket = *iter;
					int ret = send(localSocket,buff,lc_strlen,0);
					vector<SOCKET>::iterator iterTemp = iter;
					iter++;
					if (ret == 0)
					{
						closesocket(localSocket);
						lpManager->m_SocketVec_P1.erase(iterTemp);
					}
				}
				ReleaseMutex(lpManager->hMutexP2);
				//SendMessage(lpManager->m_GUIHandle,WM_GET_TERMINAL_DATA_2,(WPARAM)buff,lc_strlen);
			}

			memcpy(&(lpObject->m_recDataBuff[lpObject->m_unprocessDataLen]),buff,lc_strlen);
			lpObject->m_unprocessDataLen+=lc_strlen;
			lpObject->m_unprocessDataLen = GPSParserObj.PreProcessData((unsigned char*)(lpObject->m_recDataBuff),lpObject->m_unprocessDataLen);
			do 
			{
				int pos  = GPSParserObj.findPrococolStringHead((char *)lpObject->m_recDataBuff,lpObject->m_unprocessDataLen);
				int pos2 = GPSParserObj.findPrococolStringHead((char *)(&lpObject->m_recDataBuff[1]),lpObject->m_unprocessDataLen-1)+1;
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

				int messageLen = GPSParserObj.ProcessProtocolData((char *)(lpObject->m_recDataBuff),lpObject->m_unprocessDataLen);
				if (messageLen>0)
				{
					int wholeMessageLen = GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+messageLen+1+GPSParserObj.getProtocolHeadLength();
					if (wholeMessageLen>lpObject->m_unprocessDataLen)
					{
						break;
					}
				    
					char databuff[32];
                    memcpy(databuff,&(lpObject->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+4]),6);
                    for (int n=0;n<6;n++)
                    {
                         FormStringBuff[2*n] = '0'+ ((databuff[n]>>4)&0x0F);
                         FormStringBuff[2*n+1] = '0'+ ((databuff[n])&0x0F);
                    }
					FormStringBuff[12] = ':';

					memcpy(databuff,&(lpObject->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15]),16);

					memcpy(&(FormStringBuff[13]),&(lpObject->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15]),16);
					for(int ll=0;ll<16;ll++)
					{
						if (FormStringBuff[13+ll]==0x00)
						{
							FormStringBuff[13+ll] += '0';
						}                        
					}
					double longitude = lpObject->GPGGALangAndLatit(databuff,16);
					lpObject->m_posLongitude = longitude;

					FormStringBuff[29] = ' ';

					memcpy(databuff,&(lpObject->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15+16]),16);
					memcpy(&(FormStringBuff[30]),databuff,16);
					for(int ll=0;ll<16;ll++)
					{
                        if (FormStringBuff[30+ll]==0x00)
                        {
							FormStringBuff[30+ll] += '0';
                        }                        
					}
					double latitude = lpObject->GPGGALangAndLatit(databuff,16);
					lpObject->m_posLatitude = latitude;
					
					FormStringBuff[46] = ' ';

					memcpy(databuff,&(lpObject->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15+16+16]),16);
					memcpy(&(FormStringBuff[47]),&(lpObject->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15+16+16]),16);		
					for(int ll=0;ll<16;ll++)
					{
						if (FormStringBuff[47+ll]==0x00)
						{
							FormStringBuff[47+ll] += '0';
						}                        
					}
					databuff[16] = 0x00;
					double height1 = atof(databuff);
					lpObject->m_posHeightOnSeaLevel = height1;

                    FormStringBuff[63] = ' ';

					memcpy(databuff,&(lpObject->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15+16+16+16]),8);
					memcpy(&(FormStringBuff[64]),&(lpObject->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15+16+16+16]),8);
					for(int ll=0;ll<8;ll++)
					{
						if (FormStringBuff[64+ll]==0x00)
						{
							FormStringBuff[64+ll] += '0';
						}                        
					}

					databuff[8] = 0x00;
					double tempDegree = atof(databuff);
					lpObject->m_54DirectorAngle = 3.14*((double)tempDegree)/(180.0);
                     
					FormStringBuff[72] = ' ';
					//memcpy(databuff,&(lpLocalGps->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+15+16+16+16+24]),2);
					//int tempDegree = (int)(databuff[0])<<8+databuff[1];
					//lpLocalGps->m_54DirectorAngle = 3.14*((double)tempDegree)/(100.0*180.0);
					
					
					char QualityByte = lpObject->m_recDataBuff[GPSParserObj.getProtocolHeadLength()+GPSParserObj.getMessageInfoLength()+99];
					char byteBit = 0x01;
					for (int i=0;i<6;i++)
					{
						if (QualityByte&byteBit)
						{
							lpObject->m_PosQuality = i;
							break;
						}
						byteBit<<1;
					}
                    
					FormStringBuff[73] = '0'+lpObject->m_PosQuality;
					FormStringBuff[74] = ' ';
					GpsCoordTransfer cordTransfer;
					cordTransfer.setCoordSystemSel(C_54_SYSTEM);
					cordTransfer.setTransferParameter(-1277560,39731830,0,1);
					double flat_x,flat_y,flat_h;
					flat_x = 0.0;
					flat_y = 0.0;
					flat_h = 0.0;
					cordTransfer.BLH2XYZ(latitude, longitude, height1, flat_x, flat_y, flat_h);
					//.BLH2XYZ(latitude, longitude, height1, out flat_X, out flat_Y, out flat_H);
					lpObject->m_54X = flat_x;
					lpObject->m_54Y = flat_y;
					lpObject->m_54H = flat_h;              
					sprintf(&FormStringBuff[75], "x:%.2f y: %.2f h: %.2f\r\n", flat_x,flat_y,flat_h);
					int slen = strlen(FormStringBuff);
					if (lpObject->m_LinkedPort==VIEW_1_PORT)
					{
						SendMessage(lpManager->m_GUIHandle,WM_GET_TERMINAL_DATA,(WPARAM)FormStringBuff,slen);
					}
					else
					{
                        //SendMessage(lpManager->m_GUIHandle,WM_GET_TERMINAL_DATA_2,(WPARAM)FormStringBuff,slen);
						SendMessage(lpManager->m_GUIHandle,WM_GET_TERMINAL_DATA,(WPARAM)FormStringBuff,slen);
					}

					memcpy(lpObject->m_recDataBuff,&(lpObject->m_recDataBuff[wholeMessageLen]),lpObject->m_unprocessDataLen-wholeMessageLen);
					lpObject->m_unprocessDataLen-=wholeMessageLen;
				}
				else  //message not complete
				{
					break;		  
				}
			} while (1);

		}
		else
		{
            lpObject->m_running = false;
		}
      Sleep(500) ;
	}//while
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


int GpsObj::SendDataToGpsTerminal(char* dataBuff, int len)
{
	WaitForSingleObject(m_hSocketMutex,INFINITE);
	int ret =send(m_hClntSock,dataBuff,len,0);
	ReleaseMutex(m_hSocketMutex);
	return ret;
}