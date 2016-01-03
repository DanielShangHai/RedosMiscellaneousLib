// CodeProjectSampleDoc.h : interface of the CCodeProjectSampleDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CODEPROJECTSAMPLEDOC_H__A387EC09_7D66_4862_AF5C_93447CA2980B__INCLUDED_)
#define AFX_CODEPROJECTSAMPLEDOC_H__A387EC09_7D66_4862_AF5C_93447CA2980B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CCodeProjectSampleDoc : public CDocument
{
protected: // create from serialization only
	CCodeProjectSampleDoc();
	DECLARE_DYNCREATE(CCodeProjectSampleDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCodeProjectSampleDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCodeProjectSampleDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CCodeProjectSampleDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CODEPROJECTSAMPLEDOC_H__A387EC09_7D66_4862_AF5C_93447CA2980B__INCLUDED_)
