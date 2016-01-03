// MyCoordTransferDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MyCoordTransfer.h"
#include "MyCoordTransferDlg.h"
#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const double _LongAXIS = 6378137.0;//84 长轴
const double _ECC   = 0.00669437999014132;//45扁率
const double _PIvalue = 3.1415926535897932384626433832795;






/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyCoordTransferDlg dialog

CMyCoordTransferDlg::CMyCoordTransferDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMyCoordTransferDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMyCoordTransferDlg)
	m_B = 31.175662222222;
	m_H = 121.398605278;
	m_L = 50.0;
	m_X = 0.0;
	m_Y = 0.0;
	m_Z = 0.0;
	m_CenterLine = 0.0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMyCoordTransferDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMyCoordTransferDlg)
	DDX_Control(pDX, IDC_COMBOCORDSYS, m_CmbCordSys);
	DDX_Text(pDX, IDC_EDITB, m_B);
	DDX_Text(pDX, IDC_EDITH, m_H);
	DDX_Text(pDX, IDC_EDITL, m_L);
	DDX_Text(pDX, IDC_EDITX, m_X);
	DDX_Text(pDX, IDC_EDITY, m_Y);
	DDX_Text(pDX, IDC_EDITZ, m_Z);
	DDX_Text(pDX, IDC_EDTCENTERLINE, m_CenterLine);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMyCoordTransferDlg, CDialog)
	//{{AFX_MSG_MAP(CMyCoordTransferDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTTRANSFER, OnButttransfer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyCoordTransferDlg message handlers

BOOL CMyCoordTransferDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_B = 31.175662222222;
	m_H = 50.0;
	m_L = 121.398605278;
	this->m_CmbCordSys.SetCurSel(0);
	this->m_CenterLine = 117;
	this->UpdateData(false);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMyCoordTransferDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMyCoordTransferDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMyCoordTransferDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}




int CMyCoordTransferDlg::TransBLH(double B, double L, double H, double *X, double *Y, double *Z)
{
	double N; 
	double cosB,cosL,sinL,sinB;
	double x;
	double y;
	double z;
	cosB=cos(B);
	cosL=cos(L);
	sinL=sin(L);
	sinB=sin(B);
	N = _LongAXIS / sqrt(1.0 - _ECC*sinB*sinB);
	*X = (N+H)*cosB*cosL;
	*Y = (N+H)*cosB*sinL;
	*Z = sinB*( N*(1 - _ECC) + H );
    return 0;
}

void CMyCoordTransferDlg::OnXYZToxy(double X, double Y, double Z, double xy[],double centerLine,int cordSysflag)
{
	double PI=3.141592653589793;
    double B;
	double B0;
	double L;
	double H;
	double N;
	
	double Px;
	double Py;
	double earth_a=6378137;//84
	//double earth_a = 6378;
	double earth_squaree=0.00669437999013;// 84扁率
	double earth_squaree1=0.00673949674227;// 84扁率

	if (cordSysflag) //54坐标系
	{
     ;
	}

	
	//XYZ至BLH
	L=Get_atan(X, Y);
	
    B=0.68;
	
	do
	{
		B0=B;
		
		N=earth_a/sqrt(1-earth_squaree*sin(B0)*sin(B0));
		
		H=Z/sin(B0)-N*(1-earth_squaree);
		
		B=Get_atan(sqrt(X*X+Y*Y)*(N*(1-earth_squaree)+H), Z*(N+H));
		
	}while(fabs(B-B0)>0.000001);
	
	H=H=Z/sin(B)-N*(1-earth_squaree);
	
	//BLH至xy(注:各地区的中央子午线各异)
	BLToXY(B/PI*180, L/PI*180, &Px, &Py, centerLine,cordSysflag);//中央子午线  117
	
    xy[0]=Px;
	xy[1]=Py;
	xy[2]=H;
}

double CMyCoordTransferDlg::Get_atan(double z, double y)
{
	double x;
	double PI=3.141592653589793;
	if(z==0)  
		x=PI/2;
	
	else if(y==0)  
		x=PI;
	
	else
	{	
		x=atan(fabs(y/z));
		
		if((y>0)&&(z<0))  
			x=PI-x;
		
		else if((y<0)&&(z<0)) 
			x=PI+x;
		
		else if((y<0)&&(z>0))  
			x=2*PI-x;
    }
	
   return x;
}


void CMyCoordTransferDlg::BLToXY(double StationB, double StationL, double *StationPx, double *StationPy, double AreaCentLong,int flagcordsys)
{
	double PI=3.141592653589793;


	//84
	/*
	const double A = 6378137;
	const double B = 6356752.3142;
	const double AAS = A * A;
	const double BBS = B * B;
	const double ES = 0.00669437999013;
	const double E1S =0.00673949674227;
	*/
	/*
	//54
	const double A = 6378245;       //椭球长半径
	const double B = 6356863;    //椭球短半径
	const double AAS = A * A;
	const double BBS = B * B;
	const double a = 1/298.3;    //  扁率
	const double ES = 2 * a - a * a;    //第一偏心率  
	const double E1S = (A*A)/(B*B) - 1;   //第二偏心率平方
	//end
	*/




	 double A = 6378137;
	 double B = 6356752.3142;
	 double AAS = A * A;
	 double BBS = B * B;
	 double ES = 0.00669437999013;
	 double E1S =0.00673949674227;
     double a;

     if (flagcordsys==1)//选取54z坐标
     {
	     A = 6378245;       //椭球长半径
	     B = 6356863;    //椭球短半径
	     AAS = A * A;
	     BBS = B * B;
	     a = 1/298.3;    //  扁率
	     ES = 2 * a - a * a;    //第一偏心率  
	     E1S = (A*A)/(B*B) - 1;   //第二偏心率平方
     }
	 double E2S = ES * ES;
	 double E3S = ES * E2S;
	 double E4S = E2S * E2S;
	 double E5S = E2S * E3S;
	 double E6S = E3S * E3S;
	 double KX = A * (1 - ES);
	 double KA1 = 1 + 3 * ES / 4 + 45 * E2S / 64 + 175 * E3S / 256 + 11025 * E4S / 16384;
	 double KA = KA1 + 43659 * E5S / 65536 + 693693 * E6S / 1048576;
	 double KB = KA - 1;
	 double KC = 15 * E2S / 32 + 175 * E3S / 384 + 3675 * E4S / 8192 + 14553 * E5S / 32768 + 231231 * E6S / 524288;
	 double KD = 35 * E3S / 96 + 735 * E4S / 2048 + 14553 * E5S / 40960 + 231231 * E6S / 655360;
	 double KE = 315 * E4S / 1024 + 6237 * E5S / 20480 + 99099 * E6S / 327680;
	 double KF = 693 * E5S / 2560 + 11011 * E6S / 40960;
	 double KG = 1001 * E6S / 4096;
	
	StationB = StationB * PI / 180 ;
	StationL = StationL * PI / 180;
	AreaCentLong = AreaCentLong * PI / 180;
	
	double SC = sin(StationB) * cos(StationB);
	double SS = sin(StationB) * sin(StationB);
	double S2S = SS * SS;
	double S3S = SS * S2S;
	double XA0 = KX * KA * StationB - KX * SC * (KB + SS * (KC + SS * (KD + KE * SS + KF * S2S + KG * S3S)));
	double T = tan(StationB);
	double StationLd = StationL - AreaCentLong;
	double TS = T * T;
	double T2S = TS * TS;
	double T3S = TS * T2S;
	double CS = cos(StationB) * cos(StationB);
	double NR = AAS / sqrt(AAS * CS + BBS * SS);
	double NAS = E1S * CS;
	double N2S = NAS * NAS;
	double LDS = StationLd * StationLd;
	double FA = NR * SC * LDS;
	double FB = CS * LDS;
	double FC = NR * cos(StationB) * StationLd;
	double FD = (5 - TS + 9 * NAS + 4 * N2S) / 24;
	double FF = (61 - 58 * TS + T2S + 270 * NAS - 330 * NAS * TS) / 720;
	double FG = (1385 - 3111 * TS + 543 * T2S - T3S) / 40320;
	double FH = (1 - TS + NAS) / 6;
	double FI = (5 - 18 * TS + T2S + 14 * NAS - 58 * NAS * TS) / 120;
	double FJ = (61 - 479 * TS + 179 * T2S - T3S) / 5040;
	*StationPx = XA0 + FA * (0.5 + FB * (FD + FB * (FF + FB * FG)));
	*StationPy = FC * (1 + FB * (FH + FB * (FI + FB * FJ)));
	*StationPy =  *StationPy + 500000;
}



void CMyCoordTransferDlg::OnButttransfer() 
{
	// TODO: Add your control notification handler code here
	double X,Y,Z;
	double xyz[3];
	this->UpdateData(true);

	
	this->TransBLH(m_B*(_PIvalue/180),m_L*(_PIvalue/180),m_H,&X,&Y,&Z);
	this->OnXYZToxy(X,Y,Z,xyz,this->m_CenterLine,this->m_CmbCordSys.GetCurSel());
	X=xyz[0];
	Y=xyz[1];
	Z=xyz[2];
	m_X = X;
	m_Y = Y;
	m_Z	= Z;
	this->UpdateData(false);
}
