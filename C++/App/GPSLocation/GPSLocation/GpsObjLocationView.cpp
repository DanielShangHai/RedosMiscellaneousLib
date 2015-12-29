// GpsObjLocationView.cpp : implementation file
//

#include "stdafx.h"
#include "GPSLocation.h"
#include "GpsObjLocationView.h"


// CGpsObjLocationView

IMPLEMENT_DYNCREATE(CGpsObjLocationView, CScrollView)

CGpsObjLocationView::CGpsObjLocationView()
{

}

CGpsObjLocationView::~CGpsObjLocationView()
{
}


BEGIN_MESSAGE_MAP(CGpsObjLocationView, CScrollView)
END_MESSAGE_MAP()


// CGpsObjLocationView drawing

void CGpsObjLocationView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);
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
	//pDC->BitBlt(pos,nHeight*2/3,nWidth, nHeight/3, &MemDC, 0,0, SRCCOPY);
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
