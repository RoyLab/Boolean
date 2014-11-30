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

// Geomary3D.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "Geomary3D.h"
#include "MainFrm.h"

#include "Geomary3DDoc.h"
#include "Geomary3DView.h"
#include "GDocTemplate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGeomary3DApp

BEGIN_MESSAGE_MAP(CGeomary3DApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CGeomary3DApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()


// CGeomary3DApp construction

CGeomary3DApp::CGeomary3DApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("Geomary3D.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CGeomary3DApp object

CGeomary3DApp theApp;


// CGeomary3DApp initialization

BOOL CGeomary3DApp::InitInstance()
{
	CWinAppEx::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CGDocTemplate* pDocTemplate;
	pDocTemplate = new CGDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CGeomary3DDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CGeomary3DView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

int CGeomary3DApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CGeomary3DApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CGeomary3DApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CGeomary3DApp customization load/save methods

void CGeomary3DApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CGeomary3DApp::LoadCustomState()
{
}

void CGeomary3DApp::SaveCustomState()
{
}

// CGeomary3DApp message handlers





BOOL CGeomary3DApp::OnIdle(LONG lCount)
{
	// TODO: Add your specialized code here and/or call the base class
	CWinApp::OnIdle(lCount);  

	CGeomary3DView *cview = (CGeomary3DView*)((CFrameWnd*)m_pMainWnd)->GetActiveView();     
	cview->Render();
	return CWinAppEx::OnIdle(lCount);
}
