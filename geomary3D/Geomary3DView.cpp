// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// Geomary3DView.cpp : implementation of the CGeomary3DView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Geomary3D.h"
#endif

#include "Geomary3DDoc.h"
#include "Geomary3DView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGeomary3DView

IMPLEMENT_DYNCREATE(CGeomary3DView, CView)

BEGIN_MESSAGE_MAP(CGeomary3DView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_CREATE()
//	ON_WM_CLOSE()
//ON_WM_MOUSEHWHEEL()
ON_WM_MOUSEWHEEL()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_CAPTURECHANGED()
ON_COMMAND(ID_BOOL_UNION, &CGeomary3DView::OnBoolUnion)
ON_UPDATE_COMMAND_UI(ID_BOOL_UNION, &CGeomary3DView::OnUpdateBoolUnion)
ON_UPDATE_COMMAND_UI(ID_BOOL_INTERSECT, &CGeomary3DView::OnUpdateBoolIntersect)
ON_COMMAND(ID_BOOL_INTERSECT, &CGeomary3DView::OnBoolIntersect)
ON_COMMAND(ID_BOOL_DIFFERENCE, &CGeomary3DView::OnBoolDifference)
ON_UPDATE_COMMAND_UI(ID_BOOL_DIFFERENCE, &CGeomary3DView::OnUpdateBoolDifference)
ON_COMMAND(ID_VISUALSTYLE, &CGeomary3DView::OnVisualstyle)
ON_COMMAND(ID_BOOL_EXPRESSION, &CGeomary3DView::OnBoolExpression)
ON_UPDATE_COMMAND_UI(ID_BOOL_EXPRESSION, &CGeomary3DView::OnUpdateBoolExpression)
ON_COMMAND(ID_BSP_UNION, &CGeomary3DView::OnBspBooleanUnion)
ON_COMMAND(ID_BSP_INTERSECT, &CGeomary3DView::OnBspBooleanIntersect)
ON_COMMAND(ID_BSP_DIFFERENCE, &CGeomary3DView::OnBspBooleanDifference)
ON_UPDATE_COMMAND_UI(ID_BSP_UNION, &CGeomary3DView::OnUpdateBspBooleanUnion)
ON_UPDATE_COMMAND_UI(ID_BSP_INTERSECT, &CGeomary3DView::OnUpdateBspBooleanIntersect)
ON_UPDATE_COMMAND_UI(ID_BSP_DIFFERENCE, &CGeomary3DView::OnUpdateBspBooleanDifference)
ON_COMMAND(ID_LOCALIZED_BSP_UNION, &CGeomary3DView::OnLocalizedBspUnion)
ON_UPDATE_COMMAND_UI(ID_LOCALIZED_BSP_UNION, &CGeomary3DView::OnUpdateLocalizedBspUnion)
ON_COMMAND(ID_LOCALIZED_BSP_INTERSECT, &CGeomary3DView::OnLocalizedBspIntersect)
ON_UPDATE_COMMAND_UI(ID_LOCALIZED_BSP_INTERSECT, &CGeomary3DView::OnUpdateLocalizedBspIntersect)
ON_COMMAND(ID_LOCALIZED_BSP_DIFFERENCE, &CGeomary3DView::OnLocalizedBspDifference)
ON_UPDATE_COMMAND_UI(ID_LOCALIZED_BSP_DIFFERENCE, &CGeomary3DView::OnUpdateLocalizedBspDifference)
ON_UPDATE_COMMAND_UI(ID_BUTTON2, &CGeomary3DView::OnUpdateOctreeViewStatus)
ON_COMMAND(ID_BUTTON2, &CGeomary3DView::OnOcteeShow)
END_MESSAGE_MAP()

// CGeomary3DView construction/destruction

CGeomary3DView::CGeomary3DView()
	:mbDrag(false)
    ,mbOctreeShow(false)
{
	mpGraphic = new CGraphics();

}

CGeomary3DView::~CGeomary3DView()
{
	delete mpGraphic;
}

BOOL CGeomary3DView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CView::PreCreateWindow(cs);
}

void CGeomary3DView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	/*RECT rc;
	GetClientRect(&rc);
	LONG lw = rc.right - rc.left;
	LONG lh = rc.bottom - rc.top;
	mpGraphic->Initialize(lw, lh, GetSafeHwnd());*/

}

// CGeomary3DView drawing

void CGeomary3DView::OnDraw(CDC* /*pDC*/)
{
	CGeomary3DDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	mpGraphic->Frame();

	// TODO: add draw code for native data here
}

void CGeomary3DView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CGeomary3DView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CGeomary3DView diagnostics

#ifdef _DEBUG
void CGeomary3DView::AssertValid() const
{
	CView::AssertValid();
}

void CGeomary3DView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGeomary3DDoc* CGeomary3DView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeomary3DDoc)));
	return (CGeomary3DDoc*)m_pDocument;
}
#endif //_DEBUG


// CGeomary3DView message handlers


void CGeomary3DView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	CRect rc;
	GetClientRect(&rc);
	mpGraphic->ResizeWindow(rc.Width(), rc.Height());
	UpdateWindow();

	// TODO: Add your message handler code here
}


int CGeomary3DView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO:  Add your specialized creation code here
    RECT rc;
	GetClientRect(&rc);
	LONG lw = rc.right - rc.left;
	LONG lh = rc.bottom - rc.top;
	mpGraphic->Initialize(lw, lh, GetSafeHwnd());
	return 0;
}





void CGeomary3DView::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
	mpGraphic->Shutdown();
	CView::PostNcDestroy();
}


void  CGeomary3DView::Render()
{
	if (mbDrag)
	{
		POINT point;
		GetCursorPos(&point);
		::ScreenToClient(GetSafeHwnd(), &point);
		mpGraphic->UpdateOrbitView(point.x, point.y);
	}

	mpGraphic->Frame();
}

//void CGeomary3DView::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
//{
//	// This feature requires Windows Vista or greater.
//	// The symbol _WIN32_WINNT must be >= 0x0600.
//	// TODO: Add your message handler code here and/or call default
//	mpGraphic->ZoomView( zDelta);
//	CView::OnMouseHWheel(nFlags, zDelta, pt);
//}


BOOL CGeomary3DView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	mpGraphic->ZoomView( zDelta);
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void CGeomary3DView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect rc;
	GetClientRect(&rc);
	if( rc.PtInRect(point))
	{
		mbDrag = true;
	   SetCapture();
	   mpGraphic->StartOrbitView(point.x, point.y);
	}
	CView::OnLButtonDown(nFlags, point);
}


void CGeomary3DView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	mbDrag = false ;
	ReleaseCapture();
	CView::OnLButtonUp(nFlags, point);
}


void CGeomary3DView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	/*if (mbDrag)
	{
		mpGraphic->UpdateOrbitView(point.x, point.y);
	}*/
	CView::OnMouseMove(nFlags, point);
}


void CGeomary3DView::OnCaptureChanged(CWnd *pWnd)
{
	// TODO: Add your message handler code here
	if (pWnd->GetSafeHwnd() != this->GetSafeHwnd())
	{
		if (mbDrag)
		{
			mbDrag = false ;
			ReleaseCapture();
		}
	}
	CView::OnCaptureChanged(pWnd);
}


void CGeomary3DView::OnBoolUnion()
{
	// TODO: Add your command handler code here
	mpGraphic->Union();
}


void CGeomary3DView::OnUpdateBoolUnion(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(mpGraphic->GetModelMgr().Count() > 1 );
}


void CGeomary3DView::OnUpdateBoolIntersect(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(mpGraphic->GetModelMgr().Count() > 1 );
}


void CGeomary3DView::OnBoolIntersect()
{
	// TODO: Add your command handler code here
	mpGraphic->Intersect();
	
}


void CGeomary3DView::OnBoolDifference()
{
	// TODO: Add your command handler code here
	mpGraphic->Difference();
	

}


void CGeomary3DView::OnUpdateBoolDifference(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(mpGraphic->GetModelMgr().Count() > 1 );
}


void CGeomary3DView::OnVisualstyle()
{
	// TODO: Add your command handler code here
   
}


void CGeomary3DView::OnBoolExpression()
{
    // TODO: Add your command handler code here
    mpGraphic->EvaluateBoolExpression();
}


void CGeomary3DView::OnUpdateBoolExpression(CCmdUI *pCmdUI)
{
    // TODO: Add your command update UI handler code here

    pCmdUI->Enable(mpGraphic->GetModelMgr().Count() >= 3 );
}


void CGeomary3DView::OnBspBooleanUnion()
{
    // TODO: Add your command handler code here
    mpGraphic->BSPUnion();
}


void CGeomary3DView::OnBspBooleanIntersect()
{
    // TODO: Add your command handler code here
    mpGraphic->BSPIntersect();
}


void CGeomary3DView::OnBspBooleanDifference()
{
    // TODO: Add your command handler code here
    mpGraphic->BSPDifference();
}


void CGeomary3DView::OnUpdateBspBooleanUnion(CCmdUI *pCmdUI)
{
   pCmdUI->Enable(mpGraphic->GetModelMgr().Count() > 1 );

}


void CGeomary3DView::OnUpdateBspBooleanIntersect(CCmdUI *pCmdUI)
{
     pCmdUI->Enable(mpGraphic->GetModelMgr().Count() > 1 );
}


void CGeomary3DView::OnUpdateBspBooleanDifference(CCmdUI *pCmdUI)
{
     pCmdUI->Enable(mpGraphic->GetModelMgr().Count() > 1 );
}


void CGeomary3DView::OnLocalizedBspUnion()
{
    mpGraphic->LocalizedBSPUnion();
}


void CGeomary3DView::OnUpdateLocalizedBspUnion(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(mpGraphic->GetModelMgr().Count() > 1 );
}


void CGeomary3DView::OnLocalizedBspIntersect()
{
    mpGraphic->LocalizedBSPIntersect();
}


void CGeomary3DView::OnUpdateLocalizedBspIntersect(CCmdUI *pCmdUI)
{
     pCmdUI->Enable(mpGraphic->GetModelMgr().Count() > 1 );
}


void CGeomary3DView::OnLocalizedBspDifference()
{
    mpGraphic->LocalizedBSPDifference();
}


void CGeomary3DView::OnUpdateLocalizedBspDifference(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(mpGraphic->GetModelMgr().Count() > 1 );
}


void CGeomary3DView::OnUpdateOctreeViewStatus(CCmdUI *pCmdUI)
{
    // TODO: Add your command update UI handler code here
     pCmdUI->Enable(mpGraphic->GetModelMgr().Count() > 1 );
     pCmdUI->SetCheck(mbOctreeShow);
}


void CGeomary3DView::OnOcteeShow()
{
    // TODO: Add your command handler code here
    mbOctreeShow = !mbOctreeShow;
    mpGraphic->ShowOctree(mbOctreeShow);
    
}
