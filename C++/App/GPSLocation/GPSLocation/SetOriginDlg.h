#pragma once


// CSetOriginDlg dialog

class CSetOriginDlg : public CDialog
{
	DECLARE_DYNAMIC(CSetOriginDlg)

public:
	CSetOriginDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSetOriginDlg();

// Dialog Data
	enum { IDD = IDD_DLG_SETORIGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_x;
	int m_y;
	double m_scale;
};
