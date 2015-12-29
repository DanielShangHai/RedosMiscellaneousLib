// LeftView.cpp : implementation file
//

#include "stdafx.h"
#include "GPSLocation.h"
#include "LeftView.h"


// CLeftView

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)

CLeftView::CLeftView()
: m_pTreeCtrl(NULL)
{

}

CLeftView::~CLeftView()
{
}

BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
	ON_NOTIFY_REFLECT(NM_RCLICK, &CLeftView::OnNMRclick)
END_MESSAGE_MAP()


// CLeftView diagnostics

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
	CTreeView::AssertValid();
}

#ifndef _WIN32_WCE
void CLeftView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif
#endif //_DEBUG


// CLeftView message handlers

void CLeftView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	m_pTreeCtrl=&(this->GetTreeCtrl());
	ASSERT(m_pTreeCtrl != NULL);
	TVINSERTSTRUCT tvInsert;
	tvInsert.hParent = NULL;
	tvInsert.hInsertAfter = NULL;
	tvInsert.item.mask = TVIF_TEXT;
	tvInsert.item.pszText = _T("��λ��ֻ");

	HTREEITEM hCountry = m_pTreeCtrl->InsertItem(&tvInsert);
	HTREEITEM hPA = m_pTreeCtrl->InsertItem(TVIF_TEXT,_T("��ֻ1"), 0, 0, 0, 0, 0, hCountry, NULL);
	HTREEITEM hstatus1 = m_pTreeCtrl->InsertItem(TVIF_TEXT,_T("״̬"), 0, 0, 0, 0, 0, hPA, NULL);
	HTREEITEM cord1 = m_pTreeCtrl->InsertItem(TVIF_TEXT,_T("��γ��"), 0, 0, 0, 0, 0, hPA, NULL);
    HTREEITEM cordx1 = m_pTreeCtrl->InsertItem(TVIF_TEXT,_T("����ֵ"), 0, 0, 0, 0, 0, hPA, NULL);


	HTREEITEM hWA = m_pTreeCtrl->InsertItem(_T("��ֻ2"),0, 0, hCountry, hPA);
	HTREEITEM hstatus2 = m_pTreeCtrl->InsertItem(TVIF_TEXT,_T("״̬"), 0, 0, 0, 0, 0, hWA, NULL);
	HTREEITEM cord2 = m_pTreeCtrl->InsertItem(TVIF_TEXT,_T("��γ��"), 0, 0, 0, 0, 0, hWA, NULL);
	HTREEITEM cordx2 = m_pTreeCtrl->InsertItem(TVIF_TEXT,_T("����ֵ"), 0, 0, 0, 0, 0, hWA, NULL);

	//HTREEITEM hDA = m_pTreeCtrl->InsertItem(_T("��ֻ2"),0, 0, hCountry, hWA);
	//m_TreeNum[0]=(DWORD)hCountry;
	//m_TreeNum[1]=(DWORD)hPA;
	//m_TreeNum[2]=(DWORD)hWA;
	
}

void CLeftView::OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
