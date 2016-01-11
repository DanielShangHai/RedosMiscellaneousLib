// LeftView.cpp : implementation file
//

#include "stdafx.h"
#include "GPSLocation.h"
#include "LeftView.h"
#include "GpsObj.h"
#include "GpsObjectManager.h"
#include "globalobjectManager.h"
const double _PIvalue = 3.1415926535897932384626433832795;
// CLeftView


#define ID_TIMER_FLASH_POS 0x1004 //滚动定时器，用于 按住+或-，数值一直增加或减少，直到松开按键
#define TIME_INTERVAL 100 //定时时间为200毫秒

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)

CLeftView::CLeftView()
: m_pTreeCtrl(NULL)
, m_testNum(0)
{

}

CLeftView::~CLeftView()
{
}

BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
	ON_NOTIFY_REFLECT(NM_RCLICK, &CLeftView::OnNMRclick)
	ON_WM_TIMER()
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
	tvInsert.item.pszText = _T("定位船只");
    

	
	HTREEITEM hCountry = m_pTreeCtrl->InsertItem(&tvInsert);
	HTREEITEM hPA = m_pTreeCtrl->InsertItem(TVIF_TEXT,_T("船只1"), 0, 0, 0, 0, 0, hCountry, NULL);
	          m_pTreeCtrl->SetItemData(hPA,0x100);
	HTREEITEM hstatus1 = m_pTreeCtrl->InsertItem(TVIF_TEXT,_T("状态"), 0, 0, 0, 0, 0, hPA, NULL);
	          m_pTreeCtrl->SetItemData(hstatus1,0x101);
	HTREEITEM cord1 = m_pTreeCtrl->InsertItem(TVIF_TEXT,_T("经纬度"), 0, 0, 0, 0, 0, hPA, NULL);
	          m_pTreeCtrl->SetItemData(cord1,0x102);
    HTREEITEM cordx1 = m_pTreeCtrl->InsertItem(TVIF_TEXT,_T("坐标值"), 0, 0, 0, 0, 0, hPA, NULL);
	           m_pTreeCtrl->SetItemData(cordx1,0x103);


	HTREEITEM hWA = m_pTreeCtrl->InsertItem(_T("船只2"),0, 0, hCountry, hPA);
	 m_pTreeCtrl->SetItemData(hWA,0x200);
	HTREEITEM hstatus2 = m_pTreeCtrl->InsertItem(TVIF_TEXT,_T("状态"), 0, 0, 0, 0, 0, hWA, NULL);
    m_pTreeCtrl->SetItemData(hstatus2,0x201);
	HTREEITEM cord2 = m_pTreeCtrl->InsertItem(TVIF_TEXT,_T("经纬度"), 0, 0, 0, 0, 0, hWA, NULL);
	m_pTreeCtrl->SetItemData(cord2,0x202);
	HTREEITEM cordx2 = m_pTreeCtrl->InsertItem(TVIF_TEXT,_T("坐标值"), 0, 0, 0, 0, 0, hWA, NULL);
	 m_pTreeCtrl->SetItemData(cordx2,0x203);

	//HTREEITEM hDA = m_pTreeCtrl->InsertItem(_T("船只2"),0, 0, hCountry, hWA);
	//m_TreeNum[0]=(DWORD)hCountry;
	//m_TreeNum[1]=(DWORD)hPA;
	//m_TreeNum[2]=(DWORD)hWA;
	 m_testNum = 0;
	SetTimer(ID_TIMER_FLASH_POS,1000,NULL);
}

void CLeftView::OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CLeftView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	CTreeCtrl   &tTree = this->GetTreeCtrl ();
	// the current item
	HTREEITEM    tiItem= tTree.GetNextItem(TVGN_ROOT,TVGN_ROOT);
	long	     lCurrentData;

	GpsObj* terminal1 = GpsObjectManager::instance()->getGpsObject(1);
	GpsObj* terminal2 = GpsObjectManager::instance()->getGpsObject(2);
    switch(nIDEvent)
	{
	case ID_TIMER_FLASH_POS:
        if (globalobjectManager::instance()->enabledStartDraw())
        {
 			while (tiItem)
			{
				tTree.Expand(tiItem,TVE_EXPAND);

				lCurrentData = (long)tTree.GetItemData(tiItem);
				CString aa;
				
				switch(lCurrentData)
				{
				case  0x100:
					//aa.Format(_T("船只1_%d"),m_testNum%10);
					aa.Format(_T("船只1"));
					tTree.SetItemText(tiItem,aa);
					break;
				case  0x101:
					//aa.Format(_T("状态_%d"),m_testNum%10);
		            switch(terminal1->m_PosQuality)
					{
					case 0:
						aa.Format(_T("状态：未解算"));
						break;
					case 1:
						aa.Format(_T("状态：单点"));
						break;
					case 2:
						aa.Format(_T("状态：伪距差分"));
						break;
					case 3:
						aa.Format(_T("状态：未解算"));
						break;
					case 4:
						aa.Format(_T("状态：固定解"));
						break;
					case 5:
						aa.Format(_T("状态：浮点解"));
						break;
					default:
						aa.Format(_T("状态：未解算"));
						break;
					}
					
					tTree.SetItemText(tiItem,aa);
					break;
				case  0x102:
					aa.Format(_T("经纬度_%.4f E %.4f N"),180*terminal1->m_posLongitude/_PIvalue,180*terminal1->m_posLatitude/_PIvalue);
					tTree.SetItemText(tiItem,aa);
					break;
				case  0x103:
					aa.Format(_T("坐标值_X: %.2f Y: %.2f H: %.2f"),terminal1->m_54X,terminal1->m_54Y,terminal1->m_54H);
					tTree.SetItemText(tiItem,aa);
					break;
				case  0x200:
					//aa.Format(_T("船只1_%d"),m_testNum%10);
					aa.Format(_T("船只2"));
					tTree.SetItemText(tiItem,aa);
					break;
				case  0x201:
					//aa.Format(_T("状态"));
					switch(terminal1->m_PosQuality2)
					{
					case 0:
						aa.Format(_T("状态：未解算"));
						break;
					case 1:
						aa.Format(_T("状态：单点"));
						break;
					case 2:
						aa.Format(_T("状态：伪距差分"));
						break;
					case 3:
						aa.Format(_T("状态：未解算"));
						break;
					case 4:
						aa.Format(_T("状态：固定解"));
						break;
					case 5:
						aa.Format(_T("状态：浮点解"));
						break;
					default:
						aa.Format(_T("状态：未解算"));
						break;
					}
					tTree.SetItemText(tiItem,aa);
					break;
				case  0x202:
					//aa.Format(_T("经纬度_%.4f E %.4f N"),180*terminal2->m_posLongitude/_PIvalue,180*terminal2->m_posLatitude/_PIvalue);
					aa.Format(_T("经纬度_%.4f E %.4f N"),180*terminal1->m_posLongitude2/_PIvalue,180*terminal1->m_posLatitude2/_PIvalue);
					tTree.SetItemText(tiItem,aa);
					break;
				case  0x203:
					//aa.Format(_T("坐标值_X: %.2f Y: %.2f H: %.2f"),terminal2->m_54X,terminal2->m_54Y,terminal2->m_54H);
					aa.Format(_T("坐标值_X: %.2f Y: %.2f H: %.2f"),terminal1->m_54X2,terminal1->m_54Y2,terminal1->m_54H2);
					tTree.SetItemText(tiItem,aa);
					break;
				}
				tiItem= tTree.GetNextItem(tiItem,TVGN_NEXTVISIBLE);
			}
			m_testNum++;
		}
		break;
	default:
		break;
	}
	CTreeView::OnTimer(nIDEvent);
}
