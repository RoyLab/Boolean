
// geomary3DView.h : interface of the CGeomary3DView class
//

#pragma once


class CGeomary3DView : public CView
{
protected: // create from serialization only
	CGeomary3DView();
	DECLARE_DYNCREATE(CGeomary3DView)

// Attributes
public:
	CGeomary3DDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

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
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in geomary3DView.cpp
inline CGeomary3DDoc* CGeomary3DView::GetDocument() const
   { return reinterpret_cast<CGeomary3DDoc*>(m_pDocument); }
#endif

