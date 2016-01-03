//*******************************************************************
//
// FILE:       TreeViewIterator.cpp
//
// AUTHOR:     Julien Martino
//
// PROJECT:    ProjectLeading
//
// COMPONENT:  CTreeViewIterator
//
// DATE:       01/29/2002
//
// COMMENTS:   Parser, dedicated to LeftView
//
//
//*******************************************************************
//Includes
#include "stdafx.h"
#include "TreeViewIterator.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//CTreeViewIterator
//*******************************************************************
//
//  FUNCTION:   CTreeViewIterator
//
//  RETURNS:    -
//
//  COMMENTS:   Constructor
//
//
//*******************************************************************
CTreeViewIterator::CTreeViewIterator ( void )
{
}

//*******************************************************************
//
//  FUNCTION:   ~CTreeViewIterator
//
//  RETURNS:    -
//
//  COMMENTS:   Destructor
//
//
//*******************************************************************
CTreeViewIterator::~CTreeViewIterator ( void )
{
}

BEGIN_MESSAGE_MAP( CTreeViewIterator, CTreeCtrl )
//{{AFX_MSG_MAP(CTreeViewIterator)
// NOTE - the ClassWizard will add and remove mapping macros here.
//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
//CTreeViewIterator message handlers
//*******************************************************************
//
//  FUNCTION:   ApplyFunction
//
//  RETURNS:    int, 1 if ok, -1 if any problem, 0 if problem in 
//				called function
//
//  COMMENTS:   Parser, apply parameter passed function to each node
//				under the tiStart node
//
//*******************************************************************
int CTreeViewIterator::ApplyFunction (
    CLeftView   *tvView,       /* Handler on the tree view */
    HTREEITEM   tiStart,       /* Item to start with */
    FuncPtrView fptrFunction)  /* Function to launch */
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    HTREEITEM   tiCurrItem = tiStart;
    HTREEITEM   tiNextItem;
    int         iRoot = 1;
    int         iRet = 1;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    do
    {
        //here we use the current item
        if(!( *fptrFunction  ) ( tvView, tiCurrItem ))
			iRet=0;

        //then we try to get the next one
        //if it has a child
        if ( ( tiNextItem = this->GetChildItem (tiCurrItem) ) )
        {
            //we get the next one
            tiCurrItem = tiNextItem;
        }
        else
        {
            /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
            //we are on a final item, so we gotta climb up
            //to the parent to get the next one till we get one
            int iSearchChild = 1;
            /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

            //if the first item is the last one
            if ( tiCurrItem == tiStart )
            {
                iSearchChild = 0;
                iRoot = 0;
            }

            while ( iSearchChild == 1 )
            {
                if ( ( tiNextItem = this->GetParentItem (tiCurrItem) ) )
                {
                    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
                    //nextitem is the father whose we catch childs
                    HTREEITEM   tiNextChild = this->GetChildItem ( tiNextItem );
                    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

                    //we try to find the next child after curritem
                    while ( tiNextChild && tiNextChild != tiCurrItem )
                    {
                        tiNextChild = this->GetNextSiblingItem ( tiNextChild );
                    }

                    //if NextChild is not null, it should be
                    //on curritem, so we go to the next one
                    if ( tiNextChild )
                    {
                        if ( ( tiNextChild = this->GetNextSiblingItem (tiNextChild) ) )
                        {
                            //if nextchild is ok, it s the next item
                            tiCurrItem = tiNextChild;
                            iSearchChild = 0;
                        }
                        else
                        {
                            //no more child, we go up and continue
                            tiCurrItem = tiNextItem;
                        }
                    }
                    else
                    {
                        //no more child, we go up and continue
                        tiCurrItem = tiNextItem;
                    }
                }
                else
                {
                    //error, no parent found
                    iSearchChild = 2;
                    iRet = -1;
                }

                //if we r up to the start, we stop
                if ( tiCurrItem == tiStart )
                {
                    iRoot = 0;
                }
            }
        }
    } while ( iRoot && iRet == 1);
    return ( iRet );
}
