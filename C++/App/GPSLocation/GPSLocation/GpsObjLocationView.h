#pragma once
#include "afxwin.h"



// CGpsObjLocationView view

class CGpsObjLocationView : public CScrollView
{
	DECLARE_DYNCREATE(CGpsObjLocationView)

protected:
	CGpsObjLocationView();           // protected constructor used by dynamic creation
	virtual ~CGpsObjLocationView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct

	DECLARE_MESSAGE_MAP()
private:
	CBitmap m_MemBmp;
	RECT m_clientRect,m_gridRect;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void DrawGpsObj(void);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
private:
	double m_mapScale;
public:
	afx_msg void OnStart();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


