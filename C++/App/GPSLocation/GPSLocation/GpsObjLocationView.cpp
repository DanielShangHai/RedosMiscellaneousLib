// GpsObjLocationView.cpp : implementation file
//

#include "stdafx.h"
#include "GPSLocation.h"
#include "GpsObjLocationView.h"
#include "globalobjectManager.h"
#include "GpsObjectManager.h"
#include "GpsObj.h"
#include "math.h"


#define ID_TIMER_FLASH_POS 0x1003 //滚动定时器，用于 按住+或-，数值一直增加或减少，直到松开按键
#define TIME_INTERVAL 100 //定时时间为200毫秒


// CGpsObjLocationView

IMPLEMENT_DYNCREATE(CGpsObjLocationView, CScrollView)

CGpsObjLocationView::CGpsObjLocationView()
: m_mapScale(0)
{

}

CGpsObjLocationView::~CGpsObjLocationView()
{
}


BEGIN_MESSAGE_MAP(CGpsObjLocationView, CScrollView)
	ON_WM_TIMER()
	ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CGpsObjLocationView drawing

void CGpsObjLocationView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);
	CRect conRect;
	CDC MemDC;
	int nWidth, nHeight;

	CDC *pDC = this->GetDC();
	this->GetClientRect(conRect);
	nWidth=conRect.Width();
	nHeight=conRect.Height();

	sizeTotal.cx = 50000;
	sizeTotal.cy = nHeight;
	CSize sizePage(nWidth,0);
	CSize sizeLine(nWidth/2,0);

	pDC->SetViewportOrg(nWidth/2, nHeight/2); 

	MemDC.CreateCompatibleDC(pDC);

	//m_MemBmpWhole.CreateBitmap

	//  m_MemBmpWhole.CreateBitmap(2*nWidth,2*nHeight,1,24,NULL);
	//	m_MemBmpWhole.CreateCompatibleBitmap(pDC, nWidth, nHeight/3);
	//	ASSERT(m_MemBmpWhole.m_hObject != NULL);
	m_MemBmp.CreateCompatibleBitmap(pDC,nWidth,nHeight);


    m_mapScale = 20;


	SetTimer(ID_TIMER_FLASH_POS,TIME_INTERVAL,0);
}

void CGpsObjLocationView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
	CRect conRect;
	CDC MemDC;
	int nWidth, nHeight,i;
	double diffxyz[3];
	//CDC *pDC = this->GetDC();
	int pos=this->GetScrollPos(SB_HORZ);
	this->GetClientRect(conRect);
	CPen myPen;
	myPen.CreatePen(PS_SOLID, 1, RGB(0xff,0x00,0x00));
	nWidth=conRect.Width();
	nHeight=conRect.Height();
	MemDC.CreateCompatibleDC(pDC);
	CBitmap* pOldBitmap = MemDC.SelectObject(&m_MemBmp);	
	pDC->BitBlt(0,0,nWidth, nHeight, &MemDC, 0,0, SRCCOPY);
/*
	GetClientRect(&m_clientRect);
	//
	const wchar_t x[13][4] = {_T(" 0"),_T("20"),_T("40"),_T("60"),_T("80"),_T("100"),_T("120"),_T("140"),_T("160"),_T("180"),_T("200"),_T("220"),_T("240")};
	const wchar_t y[13][4] = {_T("520"),_T("500"),_T("480"),_T("460"),_T("440"),_T("420"),_T("400"),_T("380"),_T("360"),_T("340"),_T("320"),_T("300"),_T("280")};
	int gridXnums = 12, gridYnums = 12;//网格坐标间隔数

	m_gridRect.left  = m_clientRect.left  + 25;
	m_gridRect.top   = m_clientRect.top   + 25;
	m_gridRect.right = m_clientRect.right - 10;
	m_gridRect.bottom= m_clientRect.bottom -25;

	int dx  = (m_gridRect.right-m_gridRect.left)/gridXnums;
	int dy  = (m_gridRect.bottom-m_gridRect.top)/gridYnums;
	// Clear brush fill
	pDC->SelectStockObject(BLACK_BRUSH);
	pDC->Rectangle(&m_clientRect);
	// Set FG, BG colors and font
	//dc.SetBkColor(GetSysColor(COLOR_BTNFACE));
	pDC->SetBkMode( TRANSPARENT );
	pDC->SetTextColor(RGB( 213, 213, 213 ));
	pDC->SelectStockObject(DEFAULT_GUI_FONT);
	// Set pen parameter
	CPen thePen(PS_DOT,1,RGB(  0, 140, 240 ));
	pDC->SelectObject(thePen);

	pDC->SetTextAlign( TA_RIGHT );


	for( int i = 0; i <= gridXnums; i++ )
	{
		pDC->TextOut(m_gridRect.left+i*dx+4,m_gridRect.bottom,x[i]);

		pDC->MoveTo(m_gridRect.left+i*dx,m_gridRect.bottom);
		pDC->LineTo(m_gridRect.left+i*dx,m_gridRect.top);
	}
	for(int j = 0;j <= gridYnums; j++ )
	{
		// if (j != gridYnums )
		pDC->TextOut(m_gridRect.left,m_gridRect.top+j*dy-7,y[j]);

		pDC->MoveTo(m_gridRect.left,  m_gridRect.top+j*dy);
		pDC->LineTo(m_gridRect.right, m_gridRect.top+j*dy);
	}
	pDC->SetTextAlign( TA_CENTER );
	pDC->TextOut(m_gridRect.left+450, m_gridRect.bottom+12, _T("X-Axis (cm)"));
	pDC->TextOut(m_gridRect.left+17, m_gridRect.top-22, _T("Y-Axis (cm)"));
	pDC->TextOut(m_gridRect.left+250, m_gridRect.top-18, _T("－－User Pos Trajectory－－"));

	//plot the legend
	pDC->SetTextColor(RGB(0x2F, 0x82, 0x35));
	// Set satellite name string
	CRect legendRect;
	legendRect.left   = m_clientRect.right  -  10;
	legendRect.right  = m_clientRect.right  - 125;
	legendRect.top    = m_clientRect.top    +  10;
	legendRect.bottom = m_clientRect.top    +  100;
	pDC->TextOut(m_gridRect.left+250, m_gridRect.top-18, _T("－－User Pos Trajectory－－"));
	pDC->TextOut(m_gridRect.left+250, m_gridRect.top-18, _T("－－User Pos Trajectory－－"));
	pDC->SelectStockObject(LTGRAY_BRUSH);
	CPen thePen1(PS_SOLID, 1, RGB( 255, 255, 0));
	pDC->SelectObject(thePen1);
	pDC->Rectangle( &legendRect );
	pDC->SetTextColor(RGB(0x2F, 0x82, 0x35));
	pDC->TextOut(legendRect.left-55, legendRect.top+10, _T("Start Point:   ▲"));
	pDC->SetTextColor(RGB(243, 243, 243));
	pDC->TextOut(legendRect.left-55, legendRect.top+30, _T("Trace Point:   ＋"));
	pDC->SetTextColor(RGB(0xB7, 0x00, 0x1C));
	pDC->TextOut(legendRect.left-55, legendRect.top+50, _T("Current Point: ●"));
	pDC->SetTextColor(RGB(  0, 140, 240 ));
	pDC->TextOut(legendRect.left-55, legendRect.top+70, _T(" |______| 20cm"));
	//end of plot the legend
*/
   this->ReleaseDC(pDC);
}


// CGpsObjLocationView diagnostics

#ifdef _DEBUG
void CGpsObjLocationView::AssertValid() const
{
	CScrollView::AssertValid();
}

#ifndef _WIN32_WCE
void CGpsObjLocationView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif
#endif //_DEBUG


// CGpsObjLocationView message handlers

void CGpsObjLocationView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	switch(nIDEvent)
	{
	case ID_TIMER_FLASH_POS:
		DrawGpsObj();
		break;
	default:
		break;
	}
	CScrollView::OnTimer(nIDEvent);
}

void CGpsObjLocationView::DrawGpsObj(void)
{
	CDC MemDC;
	CPen myPen,myBlackPen;
	CRect conRect;
	CDC *pDC = this->GetDC();
	int i;
	int m;
	int deltalasttime,deltacurrtime;
	int tempt;
	int *tempint;
	double doudeltautc;
	int intdiffxyzSr[3],intdiffxyzdi[3];
	double diffxyz[3];
	int nWidth,nHeight;
	this->GetClientRect(conRect);
	myPen.CreatePen(PS_SOLID, 1, RGB(0xff,0x00,0x00));
	myBlackPen.CreatePen(PS_SOLID, 1, RGB(0x00,0x00,0x00));
	nWidth=conRect.Width();
	nHeight=conRect.Height();
	MemDC.CreateCompatibleDC(pDC);
//	CMainFrame *tempMainFrm=(CMainFrame*)AfxGetMainWnd();

	CBitmap* pOldBitmap = MemDC.SelectObject(&m_MemBmp);
	//myPen.CreatePen(PS_SOLID, 1, RGB(0x00,0x00,0x00));
	MemDC.SelectObject(&myPen);
//	MemDC.MoveTo(deltacurrtime-2,nHeight/(2*3)-intdiffxyzdi[2]);
//	MemDC.LineTo(deltacurrtime+2,nHeight/(2*3)-intdiffxyzdi[2]);
//	MemDC.MoveTo(deltacurrtime,nHeight/(2*3)-intdiffxyzdi[2]-2);
//	MemDC.LineTo(deltacurrtime,nHeight/(2*3)-intdiffxyzdi[2]+2); 
   

	//this->ReleaseDC(pDC);

	GetClientRect(&m_clientRect);
	//
	const wchar_t x[13][4] = {_T(" 0"),_T("20"),_T("40"),_T("60"),_T("80"),_T("100"),_T("120"),_T("140"),_T("160"),_T("180"),_T("200"),_T("220"),_T("240")};
	const wchar_t y[13][4] = {_T("520"),_T("500"),_T("480"),_T("460"),_T("440"),_T("420"),_T("400"),_T("380"),_T("360"),_T("340"),_T("320"),_T("300"),_T("280")};

	const CString X_t[13] = {_T(" 0"),_T("20"),_T("40"),_T("60"),_T("80"),_T("100"),_T("120"),_T("140"),_T("160"),_T("180"),_T("200"),_T("220"),_T("240")};
	const CString Y_t[13] = {_T("520"),_T("500"),_T("480"),_T("460"),_T("440"),_T("420"),_T("400"),_T("380"),_T("360"),_T("340"),_T("320"),_T("300"),_T("280")};

	int gridXnums = 12, gridYnums = 12;//网格坐标间隔数

	m_gridRect.left  = m_clientRect.left  + 25;
	m_gridRect.top   = m_clientRect.top   + 25;
	m_gridRect.right = m_clientRect.right - 10;
	m_gridRect.bottom= m_clientRect.bottom -25;

	int dx  = (m_gridRect.right-m_gridRect.left)/gridXnums;
	int dy  = (m_gridRect.bottom-m_gridRect.top)/gridYnums;
	// Clear brush fill
	MemDC.SelectStockObject(BLACK_BRUSH);
	MemDC.Rectangle(&m_clientRect);
	// Set FG, BG colors and font
	//dc.SetBkColor(GetSysColor(COLOR_BTNFACE));
	MemDC.SetBkMode( TRANSPARENT );
	MemDC.SetTextColor(RGB( 213, 213, 213 ));
	MemDC.SelectStockObject(DEFAULT_GUI_FONT);
	// Set pen parameter
	CPen thePen(PS_DOT,1,RGB(  0, 140, 240 ));
	MemDC.SelectObject(thePen);

	MemDC.SetTextAlign( TA_RIGHT );


	for( int i = 0; i <= gridXnums; i++ )
	{
		//MemDC.TextOut(m_gridRect.left+i*dx+4,m_gridRect.bottom,x[i]);
        MemDC.TextOut(m_gridRect.left+i*dx+4,m_gridRect.bottom,X_t[i]);
		MemDC.MoveTo(m_gridRect.left+i*dx,m_gridRect.bottom);
		MemDC.LineTo(m_gridRect.left+i*dx,m_gridRect.top);
	}
	for(int j = 0;j <= gridYnums; j++ )
	{
		// if (j != gridYnums )
		//MemDC.TextOut(m_gridRect.left,m_gridRect.top+j*dy-7,y[j]);
        MemDC.TextOut(m_gridRect.left,m_gridRect.top+j*dy-7,Y_t[j]);
		MemDC.MoveTo(m_gridRect.left,  m_gridRect.top+j*dy);
		MemDC.LineTo(m_gridRect.right, m_gridRect.top+j*dy);
	}
	MemDC.SetTextAlign( TA_CENTER );
	MemDC.TextOut(m_gridRect.left+450, m_gridRect.bottom+12, _T("X-Axis (cm)"));
	MemDC.TextOut(m_gridRect.left+17, m_gridRect.top-22, _T("Y-Axis (cm)"));
	MemDC.TextOut(m_gridRect.left+250, m_gridRect.top-18, _T("－－User Pos Trajectory－－"));

	//plot the legend
	MemDC.SetTextColor(RGB(0x2F, 0x82, 0x35));
	// Set satellite name string
	CRect legendRect;
	legendRect.left   = m_clientRect.right  -  10;
	legendRect.right  = m_clientRect.right  - 125;
	legendRect.top    = m_clientRect.top    +  10;
	legendRect.bottom = m_clientRect.top    +  100;
	MemDC.TextOut(m_gridRect.left+250, m_gridRect.top-18, _T("－－User Pos Trajectory－－"));
	MemDC.TextOut(m_gridRect.left+250, m_gridRect.top-18, _T("－－User Pos Trajectory－－"));
	MemDC.SelectStockObject(LTGRAY_BRUSH);
	CPen thePen1(PS_SOLID, 1, RGB( 255, 255, 0));
	MemDC.SelectObject(thePen1);
	MemDC.Rectangle( &legendRect );
	MemDC.SetTextColor(RGB(0x2F, 0x82, 0x35));
	MemDC.TextOut(legendRect.left-55, legendRect.top+10, _T("Start Point:   ▲"));
	MemDC.SetTextColor(RGB(243, 243, 243));
	MemDC.TextOut(legendRect.left-55, legendRect.top+30, _T("Trace Point:   ＋"));
	MemDC.SetTextColor(RGB(0xB7, 0x00, 0x1C));
	MemDC.TextOut(legendRect.left-55, legendRect.top+50, _T("Current Point: ●"));
	MemDC.SetTextColor(RGB(  0, 140, 240 ));
	MemDC.TextOut(legendRect.left-55, legendRect.top+70, _T(" |______| 20cm"));
	//end of plot the legend

	int arrowLength = 45;
	double arrowAngle = 3.1415926/4;
	if (globalobjectManager::instance()->enabledStartDraw())
    {
		GpsObj *gpsobj = GpsObjectManager::instance()->getGpsObject(1);
        gpsobj->m_54X;
		gpsobj->m_54Y;
		gpsobj->m_54H;
        gpsobj->m_54DirectorAngle;
		double deltaX = sin(gpsobj->m_54DirectorAngle)*arrowLength;
		double deltaY = cos(gpsobj->m_54DirectorAngle)*arrowLength;
        MemDC.Ellipse( nWidth/2, nHeight/2, nWidth/2+15,nHeight/2+15 );
        MemDC.MoveTo( nWidth/2+7, nHeight/2+8);  
        MemDC.LineTo( nWidth/2+7, nHeight/2+8-arrowLength); 
		/*
		for(int i= 0;i<2;i++)
		{
            GpsObj* lpGpsObj = GpsObjectManager::instance()->getGpsObject(1+i);
            if (lpGpsObj)
            {
				lpGpsObj->m_54X;
				lpGpsObj->m_54Y;
				lpGpsObj->m_54DirectorAngle;
            }
		}
		*/
		CPen thePen2(PS_SOLID, 1, RGB( 255, 0, 255));
		MemDC.SelectObject(thePen2);
		MemDC.Ellipse( nWidth/2+100, nHeight/2+100, nWidth/2+15+100,nHeight/2+15+100 );
		MemDC.MoveTo( nWidth/2+7+100, nHeight/2+8+100);  
		MemDC.LineTo( nWidth/2+7+100, nHeight/2+8-arrowLength+100); 
    }

    


	Graphics myg(MemDC.m_hDC);
	Pen pen(Color(255,0,255));
	SolidBrush brush(Color(255,0,255));
	myg.DrawEllipse(&pen,30,30,100,100);

	MemDC.SelectObject(pOldBitmap);
	myPen.DeleteObject();
	myBlackPen.DeleteObject();
	MemDC.DeleteDC();
	this->Invalidate();
}

void CGpsObjLocationView::OnNcPaint()
{
	// TODO: Add your message handler code here
	// Do not call CScrollView::OnNcPaint() for painting messages
}

BOOL CGpsObjLocationView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
    return TRUE;
	//return CScrollView::OnEraseBkgnd(pDC);
}

void CGpsObjLocationView::OnStart()
{
	// TODO: Add your command handler code here
	;//MessageBox(_T("In view"));
}

void CGpsObjLocationView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
    MessageBox(_T("click 1"));
	CScrollView::OnLButtonUp(nFlags, point);
}
