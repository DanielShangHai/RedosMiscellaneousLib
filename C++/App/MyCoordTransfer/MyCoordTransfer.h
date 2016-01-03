// MyCoordTransfer.h : main header file for the MYCOORDTRANSFER application
//

#if !defined(AFX_MYCOORDTRANSFER_H__D89B1658_334E_41BC_8FBB_004E32A03AA4__INCLUDED_)
#define AFX_MYCOORDTRANSFER_H__D89B1658_334E_41BC_8FBB_004E32A03AA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMyCoordTransferApp:
// See MyCoordTransfer.cpp for the implementation of this class
//

class CMyCoordTransferApp : public CWinApp
{
public:
	CMyCoordTransferApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyCoordTransferApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMyCoordTransferApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYCOORDTRANSFER_H__D89B1658_334E_41BC_8FBB_004E32A03AA4__INCLUDED_)
