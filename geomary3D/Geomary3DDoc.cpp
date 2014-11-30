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

// Geomary3DDoc.cpp : implementation of the CGeomary3DDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Geomary3D.h"
#endif

#include "Geomary3DDoc.h"
#include "Geomary3DView.h"
#include <propkey.h>
#include <atlconv.h>
#include <afxdatarecovery.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CGeomary3DDoc

IMPLEMENT_DYNCREATE(CGeomary3DDoc, CDocument)

BEGIN_MESSAGE_MAP(CGeomary3DDoc, CDocument)
END_MESSAGE_MAP()


// CGeomary3DDoc construction/destruction

CGeomary3DDoc::CGeomary3DDoc()
{
	// TODO: add one-time construction code here

}

CGeomary3DDoc::~CGeomary3DDoc()
{
}

BOOL CGeomary3DDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CGeomary3DDoc serialization

void CGeomary3DDoc::Serialize(CArchive& ar)
{
	
	//size_t len = wcslen(lpszPathName) + 1;
	//size_t converted = 0;
	//char fbxFileName[MAX_PATH];
	//wcstombs_s(&converted, fbxFileName, len, lpszPathName, _TRUNCATE);
	//mFbx.LoadScene(fbxFileName);
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		size_t len = ar.GetFile()->GetFilePath().GetLength() + 1;
		size_t converted = 0;
		char fbxFileName[MAX_PATH];
		wcstombs_s(&converted, fbxFileName, len, ar.GetFile()->GetFilePath().GetBuffer(), _TRUNCATE);
		POSITION pos = GetFirstViewPosition();
		CGeomary3DView* pView = (CGeomary3DView*)GetNextView(pos);
		if (!pView->Graphic()->LoadSceneFromStream(fbxFileName))
			AfxMessageBox(L" Load Scene Form  FBX file Failed");

	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CGeomary3DDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CGeomary3DDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CGeomary3DDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CGeomary3DDoc diagnostics

#ifdef _DEBUG
void CGeomary3DDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGeomary3DDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CGeomary3DDoc commands


BOOL CGeomary3DDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;	
	return TRUE;
}


void CGeomary3DDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class
	CDocument::OnCloseDocument();
}

BOOL CGeomary3DDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace )
{
	CString newName = lpszPathName;

	if (newName.IsEmpty())
	{
		CDocTemplate* pTemplate = GetDocTemplate();
		ASSERT(pTemplate != NULL);

		newName = m_strPathName;
		if (bReplace && newName.IsEmpty())
		{
			newName = m_strTitle;
			// check for dubious filename
			int iBad = newName.FindOneOf(_T(":/\\"));
			if (iBad != -1)
				newName.ReleaseBuffer(iBad);
			if (newName.GetLength() > _MAX_FNAME)
				newName.ReleaseBuffer(_MAX_FNAME);

			if (AfxGetApp() && AfxGetApp()->GetDataRecoveryHandler())
			{
				// remove "[recovered]" from the title if it exists
				CString strNormalTitle = AfxGetApp()->GetDataRecoveryHandler()->GetNormalDocumentTitle(this);
				if (!strNormalTitle.IsEmpty())
					newName = strNormalTitle;
			}

			// append the default suffix if there is one
			CString strExt;
			if (pTemplate->GetDocString(strExt, CDocTemplate::filterExt) && !strExt.IsEmpty())
			{
				ASSERT(strExt[0] == '.');
				int iStart = 0;
			    int nSemi;                       //added
				if(nSemi = strExt.Find(';'));    //added
				strExt = strExt.Left(nSemi);     //added
				newName += strExt.Tokenize(_T(";"), iStart);
			}
		}

		if (!AfxGetApp()->DoPromptFileName(newName,
		  bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
		  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, pTemplate))
			return FALSE;       // don't even attempt to save
	}

	CWaitCursor wait;

	if (!OnSaveDocument(newName))
	{
		if (lpszPathName == NULL)
		{
			// be sure to delete the file
			TRY
			{
				CFile::Remove(newName);
			}
			CATCH_ALL(e)
			{
				TRACE(traceAppMsg, 0, "Warning: failed to delete file after failed SaveAs.\n");
				e->Delete();
			}
			END_CATCH_ALL
		}
		return FALSE;
	}

	// reset the title and change the document name
	if (bReplace)
	{
		SetPathName(newName);
		OnDocumentEvent(onAfterSaveDocument);
	}

	return TRUE;        // success
}