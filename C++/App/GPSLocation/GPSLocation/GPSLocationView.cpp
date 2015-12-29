// GPSLocationView.cpp : implementation of the CGPSLocationView class
//

#include "stdafx.h"
#include "GPSLocation.h"

#include "GPSLocationDoc.h"
#include "GPSLocationView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGPSLocationView

IMPLEMENT_DYNCREATE(CGPSLocationView, CView)

BEGIN_MESSAGE_MAP(CGPSLocationView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CGPSLocationView construction/destruction

CGPSLocationView::CGPSLocationView()
{
	// TODO: add construction code here

}

CGPSLocationView::~CGPSLocationView()
{
}

BOOL CGPSLocationView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CGPSLocationView drawing

void CGPSLocationView::OnDraw(CDC* /*pDC*/)
{
	CGPSLocationDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CGPSLocationView printing

BOOL CGPSLocationView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CGPSLocationView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CGPSLocationView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CGPSLocationView diagnostics

#ifdef _DEBUG
void CGPSLocationView::AssertValid() const
{
	CView::AssertValid();
}

void CGPSLocationView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGPSLocationDoc* CGPSLocationView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGPSLocationDoc)));
	return (CGPSLocationDoc*)m_pDocument;
}
#endif //_DEBUG


// CGPSLocationView message handlers
