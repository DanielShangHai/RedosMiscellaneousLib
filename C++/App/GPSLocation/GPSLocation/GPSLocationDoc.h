// GPSLocationDoc.h : interface of the CGPSLocationDoc class
//


#pragma once


class CGPSLocationDoc : public CDocument
{
protected: // create from serialization only
	CGPSLocationDoc();
	DECLARE_DYNCREATE(CGPSLocationDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CGPSLocationDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void SetTitle(LPCTSTR lpszTitle);
};


