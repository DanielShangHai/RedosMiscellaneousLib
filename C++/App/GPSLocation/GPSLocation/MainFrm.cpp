// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "GPSLocation.h"
#include "LeftView.h"
#include "MainFrm.h"
#include "GpsObjLocationView.h"
#include "GpsObj.h"
#include "GpsObjectManager.h"
#include "globalobjectManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//const CString IP_Address1 = _T("222.73.198.140");
const CString IP_Address1 = _T("127.0.0.1");
const long IP_Port = 6014;
const long IP_Port2 = 6015;

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND(ID_START, &CMainFrame::OnStart)
	ON_UPDATE_COMMAND_UI(ID_SETORIGIN, &CMainFrame::OnUpdateSetorigin)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	//m_lpRview=this->GetRightPane();
	//m_connectMenu=1;
	//m_Portconnect=1;
	//m_menuflagforsigma=0;
	//m_menuflagstopsigma=1;

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers




BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// TODO: Add your specialized code here and/or call the base class
	// create splitter window
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
		return FALSE;

	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CLeftView), CSize(200, 100), pContext) ||
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CGpsObjLocationView), CSize(100, 100), pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}
   
	//return CFrameWnd::OnCreateClient(lpcs, pContext);
}

void CMainFrame::OnStart()
{
	// TODO: Add your command handler code here
	AfxSocketInit();
	WSADATA Ws;
	if ( WSAStartup(MAKEWORD(2,2), &Ws) != 0 )
	{
		cout<<"Init Windows Socket Failed::"<<GetLastError()<<endl;
		return;
	}
	GpsObjectManager::instance()->initialise();
	GpsObjectManager::instance()->AddGpsObj(1);
	GpsObjectManager::instance()->AddGpsObj(2);
	GpsObjectManager::instance()->getGpsObject(1)->setNetParameters(IP_Address1,IP_Port,false);
	GpsObjectManager::instance()->getGpsObject(1)->StartDataThread(0);


	//GpsObjectManager::instance()->getGpsObject(2)->setNetParameters(IP_Address1,IP_Port2,false);
	//GpsObjectManager::instance()->getGpsObject(2)->StartDataThread(0);


	//GpsObjectManager::instance()->getGpsObject(2)->setNetParameters("222.73.198.140",6011);
	globalobjectManager::instance()->enableStartDraw(true);
	MessageBox(_T("Æô¶¯"));
}

void CMainFrame::OnUpdateSetorigin(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(TRUE);
}
