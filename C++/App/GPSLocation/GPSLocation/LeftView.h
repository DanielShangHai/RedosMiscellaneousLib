#pragma once


// CLeftView view

class CLeftView : public CTreeView
{
	DECLARE_DYNCREATE(CLeftView)

protected:
	CLeftView();           // protected constructor used by dynamic creation
	virtual ~CLeftView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
private:
	CTreeCtrl* m_pTreeCtrl;
public:
	afx_msg void OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult);
};


