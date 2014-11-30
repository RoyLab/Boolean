
// geomary3D.h : main header file for the geomary3D application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CGeomary3DApp:
// See geomary3D.cpp for the implementation of this class
//

class CGeomary3DApp : public CWinApp
{
public:
	CGeomary3DApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CGeomary3DApp theApp;
