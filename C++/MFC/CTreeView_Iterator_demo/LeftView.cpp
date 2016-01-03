// LeftView.cpp : implementation of the CLeftView class
//

#include "stdafx.h"
#include "CodeProjectSample.h"

#include "CodeProjectSampleDoc.h"
#include "CodeProjectSample.h"
#include "LeftView.h"

#include "TreeViewIterator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLeftView

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)

BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
	//{{AFX_MSG_MAP(CLeftView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CTreeView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLeftView construction/destruction

CLeftView::CLeftView()
{
	// TODO: add construction code here

}

CLeftView::~CLeftView()
{
}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CTreeView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CLeftView drawing

void CLeftView::OnDraw(CDC* pDC)
{
	CCodeProjectSampleDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}


/////////////////////////////////////////////////////////////////////////////
// CLeftView printing

BOOL CLeftView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CLeftView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CLeftView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CLeftView::OnInitialUpdate()
{
	
	// the tree object
	CTreeCtrl   &tTree = this->GetTreeCtrl ();

	// variables to compute new items
	long		lItemTitle;
	CString		sItemTitle;
	char		cItemTitle[10];
	int			iItemSize;
	int			iCptr;

	// item created
	HTREEITEM	tiTestNode;
	HTREEITEM	tiParentNode;

	CTreeView::OnInitialUpdate();

	// update the tree style
    tTree.ModifyStyle ( 0, TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT);

    
	// Add items to the tree
	for(lItemTitle=1; lItemTitle<=999; lItemTitle+=2)
	{
		ltoa(lItemTitle,cItemTitle,10);
		sItemTitle = "";
		iItemSize= strlen(cItemTitle);

		sItemTitle=sItemTitle+cItemTitle[0];

		for(iCptr=1; iCptr<iItemSize; iCptr++)
		{
			sItemTitle=sItemTitle+"."+cItemTitle[iCptr];
		}
		// insert the node
		tiParentNode = FindItem((long)(lItemTitle/10));
		//for the first level nodes
		if((long)(lItemTitle/10)==0)
		{
			tiTestNode = tTree.InsertItem(sItemTitle);
			tTree.SetItemData(tiTestNode,lItemTitle);
		}
		//for the other nodes
		if( tiParentNode )
		{
			tiTestNode = tTree.InsertItem(sItemTitle, tiParentNode);
			tTree.SetItemData(tiTestNode,lItemTitle);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLeftView diagnostics

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CLeftView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CCodeProjectSampleDoc* CLeftView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCodeProjectSampleDoc)));
	return (CCodeProjectSampleDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLeftView message handlers


//*******************************************************************
//
//  FUNCTION:   FindItem
//
//  RETURNS:    HTREEITEM, the item searched
//
//  COMMENTS:   Parse the whole nodes till those having data equal to
//				the parameter is found
//
//*******************************************************************
HTREEITEM CLeftView::FindItem(long lItemData)
{
	// the tree object
	CTreeCtrl   &tTree = this->GetTreeCtrl ();
	// the current item
    HTREEITEM    tiItem= tTree.GetNextItem(TVGN_ROOT,TVGN_ROOT);
	long	     lCurrentData;

    while (tiItem)
	{
	  tTree.Expand(tiItem,TVE_EXPAND);

	  lCurrentData = (long)tTree.GetItemData(tiItem);

	  if( lCurrentData == lItemData )
		  return tiItem;
      tiItem= tTree.GetNextItem(tiItem,TVGN_NEXTVISIBLE);
	}

	return NULL;

}

//*******************************************************************
//
//  FUNCTION:   ExternalDisplayItem
//
//  RETURNS:    int
//
//  COMMENTS:   External function to display the subtree as a list
//
//*******************************************************************
int ExternalDisplayItem (
    CLeftView   *tvTree,    /* Handle on the tree */
    HTREEITEM   tiItem )    /* Item in the tree */
{
	CTreeCtrl   &tTree = tvTree->GetTreeCtrl ();

	// store the name
	tvTree->sFullList += tTree.GetItemText(tiItem) + "\r\n";

    return ( 1);

}


//*******************************************************************
//
//  FUNCTION:   OnSelchanged
//
//  RETURNS:    void
//
//  COMMENTS:   Current selected item has changed
//
//
//*******************************************************************
void CLeftView::OnSelchanged (
    NMHDR   *pNMHDR,    /* handle on event values */
    LRESULT *pResult )  /* handle on event return value */
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *) pNMHDR;
    CTreeCtrl   &tTree = this->GetTreeCtrl ();
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	// create the iterator
    CTreeViewIterator   *ptrTree = (CTreeViewIterator *) &tTree;

	// call the function
	sFullList = "";
    ptrTree->ApplyFunction ( this, pNMTreeView->itemNew.hItem, &ExternalDisplayItem );
	
	GetDocument ()->UpdateAllViews ( this, 1L, (CObject *) &sFullList  );

    *pResult = 0;
   
}
