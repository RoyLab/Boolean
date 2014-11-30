
// geomary3DView.cpp : implementation of the CGeomary3DView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "geomary3D.h"
#endif

#include "geomary3DDoc.h"
#include "geomary3DView.h"
#include "typedefs.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGeomary3DView

IMPLEMENT_DYNCREATE(CGeomary3DView, CView)

BEGIN_MESSAGE_MAP(CGeomary3DView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CGeomary3DView construction/destruction

CGeomary3DView::CGeomary3DView()
{
	// TODO: add construction code here

}

CGeomary3DView::~CGeomary3DView()
{
}

BOOL CGeomary3DView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CGeomary3DView drawing

void CGeomary3DView::OnDraw(CDC* /*pDC*/)
{
	CGeomary3DDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	GS::vec3<float> pt;
	pt.x =1; 
	pt.y= 2; 


	// TODO: add draw code for native data here
}


// CGeomary3DView printing

BOOL CGeomary3DView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CGeomary3DView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CGeomary3DView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
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
