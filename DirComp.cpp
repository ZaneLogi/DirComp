// DirComp.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DirComp.h"
#include "DirCompDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDirCompApp

BEGIN_MESSAGE_MAP(CDirCompApp, CWinApp)
    //{{AFX_MSG_MAP(CDirCompApp)
    //}}AFX_MSG_MAP
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirCompApp construction

CDirCompApp::CDirCompApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDirCompApp object

CDirCompApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDirCompApp initialization

BOOL CDirCompApp::InitInstance()
{
    AfxEnableControlContainer();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    //  of your final executable, you should remove from the following
    //  the specific initialization routines you do not need.

    // Enable3dControls and Enable3dControlsStatic are no longer necessary.
    // see declaration of 'CWinApp::Enable3dControls'
//#ifdef _AFXDLL
//    Enable3dControls();            // Call this when using MFC in a shared DLL
//#else
//    Enable3dControlsStatic();    // Call this when linking to MFC statically
//#endif

    SetRegistryKey(_T("LuSoft"));

    CDirCompDlg dlg;
    m_pMainWnd = &dlg;
    int nResponse = dlg.DoModal();
    if (nResponse == IDOK)
    {
        // TODO: Place code here to handle when the dialog is
        //  dismissed with OK
    }
    else if (nResponse == IDCANCEL)
    {
        // TODO: Place code here to handle when the dialog is
        //  dismissed with Cancel
    }

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}

