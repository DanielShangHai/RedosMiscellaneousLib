// CodeProjectSampleView.cpp : implementation of the CCodeProjectSampleView class
//

#include "stdafx.h"
#include "CodeProjectSample.h"

#include "CodeProjectSampleDoc.h"
#include "CodeProjectSampleView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCodeProjectSampleView

IMPLEMENT_DYNCREATE(CCodeProjectSampleView, CEditView)

BEGIN_MESSAGE_MAP(CCodeProjectSampleView, CEditView)
	//{{AFX_MSG_MAP(CCodeProjectSampleView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CEditView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCodeProjectSampleView construction/destruction

CCodeProjectSampleView::CCodeProjectSampleView()
{
	// TODO: add construction code here

}

CCodeProjectSampleView::~CCodeProjectSampleView()
{
}

BOOL CCodeProjectSampleView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CEditView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CCodeProjectSampleView drawing

void CCodeProjectSampleView::OnDraw(CDC* pDC)
{
	CCodeProjectSampleDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

}

void CCodeProjectSampleView::OnInitialUpdate()
{
	CEditView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 100;
	
}

/////////////////////////////////////////////////////////////////////////////
// CCodeProjectSampleView printing

BOOL CCodeProjectSampleView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CCodeProjectSampleView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CCodeProjectSampleView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CCodeProjectSampleView diagnostics

#ifdef _DEBUG
void CCodeProjectSampleView::AssertValid() const
{
	CEditView::AssertValid();
}

void CCodeProjectSampleView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CCodeProjectSampleDoc* CCodeProjectSampleView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCodeProjectSampleDoc)));
	return (CCodeProjectSampleDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCodeProjectSampleView message handlers

void CCodeProjectSampleView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// if event is new list to display
	if(lHint==1L)
	{
		// get control
		CEdit& theEdit = GetEditCtrl();

		// retrieve text
		sListToDisplay = ((CString*)pHint)->GetBuffer(0);
		
		// set text
		theEdit.SetWindowText(sListToDisplay);

		// to be sure to display it
		this->Invalidate();
	}

}
