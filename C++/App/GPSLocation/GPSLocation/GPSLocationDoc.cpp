// GPSLocationDoc.cpp : implementation of the CGPSLocationDoc class
//

#include "stdafx.h"
#include "GPSLocation.h"

#include "GPSLocationDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGPSLocationDoc

IMPLEMENT_DYNCREATE(CGPSLocationDoc, CDocument)

BEGIN_MESSAGE_MAP(CGPSLocationDoc, CDocument)
END_MESSAGE_MAP()


// CGPSLocationDoc construction/destruction

CGPSLocationDoc::CGPSLocationDoc()
{
	// TODO: add one-time construction code here

}

CGPSLocationDoc::~CGPSLocationDoc()
{
}

BOOL CGPSLocationDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CGPSLocationDoc serialization

void CGPSLocationDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CGPSLocationDoc diagnostics

#ifdef _DEBUG
void CGPSLocationDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGPSLocationDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CGPSLocationDoc commands

void CGPSLocationDoc::SetTitle(LPCTSTR lpszTitle)
{
	// TODO: Add your specialized code here and/or call the base class

	CDocument::SetTitle(lpszTitle);
	CDocument::SetTitle(_T("上海恩里克信息技术有限公司"));
}
