// SetOriginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GPSLocation.h"
#include "SetOriginDlg.h"


// CSetOriginDlg dialog

IMPLEMENT_DYNAMIC(CSetOriginDlg, CDialog)

CSetOriginDlg::CSetOriginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetOriginDlg::IDD, pParent)
	, m_x(0)
	, m_y(0)
	, m_scale(0)
{

}

CSetOriginDlg::~CSetOriginDlg()
{
}

void CSetOriginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDT_X, m_x);
	DDX_Text(pDX, IDC_EDT_Y, m_y);
	DDX_Text(pDX, IDC_EDT_SCALE, m_scale);
}


BEGIN_MESSAGE_MAP(CSetOriginDlg, CDialog)
END_MESSAGE_MAP()


// CSetOriginDlg message handlers
