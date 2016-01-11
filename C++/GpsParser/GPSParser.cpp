#include "GPSParser.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>

CGPSParser::CGPSParser(void)
: m_CurrTemplate(0)
, m_ProtocolHeadLength(1)
, m_messageInfoLength(0)
{
}

CGPSParser::~CGPSParser(void)
{
	m_ProtocolHead[0] = 0x7e;
}



long CGPSParser::PreProcessData(unsigned char* Buff,int length)
{
	//int length = *lpLength;
	int lengthM = length;
    for (int i=0;i<length-1;i++)
    {
       if (Buff[i] == 0x7d)
       {
		   if (Buff[i+1] == 0x01)
		   {
			   Buff[i] = 0x7d;
			   if (i<length-2)
			   {
				   memcpy(&Buff[i+1],&(Buff[i+2]),length-i-2);
                   lengthM--;
			   }			   
		   }
		   else if(Buff[i+1] == 0x02)
		   {
               Buff[i] = 0x7e;
			   if (i<length-2)
			   {
				   memcpy(&Buff[i+1],&(Buff[i+2]),length-i-2);
				   lengthM--;
			   }
		   }
       }
    }
	return lengthM;

}
int CGPSParser::findPrococolStringHead(char* buff, int length)
{
	int pos = length;
	bool found = false;
	bool different = false;
	if (length<m_ProtocolHeadLength)
	{
		return length;
	}
    for (int i = 0;i<length-m_ProtocolHeadLength+1;i++)
    {
	   found = false;
	   different = false;
	   int j;
	   for (j=0;j<m_ProtocolHeadLength;j++)
	   {
	      if (buff[i+j]!=m_ProtocolHead[j])
		  {
			  different = true;
			  break;
		  }
	    }
	   if ((j==m_ProtocolHeadLength)&&(!different))
	   {
		   return i;
	   }


     }
     return length;

}

void CGPSParser::SetProtocolHead(char* head, int len)
{
	//ASSERT(len>=16);
	memcpy(this->m_ProtocolHead,head,len);
	m_ProtocolHeadLength = len;
}

int CGPSParser::getProtocolHeadLength(void)
{
	return m_ProtocolHeadLength;
}

int CGPSParser::ProcessProtocolData(char* buff, int length)
{
	if (length<m_messageInfoLength)
	{
		return 0;
	}
	int messageID,messageLength;
    messageID = ((int)buff[1])<<8+(int)buff[2];
	messageLength = (((int)buff[3])<<8)+(int)buff[4];
	messageLength &= 0x03FF;
	unsigned char TerminalID[6];
	unsigned char packetMode = buff[3]&0x20;
    for (int i=0;i<6;i++)
    {
		TerminalID[i] = buff[5+i];
    }
    if (packetMode)
    {
		;
    }
    if (messageLength>length)
    {
		return 0;
    }

	return messageLength;
}

void CGPSParser::SetMessageInfoLength(int len)
{
   m_messageInfoLength = len;
}

int CGPSParser::getMessageInfoLength(void)
{
	return m_messageInfoLength;
}
