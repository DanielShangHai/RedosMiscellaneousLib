#pragma once

class CGPSParser
{
public:
	CGPSParser(void);
	~CGPSParser(void);
	int m_CurrTemplate;



private:
	char m_ProtocolHead[16];
	int m_ProtocolHeadLength;

	

	//typedef struct tagCmdProtocol
	//{
	//	CMD CMD_ID;
	//	UINT (*ProcessFun)(unsigned char *Cmd,UINT length,VJString &Param);  
	//}CmdProtocol_t;


public:
	long PreProcessData(unsigned char* Buff,int length);
	int findPrococolStringHead(char* buff, int length);
	void SetProtocolHead(char* head, int len);
	int getProtocolHeadLength(void);
	int ProcessProtocolData(char* buff, int length);
private:
	int m_messageInfoLength;
public:
	void SetMessageInfoLength(int len);
	int getMessageInfoLength(void);
};
