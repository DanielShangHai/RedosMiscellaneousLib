// CodeProjectSampleDoc.cpp : implementation of the CCodeProjectSampleDoc class
//

#include "stdafx.h"
#include "CodeProjectSample.h"

#include "CodeProjectSampleDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCodeProjectSampleDoc

IMPLEMENT_DYNCREATE(CCodeProjectSampleDoc, CDocument)

BEGIN_MESSAGE_MAP(CCodeProjectSampleDoc, CDocument)
	//{{AFX_MSG_MAP(CCodeProjectSampleDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCodeProjectSampleDoc construction/destruction

CCodeProjectSampleDoc::CCodeProjectSampleDoc()
{
	// TODO: add one-time construction code here

}

CCodeProjectSampleDoc::~CCodeProjectSampleDoc()
{
}

BOOL CCodeProjectSampleDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CCodeProjectSampleDoc serialization

void CCodeProjectSampleDoc::Serialize(CArchive& ar)
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

/////////////////////////////////////////////////////////////////////////////
// CCodeProjectSampleDoc diagnostics

#ifdef _DEBUG
void CCodeProjectSampleDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCodeProjectSampleDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCodeProjectSampleDoc commands
