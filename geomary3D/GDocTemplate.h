#pragma once
#include "afxwin.h"
class CGDocTemplate :public CSingleDocTemplate
{
	DECLARE_DYNAMIC(CGDocTemplate)
public:
	 CGDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);
	virtual ~CGDocTemplate(void);
	virtual BOOL GetDocString(CString& rString,
		enum DocStringIndex index) const; // get one of the info strings
	virtual Confidence MatchDocType(LPCTSTR lpszPathName,
					CDocument*& rpDocMatch);

};

