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

// Geomary3DView.h : interface of the CGeomary3DView class
//


#pragma once

#include "Graphics.h"

class CGeomary3DView : public CView
{
protected: // create from serialization only
	CGeomary3DView();
	DECLARE_DYNCREATE(CGeomary3DView)

// Attributes
public:
	CGeomary3DDoc* GetDocument() const;
	CGraphics*     Graphic()   {return mpGraphic;}
// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate(); // called first time after construct
	        void Render();
protected:

// Implementation
public:
	virtual ~CGeomary3DView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
//	afx_msg void OnClose();

private:
	CGraphics*  mpGraphic;
	bool        mbDrag; 
    bool        mbOctreeShow; 
	virtual void PostNcDestroy();

public:
//	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg void OnBoolUnion();
	afx_msg void OnUpdateBoolUnion(CCmdUI *pCmdUI);
	afx_msg void OnUpdateBoolIntersect(CCmdUI *pCmdUI);
	afx_msg void OnBoolIntersect();
	afx_msg void OnBoolDifference();
	afx_msg void OnUpdateBoolDifference(CCmdUI *pCmdUI);
	afx_msg void OnVisualstyle();
    afx_msg void OnBoolExpression();
    afx_msg void OnUpdateBoolExpression(CCmdUI *pCmdUI);
    afx_msg void OnBspBooleanUnion();
    afx_msg void OnBspBooleanIntersect();
    afx_msg void OnBspBooleanDifference();
    afx_msg void OnUpdateBspBooleanUnion(CCmdUI *pCmdUI);
    afx_msg void OnUpdateBspBooleanIntersect(CCmdUI *pCmdUI);
    afx_msg void OnUpdateBspBooleanDifference(CCmdUI *pCmdUI);
    afx_msg void OnLocalizedBspUnion();
    afx_msg void OnUpdateLocalizedBspUnion(CCmdUI *pCmdUI);
    afx_msg void OnLocalizedBspIntersect();
    afx_msg void OnUpdateLocalizedBspIntersect(CCmdUI *pCmdUI);
    afx_msg void OnLocalizedBspDifference();
    afx_msg void OnUpdateLocalizedBspDifference(CCmdUI *pCmdUI);
    afx_msg void OnUpdateOctreeViewStatus(CCmdUI *pCmdUI);
    afx_msg void OnOcteeShow();
};

#ifndef _DEBUG  // debug version in Geomary3DView.cpp
inline CGeomary3DDoc* CGeomary3DView::GetDocument() const
   { return reinterpret_cast<CGeomary3DDoc*>(m_pDocument); }
#endif

