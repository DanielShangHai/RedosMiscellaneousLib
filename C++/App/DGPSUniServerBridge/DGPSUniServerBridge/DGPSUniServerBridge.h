// DGPSUniServerBridge.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CDGPSUniServerBridgeApp:
// �йش����ʵ�֣������ DGPSUniServerBridge.cpp
//

class CDGPSUniServerBridgeApp : public CWinApp
{
public:
	CDGPSUniServerBridgeApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CDGPSUniServerBridgeApp theApp;