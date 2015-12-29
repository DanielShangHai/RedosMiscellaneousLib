// GPSLocation.h : main header file for the GPSLocation application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CGPSLocationApp:
// See GPSLocation.cpp for the implementation of this class
//

class CGPSLocationApp : public CWinApp
{
public:
	CGPSLocationApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CGPSLocationApp theApp;