// MyCoordTransferDlg.h : header file
//

#if !defined(AFX_MYCOORDTRANSFERDLG_H__B12651C5_C6A3_4404_B6E6_9EBAC9DB6B58__INCLUDED_)
#define AFX_MYCOORDTRANSFERDLG_H__B12651C5_C6A3_4404_B6E6_9EBAC9DB6B58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMyCoordTransferDlg dialog

class CMyCoordTransferDlg : public CDialog
{
// Construction
public:
	CMyCoordTransferDlg(CWnd* pParent = NULL);	// standard constructor
	void BLToXY(double StationB, double StationL, double *StationPx, double *StationPy, double AreaCentLong,int flagcordsys);
	double Get_atan(double z, double y);
	void OnXYZToxy(double X, double Y, double Z, double xy[],double centerLine,int cordSysflag);
	int TransBLH(double B, double L, double H, double *X, double *Y, double *Z);
// Dialog Data
	//{{AFX_DATA(CMyCoordTransferDlg)
	enum { IDD = IDD_MYCOORDTRANSFER_DIALOG };
	CComboBox	m_CmbCordSys;
	double	m_B;
	double	m_H;
	double	m_L;
	double	m_X;
	double	m_Y;
	double	m_Z;
	double	m_CenterLine;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyCoordTransferDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMyCoordTransferDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButttransfer();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYCOORDTRANSFERDLG_H__B12651C5_C6A3_4404_B6E6_9EBAC9DB6B58__INCLUDED_)
