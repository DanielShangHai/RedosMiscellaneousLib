// CodeProjectSample.h : main header file for the CODEPROJECTSAMPLE application
//

#if !defined(AFX_CODEPROJECTSAMPLE_H__5251B54C_4F7D_4057_A7C9_EB11BED3203A__INCLUDED_)
#define AFX_CODEPROJECTSAMPLE_H__5251B54C_4F7D_4057_A7C9_EB11BED3203A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CCodeProjectSampleApp:
// See CodeProjectSample.cpp for the implementation of this class
//

class CCodeProjectSampleApp : public CWinApp
{
public:
	CCodeProjectSampleApp();

//Attributes		

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCodeProjectSampleApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CCodeProjectSampleApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CODEPROJECTSAMPLE_H__5251B54C_4F7D_4057_A7C9_EB11BED3203A__INCLUDED_)
