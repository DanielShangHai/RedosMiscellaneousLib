#pragma once
#include "afxwin.h"
#include "atltypes.h"



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
	CPoint m_LastPoint;
	int m_ViewClickCount;

	CString X_t[13];// = {_T(" 0"),_T("20"),_T("40"),_T("60"),_T("80"),_T("100"),_T("120"),_T("140"),_T("160"),_T("180"),_T("200"),_T("220"),_T("240")};
	CString Y_t[13];// = {_T("520"),_T("500"),_T("480"),_T("460"),_T("440"),_T("420"),_T("400"),_T("380"),_T("360"),_T("340"),_T("320"),_T("300"),_T("280")};

	int m_OriginalX;
	int m_OriginaY;
	afx_msg void OnSetorigin();
	int m_CurrentGridPixs_X;
	int m_CurrentGridPixs_Y;
};


