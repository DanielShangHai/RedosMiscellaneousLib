//*******************************************************************
//
// FILE:       TreeViewIterator.h
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
#if !defined( AFX_TREEVIEWITERATOR_H__7014EA35_A94A_4E0B_8CD5_A5881FF2D056__INCLUDED_ )
    #define AFX_TREEVIEWITERATOR_H__7014EA35_A94A_4E0B_8CD5_A5881FF2D056__INCLUDED_

    #include "LeftView.h"

    #if _MSC_VER > 1000
        #pragma once
    #endif //_MSC_VER > 1000

//TreeViewIterator.h : header file
//
//
/////////////////////////////////////////////////////////////////////////////////
//CTreeViewIterator window
typedef int ( *FuncPtrView ) ( CLeftView * tTree, HTREEITEM tiItem );

//*******************************************************************
// CLASS		: CTreeViewIterator
// COMMENTS     : Parser, dedicated to LeftView
// CREATED BY	: Julien Martino
// DATE         : 01/29/2002
//*******************************************************************
class CTreeViewIterator :
    public CTreeCtrl
{
    //Constructor
    public:

		CTreeViewIterator ( void );

    
	//Attributes
    public:


    //Operations
    public:

        int ApplyFunction ( CLeftView *tvView, HTREEITEM tiStart, FuncPtrView fptrFunction );

        //Overrides
        //ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CTreeViewIterator)
    //}}AFX_VIRTUAL

	//Implementation
    public:

        virtual ~CTreeViewIterator ( void );

    //Generated message map functions

    protected:
    //{{AFX_MSG(CTreeViewIterator)
    // NOTE - the ClassWizard will add and remove member functions here.
    //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
//Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#endif //!defined(AFX_TREEVIEWITERATOR_H__7014EA35_A94A_4E0B_8CD5_A5881FF2D056__INCLUDED_)
