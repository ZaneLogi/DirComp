// DirCompDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DirComp.h"
#include "DirCompDlg.h"
#include <process.h>
#include "SortedStringArray.h"

#define COMP_IDENTICAL        0
#define COMP_LESS            -1
#define COMP_GREATER        1
#define COMP_BOTH_INVALID    -2
#define COMP_DST_INVALID    -3
#define COMP_SRC_INVALID    -4
#define COMP_CANCEL            -5

struct ListCtrlData {
    TCHAR src[MAX_PATH];
    TCHAR dst[MAX_PATH];
    TCHAR name[MAX_PATH];
    int   comp;
};

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void ShowErrorMessage()
{
    PVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL);
    TRACE( _T("Error: %s"), (LPTSTR)lpMsgBuf);
    LocalFree( lpMsgBuf );
}

void ShowErrorDialog()
{
    PVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL);
    AfxMessageBox( (LPCTSTR)lpMsgBuf );
    LocalFree( lpMsgBuf );
}

int CALLBACK
BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData )
{
    TCHAR szDir[MAX_PATH];

    switch(uMsg) {
    case BFFM_INITIALIZED:
        {
            SendMessage( hwnd, BFFM_SETSELECTION, TRUE, pData );
            break;
        }
    case BFFM_SELCHANGED:
        {
            // Set the status window to the currently selected path.
            if ( SHGetPathFromIDList((LPITEMIDLIST) lp , szDir ) ) {
                SendMessage( hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szDir );
            }
            break;
        }
    default:
        break;
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAboutDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    //{{AFX_MSG(CAboutDlg)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
        // No message handlers
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirCompDlg dialog

CDirCompDlg::CDirCompDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CDirCompDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDirCompDlg)
    m_sInclude = _T("*.*");
    m_bIncludeSubfolders = TRUE;
    //}}AFX_DATA_INIT
    // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_hThread = NULL;
    m_dwThreadID = 0;
    m_hStopEvent = CreateEvent( NULL, TRUE, FALSE, NULL);

    m_hFileCountThread = NULL;
    m_dwFileCountThreadID = 0;
    m_hFileCountStopEvent = CreateEvent( NULL, TRUE, FALSE, NULL);

    m_rcWindowOrig.left = -1;
}

CDirCompDlg::~CDirCompDlg()
{
    CloseHandle( m_hStopEvent);
    CloseHandle( m_hFileCountStopEvent );
}

void CDirCompDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDirCompDlg)
    DDX_Control(pDX, IDC_PROGRESS2, m_ctrlFileCountProgress);
    DDX_Control(pDX, IDC_BTN_EQUAL, m_btnEqual);
    DDX_Control(pDX, IDC_BTN_NOT_EQUAL, m_btnNotEqual);
    DDX_Control(pDX, IDC_BTN_FIND_NEXT_EQUAL, m_btnFindNextEqual);
    DDX_Control(pDX, IDC_BTN_FIND_NEXT_NOT_EQUAL, m_btnFindNextNotEqual);
    DDX_Control(pDX, IDC_FILE_COMPARING, m_sttFileComparing);
    DDX_Control(pDX, IDC_PROGRESS1, m_ctrlProgress);
    DDX_Control(pDX, IDC_BTN_STOP, m_btnStop);
    DDX_Control(pDX, IDC_BTN_START, m_btnStart);
    DDX_Control(pDX, IDC_CB_DST_FOLDER, m_cbDstFolder);
    DDX_Control(pDX, IDC_CB_SRC_FOLDER, m_cbSrcFolder);
    DDX_Control(pDX, IDC_LIST_FILES, m_lcFiles);
    DDX_Control(pDX, IDC_BTN_DST_BROWSER, m_btnDstBrowser);
    DDX_Control(pDX, IDC_BTN_SRC_BROWSER, m_btnSrcBrowser);
    DDX_Text(pDX, IDC_EDIT_INCLUDE, m_sInclude);
    DDX_Check(pDX, IDC_CHECK_INCLUDE_SUBFOLDER, m_bIncludeSubfolders);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDirCompDlg, CDialog)
    //{{AFX_MSG_MAP(CDirCompDlg)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_SRC_BROWSER, OnBtnSrcBrowser)
    ON_BN_CLICKED(IDC_BTN_DST_BROWSER, OnBtnDstBrowser)
    ON_CBN_EDITCHANGE(IDC_CB_SRC_FOLDER, OnEditchangeCbSrcFolder)
    ON_CBN_EDITCHANGE(IDC_CB_DST_FOLDER, OnEditchangeCbDstFolder)
    ON_BN_CLICKED(IDC_BTN_START, OnBtnStart)
    ON_BN_CLICKED(IDC_BTN_STOP, OnBtnStop)
    ON_BN_CLICKED(IDC_BTN_EQUAL, OnBtnEqual)
    ON_BN_CLICKED(IDC_BTN_NOT_EQUAL, OnBtnNotEqual)
    ON_CBN_SELCHANGE(IDC_CB_SRC_FOLDER, OnSelchangeCbSrcFolder)
    ON_CBN_SELCHANGE(IDC_CB_DST_FOLDER, OnSelchangeCbDstFolder)
    ON_MESSAGE( UM_END_OF_COMPARE, OnEndOfCompare)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_WM_SIZE()
    ON_WM_GETMINMAXINFO()
    ON_NOTIFY(NM_RCLICK, IDC_LIST_FILES, OnRclickListFiles)
    ON_WM_DESTROY()
    ON_COMMAND(ID_POPUP_COMP_SEL_FILES, OnPopupCompSelFiles)
    //}}AFX_MSG_MAP
    ON_COMMAND_RANGE(ID_POPUP_DEL_SEL_SRC_FILES, ID_POPUP_DEL_SEL_BOTH_FILES, OnPopupDelSelFiles)
    ON_COMMAND_RANGE(ID_POPUP_COPY_SEL_FILES_SRC2DST, ID_POPUP_COPY_SEL_FILES_DST2SRC, OnPopupCopySelFiles)
    ON_BN_CLICKED(IDC_BTN_FIND_NEXT_EQUAL, OnBnClickedBtnFindNextEqual)
    ON_BN_CLICKED(IDC_BTN_FIND_NEXT_NOT_EQUAL, OnBnClickedBtnFindNextNotEqual)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_FILES, OnNMDblclkListFiles)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirCompDlg message handlers

BOOL CDirCompDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);            // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // TODO: Add extra initialization here
    for(int i = 0; ; i++) {
        CString sTmp;
        sTmp.Format(_T("Entry%02d"), i);
        CString sLBText;
        sLBText = AfxGetApp()->GetProfileString(_T("Source"), sTmp, _T(""));
        if( sLBText.IsEmpty()) break;
        else m_cbSrcFolder.AddString( sLBText);
    }
    int nSrcCurSel = AfxGetApp()->GetProfileInt(_T("Source"), _T("CurSel"), 0);
    m_cbSrcFolder.SetCurSel( nSrcCurSel);

    for(int j = 0; ; j++) {
        CString sTmp;
        sTmp.Format(_T("Entry%02d"), j);
        CString sLBText;
        sLBText = AfxGetApp()->GetProfileString(_T("Target"), sTmp, _T(""));
        if( sLBText.IsEmpty()) break;
        else m_cbDstFolder.AddString( sLBText);
    }
    int nDstCurSel = AfxGetApp()->GetProfileInt(_T("Target"), _T("CurSel"), 0);
    m_cbDstFolder.SetCurSel( nDstCurSel);

    OnSelchangeCbSrcFolder();

    HICON iconBrowser = (HICON)LoadImage( AfxGetInstanceHandle(),
        MAKEINTRESOURCE(IDI_BROWSER),
        IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    m_btnSrcBrowser.SetIcon( iconBrowser);
    m_btnDstBrowser.SetIcon( iconBrowser);

    HICON iconNotEqual = (HICON)LoadImage( AfxGetInstanceHandle(),
        MAKEINTRESOURCE(IDI_ICON_NOT_EQUAL),
        IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    m_btnNotEqual.SetIcon( iconNotEqual);

    HICON iconEqual = (HICON)LoadImage( AfxGetInstanceHandle(),
        MAKEINTRESOURCE(IDI_ICON_EQUAL),
        IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    m_btnEqual.SetIcon( iconEqual);

    HICON iconFindNextEqual = (HICON)LoadImage( AfxGetInstanceHandle(),
        MAKEINTRESOURCE(IDI_ICON_FIND_NEXT_EQUAL),
        IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    m_btnFindNextEqual.SetIcon( iconFindNextEqual);

    HICON iconFindNextNotEqual = (HICON)LoadImage( AfxGetInstanceHandle(),
        MAKEINTRESOURCE(IDI_ICON_FIND_NEXT_NOT_EQUAL),
        IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    m_btnFindNextNotEqual.SetIcon( iconFindNextNotEqual);

    DWORD dwStyle = m_lcFiles.SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE);
    dwStyle |= LVS_EX_GRIDLINES
        | LVS_EX_FULLROWSELECT
        | LVS_EX_INFOTIP
        ;//| LVS_EX_CHECKBOXES;
    m_lcFiles.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM)dwStyle);

    CRect rc;
    m_lcFiles.GetClientRect( rc);
    m_lcFiles.InsertColumn( COL_SRC_NAME    , _T("Name"), LVCFMT_LEFT, (rc.Width()-340)/2 );
    m_lcFiles.InsertColumn( COL_SRC_SIZE    , _T("Size"), LVCFMT_LEFT, 60);
    m_lcFiles.InsertColumn( COL_SRC_MODIFIED, _T("Modified"), LVCFMT_LEFT, 100);
    m_lcFiles.InsertColumn( COL_EQUAL_SIGN  , _T("="), LVCFMT_CENTER, 20);
    m_lcFiles.InsertColumn( COL_DST_MODIFIED, _T("Modified"), LVCFMT_RIGHT, 100);
    m_lcFiles.InsertColumn( COL_DST_SIZE    , _T("Size"), LVCFMT_RIGHT, 60);
    m_lcFiles.InsertColumn( COL_DST_NAME    , _T("Name"), LVCFMT_RIGHT, (rc.Width()-340)/2 );

    m_sttFileComparing.SetWindowText(_T("Ready"));
    m_ctrlProgress.SetRange(0,1000);
    m_ctrlProgress.SetPos(1000);

    m_ctrlFileCountProgress.SetRange(0,1000);
    m_ctrlFileCountProgress.SetPos(1000);

    GetWindowRect( m_rcWindowOrig );
    GetClientRect( m_rcClientOrig );

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDirCompDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}

void CDirCompDlg::OnAppAbout()
{
    CAboutDlg dlgAbout;
    dlgAbout.DoModal();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDirCompDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDirCompDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}

void CDirCompDlg::OnDestroy()
{
    CDialog::OnDestroy();

    // TODO: Add your message handler code here
    ClearListCtrl();
}

void CDirCompDlg::OnBtnSrcBrowser()
{
    static TCHAR sDir[MAX_PATH];
    m_cbSrcFolder.GetWindowText( sDir, MAX_PATH );

    TCHAR pszDisplayName[MAX_PATH] = {0};
    BROWSEINFO bi ={0};

    bi.hwndOwner = GetSafeHwnd();
    bi.pszDisplayName = pszDisplayName;
    bi.lpszTitle = _T("Please select source folder:");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM)sDir;

    LPITEMIDLIST lpIL = SHBrowseForFolder(&bi);
    if( lpIL) {
        TCHAR sFolder[MAX_PATH+1];
        SHGetPathFromIDList( lpIL, sFolder);
        m_cbSrcFolder.SetWindowText( sFolder);
        OnEditchangeCbSrcFolder();
    }
}

void CDirCompDlg::OnBtnDstBrowser()
{
    static TCHAR sDir[MAX_PATH];
    m_cbDstFolder.GetWindowText( sDir, MAX_PATH );

    TCHAR pszDisplayName[MAX_PATH] = {0};
    BROWSEINFO bi ={0};

    bi.hwndOwner = GetSafeHwnd();
    bi.pszDisplayName = pszDisplayName;
    bi.lpszTitle = _T("Please select target folder:");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM)sDir;

    LPITEMIDLIST lpIL = SHBrowseForFolder(&bi);
    if( lpIL) {
        TCHAR sFolder[MAX_PATH+1];
        SHGetPathFromIDList( lpIL, sFolder);
        m_cbDstFolder.SetWindowText( sFolder);
        OnEditchangeCbDstFolder();
    }
}

void CDirCompDlg::OnEditchangeCbSrcFolder()
{
    int nSrcLen = m_cbSrcFolder.GetWindowTextLength();
    int nDstLen = m_cbDstFolder.GetWindowTextLength();
    if( nSrcLen && nDstLen) m_btnStart.EnableWindow();
    else m_btnStart.EnableWindow(FALSE);
}

void CDirCompDlg::OnEditchangeCbDstFolder()
{
    int nSrcLen = m_cbSrcFolder.GetWindowTextLength();
    int nDstLen = m_cbDstFolder.GetWindowTextLength();
    if( nSrcLen && nDstLen) m_btnStart.EnableWindow();
    else m_btnStart.EnableWindow(FALSE);
}

void CDirCompDlg::OnSelchangeCbSrcFolder()
{
    int nSrcLen = m_cbSrcFolder.GetWindowTextLength();
    int nDstLen = m_cbDstFolder.GetWindowTextLength();
    if( nSrcLen && nDstLen) m_btnStart.EnableWindow();
    else m_btnStart.EnableWindow(FALSE);
}

void CDirCompDlg::OnSelchangeCbDstFolder()
{
    int nSrcLen = m_cbSrcFolder.GetWindowTextLength();
    int nDstLen = m_cbDstFolder.GetWindowTextLength();
    if( nSrcLen && nDstLen) m_btnStart.EnableWindow();
    else m_btnStart.EnableWindow(FALSE);
}

void CDirCompDlg::ClearListCtrl()
{
    int nCount = m_lcFiles.GetItemCount();
    while ( nCount-- > 0 ) {
        ListCtrlData* p = (ListCtrlData*)m_lcFiles.GetItemData(nCount);
        if (p) delete p;
    }
    m_lcFiles.DeleteAllItems();
}

void CDirCompDlg::OnBtnStart()
{
    UpdateData();

    if( m_cbSrcFolder.GetWindowTextLength() == 0 ||
        m_cbDstFolder.GetWindowTextLength() == 0 ||
        m_sInclude.IsEmpty())
    {
        AfxMessageBox(_T("Invalid parameters!!!"));
        return;
    }

    ClearListCtrl();

    m_cbSrcFolder.EnableWindow(FALSE);
    m_btnSrcBrowser.EnableWindow(FALSE);
    m_cbDstFolder.EnableWindow(FALSE);
    m_btnDstBrowser.EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT_INCLUDE)->EnableWindow(FALSE);
    GetDlgItem(IDC_CHECK_INCLUDE_SUBFOLDER)->EnableWindow(FALSE);
    m_btnStart.EnableWindow(FALSE);
    m_lcFiles.EnableWindow(FALSE);
    m_btnStop.EnableWindow();

    m_cbSrcFolder.GetWindowText( m_sSrcDir);
    m_cbDstFolder.GetWindowText( m_sDstDir);
    if( m_cbSrcFolder.FindString( -1, m_sSrcDir) == CB_ERR) {
        int nIndex = m_cbSrcFolder.AddString( m_sSrcDir);
        m_cbSrcFolder.SetCurSel( nIndex);
    }
    if( m_cbDstFolder.FindString( -1, m_sDstDir) == CB_ERR) {
        int nIndex = m_cbDstFolder.AddString( m_sDstDir);
        m_cbDstFolder.SetCurSel( nIndex);
    }
    if ( m_sSrcDir.Right(1) == _T('\\')) { m_sSrcDir = m_sSrcDir.Left( m_sSrcDir.GetLength() - 1); }
    if ( m_sDstDir.Right(1) == _T('\\')) { m_sDstDir = m_sDstDir.Left( m_sDstDir.GetLength() - 1); }

    m_dwTotalNumberOfFiles = 0;
    m_bFileCountCompleted = FALSE;

    m_hFileCountThread = (HANDLE)_beginthreadex( NULL, 0,
        (PBEGINTHREADEX_THREADFUNC)FileCountThreadFunc, (LPVOID)this, 0,
        (PBEGINTHREADEX_THREADID)&m_dwFileCountThreadID);

    if( m_hFileCountThread <= 0)
        AfxMessageBox(_T("Error to create m_hFileCountThread"));
    else {
        TRACE(_T("The thread %x has created\n"), m_dwFileCountThreadID);
    }

    m_hThread = (HANDLE)_beginthreadex( NULL, 0,
        (PBEGINTHREADEX_THREADFUNC)ThreadFunc, (LPVOID)this, 0,
        (PBEGINTHREADEX_THREADID)&m_dwThreadID);

    if( m_hThread <= 0)
        AfxMessageBox(_T("Error to create m_hThread"));
    else {
        TRACE(_T("The thread %x has created\n"), m_dwThreadID);
    }
}

void CDirCompDlg::OnBtnStop()
{
    m_btnStop.EnableWindow(FALSE);
    WaitForExit();
    m_lcFiles.EnableWindow();
    m_cbSrcFolder.EnableWindow();
    m_btnSrcBrowser.EnableWindow();
    m_cbDstFolder.EnableWindow();
    m_btnDstBrowser.EnableWindow();
    GetDlgItem(IDC_EDIT_INCLUDE)->EnableWindow();
    GetDlgItem(IDC_CHECK_INCLUDE_SUBFOLDER)->EnableWindow();
    m_btnStart.EnableWindow();
    CloseHandle(m_hThread);
}

void CDirCompDlg::WaitForExit()
{
    if ( m_hFileCountThread != NULL ) {
        SetEvent( m_hFileCountStopEvent);
        while( WaitForSingleObject( m_hFileCountThread, 0) != WAIT_OBJECT_0 ) {
            MSG msg;
            if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE) ) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        ResetEvent( m_hFileCountStopEvent);
        CloseHandle(m_hFileCountThread);
        m_hFileCountThread = NULL;
    }

    if( m_hThread != NULL) {
        SetEvent( m_hStopEvent);
        while( WaitForSingleObject( m_hThread, 0) != WAIT_OBJECT_0 ) {
            MSG msg;
            if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE) ) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        ResetEvent( m_hStopEvent);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    m_ctrlProgress.SetPos(1000);
    m_ctrlFileCountProgress.SetPos(1000);
}

LONG CDirCompDlg::OnEndOfCompare( WPARAM wParam, LPARAM lParam )
{
    m_btnStop.EnableWindow(FALSE);
    m_lcFiles.EnableWindow();
    m_cbSrcFolder.EnableWindow();
    m_btnSrcBrowser.EnableWindow();
    m_cbDstFolder.EnableWindow();
    m_btnDstBrowser.EnableWindow();
    GetDlgItem(IDC_EDIT_INCLUDE)->EnableWindow();
    GetDlgItem(IDC_CHECK_INCLUDE_SUBFOLDER)->EnableWindow();
    m_btnStart.EnableWindow();

    CString sMessage;
    sMessage.Format(_T("Total Files: %d, Identical Files: %d, Different Files: %d."),
        m_dwIdenticalFiles + m_dwDifferentFiles,
        m_dwIdenticalFiles, m_dwDifferentFiles );
    AfxMessageBox( sMessage, MB_OK | MB_ICONINFORMATION );
    return 0;
}

DWORD WINAPI CDirCompDlg::FileCountThreadFunc(LPVOID param)
{
    CDirCompDlg* pto = (CDirCompDlg*)param;
    return pto->FileCountThreadMemberFunc();
}

DWORD CDirCompDlg::FileCountThreadMemberFunc()
{
    m_ctrlFileCountProgress.SetPos(0);
    DWORD dw = SearchFolder( "\\" );
    m_bFileCountCompleted = TRUE;
    return dw;
}

DWORD CDirCompDlg::SearchFolder( CString sParentDir)
{
    if( WaitForSingleObject( m_hFileCountStopEvent, 0) != WAIT_TIMEOUT) {
        return -1;
    }

    CSortedStringArray saFiles;

    CFileFind ff;
    BOOL b;

    m_cs.Lock();
    // find source files
    b= SetCurrentDirectory( m_sSrcDir + sParentDir);
    if(b) {
        b = ff.FindFile( m_sInclude);
        while(b) {
            b = ff.FindNextFile();
            if( !ff.IsDirectory()) {
                saFiles.AddString( ff.GetFileName());
            }
        }
        ff.Close();
    }
    m_cs.Unlock();

    m_cs.Lock();
    // find target files
    b = SetCurrentDirectory( m_sDstDir + sParentDir);
    if(b) {
        b = ff.FindFile( m_sInclude);
        while(b) {
            b = ff.FindNextFile();
            if( !ff.IsDirectory()) {
                saFiles.AddString( ff.GetFileName());
            }
        }
        ff.Close();
    }
    m_cs.Unlock();

    if( WaitForSingleObject( m_hStopEvent, 0) != WAIT_TIMEOUT) {
        return -1;
    }

    m_dwTotalNumberOfFiles += saFiles.GetSize();

    if( !m_bIncludeSubfolders) return 0;
    // Enter to subfolders
    CSortedStringArray saFolders;

    m_cs.Lock();
    b = SetCurrentDirectory( m_sSrcDir + sParentDir);
    if(b) {
        b = ff.FindFile(_T("*.*"));
        while(b) {
            b = ff.FindNextFile();
            if( !ff.IsDots() && !ff.IsSystem() &&ff.IsDirectory()) {
                saFolders.AddString( ff.GetFileName());
            }
        }
        ff.Close();
    }
    m_cs.Unlock();

    if( WaitForSingleObject( m_hStopEvent, 0) != WAIT_TIMEOUT) {
        return -1;
    }

    m_cs.Lock();
    b= SetCurrentDirectory( m_sDstDir + sParentDir);
    if(b) {
        b = ff.FindFile(_T("*.*"));
        while(b) {
            b = ff.FindNextFile();
            if( !ff.IsDots() && !ff.IsSystem() &&ff.IsDirectory()) {
                saFolders.AddString( ff.GetFileName());
            }
        }
        ff.Close();
    }
    m_cs.Unlock();

    for ( int i = 0; i < saFolders.GetSize(); i++ ) {

        if( WaitForSingleObject( m_hStopEvent, 0) != WAIT_TIMEOUT) {
            return -1;
        }

        SearchFolder( sParentDir + saFolders[i] + "\\");
    }

    return 0;
}

DWORD WINAPI CDirCompDlg::ThreadFunc(LPVOID param)
{
    CDirCompDlg* pto = (CDirCompDlg*)param;
    return pto->ThreadMemberFunc();
}

DWORD CDirCompDlg::ThreadMemberFunc()
{
    TRACE(_T("Comparing %s to %s...\n"), m_sSrcDir, m_sDstDir);
    m_dwIdenticalFiles = 0;
    m_dwDifferentFiles = 0;
    DWORD dw = CompareFolder(_T("\\"));
    CString s;
    s.Format(_T(" %d files compared."), m_dwIdenticalFiles + m_dwDifferentFiles );
    m_sttFileComparing.SetWindowText(s);
    PostMessage( UM_END_OF_COMPARE );
    return dw;
}

DWORD CDirCompDlg::CompareFolder( CString sParentDir)
{
    if( WaitForSingleObject( m_hStopEvent, 0) != WAIT_TIMEOUT) {
        return -1;
    }

    // Place the folder name
    if( sParentDir != _T("\\") ) {
        CString sTmp = _T("Folder: [ ");
        int nCount = m_lcFiles.GetItemCount();
        int nLen = sParentDir.GetLength();
        m_lcFiles.InsertItem( nCount, sTmp + sParentDir.Mid(1, nLen -2) + " ]");
        m_lcFiles.SetItemData( nCount, 0 );
    }

    CSortedStringArray saFiles;

    CFileFind ff;
    BOOL b;

    m_cs.Lock();
    // find source files
    b= SetCurrentDirectory( m_sSrcDir + sParentDir);
    if(b) {
        b = ff.FindFile( m_sInclude);
        while(b) {
            b = ff.FindNextFile();
            if( !ff.IsDirectory()) {
                saFiles.AddString( ff.GetFileName());
            }
        }
        ff.Close();
    }
    m_cs.Unlock();

    m_cs.Lock();
    // find target files
    b = SetCurrentDirectory( m_sDstDir + sParentDir);
    if(b) {
        b = ff.FindFile( m_sInclude);
        while(b) {
            b = ff.FindNextFile();
            if( !ff.IsDirectory()) {
                saFiles.AddString( ff.GetFileName());
            }
        }
        ff.Close();
    }
    m_cs.Unlock();

    for(int i = 0; i < saFiles.GetSize(); i++) {
        // update file completed progress
        if ( m_bFileCountCompleted ) {
            int nIndex = ( m_dwIdenticalFiles + m_dwDifferentFiles + 1 ) * 1000 / m_dwTotalNumberOfFiles;
            if ( m_ctrlFileCountProgress.GetPos() != nIndex ) {
                m_ctrlFileCountProgress.SetPos(nIndex);
            }
        }

        if( WaitForSingleObject( m_hStopEvent, 0) != WAIT_TIMEOUT) {
            return -1;
        }

        int nCount = m_lcFiles.GetItemCount();
        TRACE("%s is comparing...", saFiles[i]);
        m_sttFileComparing.SetWindowText( saFiles[i]);
        int nRet = CompareFile(
            m_sSrcDir + sParentDir + saFiles[i],
            m_sDstDir + sParentDir + saFiles[i]);

        m_lcFiles.InsertItem( nCount, _T("") );
        ListCtrlData* pListCtrlData = new ListCtrlData;
        m_lcFiles.SetItemData( nCount, (DWORD)pListCtrlData );

        lstrcpy( pListCtrlData->src, m_sSrcDir + sParentDir );
        lstrcpy( pListCtrlData->dst, m_sDstDir + sParentDir );
        lstrcpy( pListCtrlData->name, saFiles[i] );
        pListCtrlData->comp   = nRet;

        CString strTmp;
        CFileStatus status;
        CString strSrcPath = m_sSrcDir + sParentDir + saFiles[i];
        CString strDstPath = m_sDstDir + sParentDir + saFiles[i];

        switch ( nRet ) {
        case COMP_IDENTICAL:
        case COMP_GREATER:
        case COMP_LESS:
            {
                m_lcFiles.SetItemText( nCount, COL_SRC_NAME, saFiles[i] );
                CFile::GetStatus( strSrcPath, status );
                strTmp.Format(_T("%dKB"), (status.m_size + 1023)>>10);
                m_lcFiles.SetItemText( nCount, COL_SRC_SIZE, strTmp );
                m_lcFiles.SetItemText( nCount, COL_SRC_MODIFIED, status.m_mtime.Format("%c") );

                m_lcFiles.SetItemText( nCount, COL_DST_NAME, saFiles[i]);
                CFile::GetStatus( strDstPath, status );
                strTmp.Format(_T("%dKB"), (status.m_size + 1023)>>10);
                m_lcFiles.SetItemText( nCount, COL_DST_SIZE, strTmp );
                m_lcFiles.SetItemText( nCount, COL_DST_MODIFIED, status.m_mtime.Format("%c") );
            }
            break;

        case COMP_DST_INVALID:
            {
                m_lcFiles.SetItemText( nCount, COL_SRC_NAME, saFiles[i]);
                CFile::GetStatus( strSrcPath, status );
                strTmp.Format(_T("%dKB"), (status.m_size + 1023)>>10);
                m_lcFiles.SetItemText( nCount, COL_SRC_SIZE, strTmp );
                m_lcFiles.SetItemText( nCount, COL_SRC_MODIFIED, status.m_mtime.Format("%c") );
            }
            break;

        case COMP_SRC_INVALID:
            {
                m_lcFiles.SetItemText( nCount, COL_DST_NAME, saFiles[i]);
                CFile::GetStatus( strDstPath, status );
                strTmp.Format(_T("%dKB"), (status.m_size + 1023)>>10);
                m_lcFiles.SetItemText( nCount, COL_DST_SIZE, strTmp );
                m_lcFiles.SetItemText( nCount, COL_DST_MODIFIED, status.m_mtime.Format("%c") );
            }
            break;

        case COMP_BOTH_INVALID:
        case COMP_CANCEL:
            {
                m_lcFiles.SetItemText( nCount, COL_SRC_NAME, saFiles[i]);
                m_lcFiles.SetItemText( nCount, COL_DST_NAME, saFiles[i]);
            }
            break;
        }

        switch( nRet ) {
        case COMP_IDENTICAL:
            m_lcFiles.SetItemText( nCount, COL_EQUAL_SIGN, _T("="));
            m_dwIdenticalFiles++;
            TRACE(_T("equal\n"));
            break;
        case COMP_GREATER:
        case COMP_DST_INVALID:
            m_lcFiles.SetItemText( nCount, COL_EQUAL_SIGN, _T(">"));
            m_dwDifferentFiles++;
            TRACE(_T("not equal\n"));
            break;
        case COMP_LESS:
        case COMP_SRC_INVALID:
            m_lcFiles.SetItemText( nCount, COL_EQUAL_SIGN, _T("<"));
            m_dwDifferentFiles++;
            TRACE(_T("not equal\n"));
            break;
        case COMP_BOTH_INVALID:
            m_lcFiles.SetItemText( nCount, COL_EQUAL_SIGN, _T("X"));
            m_dwDifferentFiles++;
            TRACE(_T("not equal\n"));
            break;
        case COMP_CANCEL:
            m_lcFiles.SetItemText( nCount, COL_EQUAL_SIGN, _T("!"));
            m_dwDifferentFiles++;
            TRACE(_T("not compared\n"));
            break;
        }

        if ( COMP_CANCEL == nRet ) return 0;
    }

    if( !m_bIncludeSubfolders ) return 0;

    // Enter to subfolders
    CSortedStringArray saFolders;

    m_cs.Lock();
    b = SetCurrentDirectory( m_sSrcDir + sParentDir);
    if(b) {
        b = ff.FindFile(_T("*.*"));
        while(b) {
            b = ff.FindNextFile();
            if( !ff.IsDots() && !ff.IsSystem() &&ff.IsDirectory()) {
                saFolders.AddString( ff.GetFileName());
            }
        }
        ff.Close();
    }
    m_cs.Unlock();

    m_cs.Lock();
    b= SetCurrentDirectory( m_sDstDir + sParentDir);
    if(b) {
        b = ff.FindFile(_T("*.*"));
        while(b) {
            b = ff.FindNextFile();
            if( !ff.IsDots() && !ff.IsSystem() &&ff.IsDirectory()) {
                saFolders.AddString( ff.GetFileName());
            }
        }
        ff.Close();
    }
    m_cs.Unlock();

    for(int i = 0; i < saFolders.GetSize(); i++) {

        if( WaitForSingleObject( m_hStopEvent, 0) != WAIT_TIMEOUT) {
            return -1;
        }

        int nCount = m_lcFiles.GetItemCount();
        CompareFolder( sParentDir + saFolders[i] + _T("\\"));
    }

    return 0;
}

int CDirCompDlg::CompareFile(CString sSrc, CString sDst)
{
    int nRet = COMP_IDENTICAL;

    SYSTEM_INFO si;
    GetSystemInfo( &si);

    HANDLE hSrcFile = CreateFile( sSrc, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    //ShowErrorMessage();
    HANDLE hDstFile = CreateFile( sDst, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    //ShowErrorMessage();

    if( hSrcFile == INVALID_HANDLE_VALUE && hDstFile == INVALID_HANDLE_VALUE) {
        CloseHandle( hDstFile);
        return COMP_BOTH_INVALID;
    }

    if( hSrcFile != INVALID_HANDLE_VALUE && hDstFile == INVALID_HANDLE_VALUE) {
        CloseHandle( hSrcFile);
        return COMP_DST_INVALID;
    }

    if( hSrcFile == INVALID_HANDLE_VALUE && hDstFile != INVALID_HANDLE_VALUE) {
        CloseHandle( hDstFile);
        return COMP_SRC_INVALID;
    }

    __int64 qwSrcFileSize, qwDstFileSize;
    DWORD dwFileSizeHigh;

    qwSrcFileSize = GetFileSize( hSrcFile, &dwFileSizeHigh);
    qwSrcFileSize += (((__int64)dwFileSizeHigh) << 32);

    qwDstFileSize = GetFileSize( hDstFile, &dwFileSizeHigh);
    qwDstFileSize += (((__int64)dwFileSizeHigh) << 32);

    if( qwSrcFileSize != qwDstFileSize) {
        CloseHandle( hSrcFile);
        CloseHandle( hDstFile);
        if( qwSrcFileSize > qwDstFileSize) return COMP_GREATER;
        else if( qwSrcFileSize < qwDstFileSize) return COMP_LESS;
        return COMP_BOTH_INVALID; // impossible
    }

    if( qwSrcFileSize == 0) {
        CloseHandle( hSrcFile);
        CloseHandle( hDstFile);
        return COMP_IDENTICAL;
    }

    HANDLE hSrcFileMapping = CreateFileMapping( hSrcFile, NULL, PAGE_READONLY, 0, 0, NULL);
    CloseHandle( hSrcFile);
    if( hSrcFileMapping == NULL) {
        CloseHandle( hDstFile);
        return COMP_SRC_INVALID;
    }

    HANDLE hDstFileMapping = CreateFileMapping( hDstFile, NULL, PAGE_READONLY, 0, 0, NULL);
    CloseHandle( hDstFile);
    if( hDstFileMapping == NULL) {
        CloseHandle( hSrcFileMapping);
        return COMP_DST_INVALID;
    }

    __int64 qwFileOffset = 0;

    while( qwSrcFileSize > 0) {

        if( WaitForSingleObject( m_hStopEvent, 0) != WAIT_TIMEOUT) {
            return COMP_CANCEL;
        }

        DWORD dwBytesInBlock;

        if( qwSrcFileSize < si.dwAllocationGranularity)
            dwBytesInBlock = (DWORD)qwSrcFileSize;
        else
            dwBytesInBlock = si.dwAllocationGranularity;

        PBYTE pbSrcFile = (PBYTE)MapViewOfFile( hSrcFileMapping, FILE_MAP_READ,
            (DWORD)(qwFileOffset >> 32),
            (DWORD)(qwFileOffset & 0xffffffff),
            dwBytesInBlock);

        PBYTE pbDstFile = (PBYTE)MapViewOfFile( hDstFileMapping, FILE_MAP_READ,
            (DWORD)(qwFileOffset >> 32),
            (DWORD)(qwFileOffset & 0xffffffff),
            dwBytesInBlock);

        nRet = memcmp( pbSrcFile, pbDstFile, dwBytesInBlock);

        UnmapViewOfFile( pbSrcFile);
        UnmapViewOfFile( pbDstFile);

        qwFileOffset += dwBytesInBlock;
        qwSrcFileSize -= dwBytesInBlock;

        float f = (float)qwFileOffset / (float)qwDstFileSize * 1000;
        m_ctrlProgress.SetPos((int)f);

        if( nRet > 0 ) {
            nRet = COMP_GREATER;
            break;
        }

        if( nRet < 0 ) {
            nRet = COMP_LESS;
            break;
        }
    }

    CloseHandle( hSrcFileMapping);
    CloseHandle( hDstFileMapping);

    return nRet;
}

void CDirCompDlg::OnCancel()
{
    WaitForExit();

    int nSrcCount = m_cbSrcFolder.GetCount();
    for( int i = 0; i < nSrcCount; i++) {
        CString sTmp;
        sTmp.Format(_T("Entry%02d"), i);
        CString sLBText;
        m_cbSrcFolder.GetLBText( i, sLBText);
        AfxGetApp()->WriteProfileString(_T("Source"), sTmp, sLBText);
    }
    int nSrcCurSel = m_cbSrcFolder.GetCurSel();
    AfxGetApp()->WriteProfileInt(_T("Source"), _T("CurSel"), nSrcCurSel);

    int nDstCount = m_cbDstFolder.GetCount();
    for( int j = 0; j < nDstCount; j++) {
        CString sTmp;
        sTmp.Format(_T("Entry%02d"), j);
        CString sLBText;
        m_cbDstFolder.GetLBText( j, sLBText);
        AfxGetApp()->WriteProfileString(_T("Target"), sTmp, sLBText);
    }
    int nDstCurSel = m_cbDstFolder.GetCurSel();
    AfxGetApp()->WriteProfileInt(_T("Target"), _T("CurSel"), nDstCurSel);

    CDialog::OnCancel();
}

void CDirCompDlg::OnBtnEqual()
{
    int nCount = m_lcFiles.GetItemCount();
    for( int i = 0; i < nCount; i++) {
        CString s = m_lcFiles.GetItemText( i, COL_EQUAL_SIGN );
        if( s == "=") {
            m_lcFiles.SetItemState( i, LVIS_SELECTED, LVIS_SELECTED);
        }
        else {
            m_lcFiles.SetItemState( i, 0, LVIS_SELECTED);
        }
    }
}

void CDirCompDlg::OnBtnNotEqual()
{
    int nCount = m_lcFiles.GetItemCount();
    for( int i = 0; i < nCount; i++) {
        CString s0 = m_lcFiles.GetItemText( i, COL_SRC_NAME );
        CString s1 = m_lcFiles.GetItemText( i, COL_EQUAL_SIGN );
        if( s1 != "=" && s0.Find(_T("Folder:")) < 0) {
            m_lcFiles.SetItemState( i, LVIS_SELECTED, LVIS_SELECTED);
        }
        else {
            m_lcFiles.SetItemState( i, 0, LVIS_SELECTED);
        }
    }
}

void CDirCompDlg::OnBnClickedBtnFindNextEqual()
{
    POSITION pos = m_lcFiles.GetFirstSelectedItemPosition();
    int nItem = 0;
    if ( pos ) {
        nItem = m_lcFiles.GetNextSelectedItem(pos);
        nItem++;
    }

    int nCount = m_lcFiles.GetItemCount();
    // de-select all items
    for ( int i = 0; i < nItem; i++ ) {
        m_lcFiles.SetItemState( i, 0, LVIS_SELECTED);
    }

    int nFoundIndex = -1;
    for ( int i = nItem; i < nCount; i++ ) {
        CString s0 = m_lcFiles.GetItemText( i, COL_SRC_NAME );
        CString s1 = m_lcFiles.GetItemText( i, COL_EQUAL_SIGN );
        if( s1 == "=" ) {
            if ( nFoundIndex == -1 ) {
                m_lcFiles.SetItemState( i, LVIS_SELECTED, LVIS_SELECTED);
                nFoundIndex = i;
            }
            else {
                m_lcFiles.SetItemState( i, 0, LVIS_SELECTED);
            }
        }
        else {
            m_lcFiles.SetItemState( i, 0, LVIS_SELECTED);
        }
    }

    if ( nFoundIndex == -1 ) {
        AfxMessageBox(_T("No More!") );
    }
    else {
        m_lcFiles.EnsureVisible(nFoundIndex,FALSE);
    }
}

void CDirCompDlg::OnBnClickedBtnFindNextNotEqual()
{
    POSITION pos = m_lcFiles.GetFirstSelectedItemPosition();
    int nItem = 0;
    if ( pos ) {
        nItem = m_lcFiles.GetNextSelectedItem(pos);
        nItem++;
    }

    int nCount = m_lcFiles.GetItemCount();
    // de-select all items
    for ( int i = 0; i < nItem; i++ ) {
        m_lcFiles.SetItemState( i, 0, LVIS_SELECTED);
    }

    int nFoundIndex = -1;
    for ( int i = nItem; i < nCount; i++ ) {
        CString s0 = m_lcFiles.GetItemText( i, COL_SRC_NAME );
        CString s1 = m_lcFiles.GetItemText( i, COL_EQUAL_SIGN );
        if( s1 != "=" && s0.Find(_T("Folder:")) < 0 ) {
            if ( nFoundIndex == -1 ) {
                m_lcFiles.SetItemState( i, LVIS_SELECTED, LVIS_SELECTED);
                nFoundIndex = i;
            }
            else {
                m_lcFiles.SetItemState( i, 0, LVIS_SELECTED);
            }
        }
        else {
            m_lcFiles.SetItemState( i, 0, LVIS_SELECTED);
        }
    }

    if ( nFoundIndex == -1 ) {
        AfxMessageBox(_T("No More!") );
    }
    else {
        m_lcFiles.EnsureVisible(nFoundIndex,FALSE);
    }
}

void CDirCompDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    // TODO: Add your message handler code here
    int nDiffX = cx - m_rcClientOrig.right;
    int nDiffY = cy - m_rcClientOrig.bottom;

    if ( GetDlgItem(IDC_LIST_FILES)->GetSafeHwnd() ) {
        CRect rc;
        GetDlgItem(IDC_LIST_FILES)->GetWindowRect(rc);
        ScreenToClient( rc );
        rc.right += nDiffX;
        rc.bottom += nDiffY;
        GetDlgItem(IDC_LIST_FILES)->MoveWindow( rc, FALSE );

        GetDlgItem(IDC_STATIC_FRAME)->GetWindowRect(rc);
        ScreenToClient( rc );
        rc.right += nDiffX;
        rc.bottom += nDiffY;
        GetDlgItem(IDC_STATIC_FRAME)->MoveWindow( rc, FALSE );

        m_lcFiles.GetClientRect( rc);
        m_lcFiles.SetColumnWidth( COL_SRC_NAME        , (rc.Width()-340)/2 );
        m_lcFiles.SetColumnWidth( COL_SRC_SIZE        , 60);
        m_lcFiles.SetColumnWidth( COL_SRC_MODIFIED    , 100);
        m_lcFiles.SetColumnWidth( COL_EQUAL_SIGN    , 20);
        m_lcFiles.SetColumnWidth( COL_DST_MODIFIED    , 100);
        m_lcFiles.SetColumnWidth( COL_DST_SIZE        , 60);
        m_lcFiles.SetColumnWidth( COL_DST_NAME        , (rc.Width()-340)/2 );

        Invalidate();
    }

    m_rcClientOrig.right = cx;
    m_rcClientOrig.bottom = cy;
}

void CDirCompDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
    // TODO: Add your message handler code here and/or call default
    if ( m_rcWindowOrig.left != -1 ) {
        lpMMI->ptMinTrackSize.x = m_rcWindowOrig.Width();
        lpMMI->ptMinTrackSize.y = m_rcWindowOrig.Height();
    }

    CDialog::OnGetMinMaxInfo(lpMMI);
}

void CDirCompDlg::OnRclickListFiles(NMHDR* pNMHDR, LRESULT* pResult)
{
    // TODO: Add your control notification handler code here
    LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pNMHDR;

    if ( lpnmitem->iItem != -1 ) {
        CRect rc;
        m_lcFiles.GetWindowRect(rc);
        CPoint pt = rc.TopLeft() + lpnmitem->ptAction;
        CMenu* pmnMain = AfxGetMainWnd()->GetMenu();
        pmnMain->GetSubMenu(1)->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,
            pt.x, pt.y,    this);
    }

    *pResult = 0;
}

void CDirCompDlg::OnPopupDelSelFiles(UINT nID)
{
    CDWordArray dwa;

    POSITION pos = m_lcFiles.GetFirstSelectedItemPosition();
    CString strSrcFile;
    CString strDstFile;
    int nSrcLen = 0;
    int nDstLen = 0;
    while ( NULL != pos ) {
        int nItem = m_lcFiles.GetNextSelectedItem(pos);
        ListCtrlData* p = (ListCtrlData*)m_lcFiles.GetItemData(nItem);
        if ( p && p->comp != COMP_BOTH_INVALID && p->comp != COMP_CANCEL ) {
            if ( nID == ID_POPUP_DEL_SEL_SRC_FILES && p->comp == COMP_SRC_INVALID )
                continue;
            if ( nID == ID_POPUP_DEL_SEL_DST_FILES && p->comp == COMP_DST_INVALID )
                continue;

            dwa.Add(nItem);

            if ( p->comp != COMP_SRC_INVALID ) {
                if ( strSrcFile.IsEmpty() ) {
                    strSrcFile = p->src; strSrcFile += p->name;
                    nSrcLen = strSrcFile.GetLength();
                }
                else {
                    int nAddedLen = lstrlen(p->src) + lstrlen(p->name);
                    LPTSTR pstr = strSrcFile.GetBuffer( nSrcLen + 1 + nAddedLen );
                    lstrcpy( pstr + nSrcLen + 1 , p->src );
                    lstrcpy( pstr + nSrcLen + 1 + lstrlen(p->src), p->name );
                    nSrcLen += 1 + nAddedLen;
                    strSrcFile.ReleaseBuffer(nSrcLen);
                }
            }

            if ( p->comp != COMP_DST_INVALID ) {
                if ( strDstFile.IsEmpty() ) {
                    strDstFile = p->dst; strDstFile += p->name;
                    nDstLen = strDstFile.GetLength();
                }
                else {
                    int nAddedLen = lstrlen(p->dst) + lstrlen(p->name);
                    LPTSTR pstr = strDstFile.GetBuffer( nDstLen + 1 + nAddedLen );
                    lstrcpy( pstr + nDstLen + 1 , p->dst );
                    lstrcpy( pstr + nDstLen + 1 + lstrlen(p->dst), p->name );
                    nDstLen += 1 + nAddedLen;
                    strDstFile.ReleaseBuffer(nDstLen);
                }
            }
        }
    }

    CString strFiles;
    int nLen = 0;

    if ( nSrcLen > 0 && ( nID == ID_POPUP_DEL_SEL_SRC_FILES || nID == ID_POPUP_DEL_SEL_BOTH_FILES ) )
    {
        nLen = nSrcLen;
        LPTSTR pstr = strFiles.GetBuffer( nLen );
        memcpy( pstr, (LPCTSTR)strSrcFile, nLen );
        strFiles.ReleaseBuffer(nLen);
        nLen++;
    }

    if ( nDstLen > 0 && ( nID == ID_POPUP_DEL_SEL_DST_FILES || nID == ID_POPUP_DEL_SEL_BOTH_FILES ) )
    {
        LPTSTR pstr = strFiles.GetBuffer( nLen + nDstLen );
        memcpy( pstr + nLen, (LPCTSTR)strDstFile, nDstLen );
        nLen += nDstLen;
        strFiles.ReleaseBuffer(nLen);
        nLen++;
    }

    if ( nLen > 0 ) {
        LPTSTR pstr = strFiles.GetBuffer(nLen);
        pstr[nLen-1] = _T('\0');
        strFiles.ReleaseBuffer(nLen);

        SHFILEOPSTRUCT FileOp;
        FileOp.hwnd                    = GetSafeHwnd();
        FileOp.wFunc                = FO_DELETE;
        FileOp.pFrom                = strFiles;
        FileOp.pTo                    = NULL;
        FileOp.fFlags                = FOF_ALLOWUNDO ;
        FileOp.hNameMappings        = NULL;
        FileOp.lpszProgressTitle    = NULL;
        SHFileOperation( &FileOp );
    }

    UpdateFileListStatus(dwa);
}

void CDirCompDlg::OnPopupCopySelFiles(UINT nID)
{
    CDWordArray dwa;

    POSITION pos = m_lcFiles.GetFirstSelectedItemPosition();
    CString strSrcFile;
    CString strDstFile;
    int nSrcLen = 0;
    int nDstLen = 0;
    while ( NULL != pos ) {
        int nItem = m_lcFiles.GetNextSelectedItem(pos);
        ListCtrlData* p = (ListCtrlData*)m_lcFiles.GetItemData(nItem);
        if ( p && p->comp != COMP_BOTH_INVALID && p->comp != COMP_CANCEL ) {
            if ( nID == ID_POPUP_COPY_SEL_FILES_SRC2DST && p->comp == COMP_SRC_INVALID )
                continue;
            if ( nID == ID_POPUP_COPY_SEL_FILES_DST2SRC && p->comp == COMP_DST_INVALID )
                continue;

            dwa.Add(nItem);

            if ( strSrcFile.IsEmpty() ) {
                strSrcFile = p->src; strSrcFile += p->name;
                nSrcLen = strSrcFile.GetLength();
            }
            else {
                int nAddedLen = lstrlen(p->src) + lstrlen(p->name);
                LPTSTR pstr = strSrcFile.GetBuffer( nSrcLen + 1 + nAddedLen );
                lstrcpy( pstr + nSrcLen + 1 , p->src );
                lstrcpy( pstr + nSrcLen + 1 + lstrlen(p->src), p->name );
                nSrcLen += 1 + nAddedLen;
                strSrcFile.ReleaseBuffer(nSrcLen);
            }

            if ( strDstFile.IsEmpty() ) {
                strDstFile = p->dst; strDstFile += p->name;
                nDstLen = strDstFile.GetLength();
            }
            else {
                int nAddedLen = lstrlen(p->dst) + lstrlen(p->name);
                LPTSTR pstr = strDstFile.GetBuffer( nDstLen + 1 + nAddedLen );
                lstrcpy( pstr + nDstLen + 1 , p->dst );
                lstrcpy( pstr + nDstLen + 1 + lstrlen(p->dst), p->name );
                nDstLen += 1 + nAddedLen;
                strDstFile.ReleaseBuffer(nDstLen);
            }
        }
    }

    if ( nSrcLen > 0 ) {
        LPTSTR pstr = strSrcFile.GetBuffer( nSrcLen + 1 );
        pstr[nSrcLen] = '\0';
        nSrcLen++;
        strSrcFile.ReleaseBuffer(nSrcLen);
    }

    if ( nDstLen > 0 ) {
        LPTSTR pstr = strDstFile.GetBuffer( nDstLen + 1 );
        pstr[nDstLen] = '\0';
        nDstLen++;
        strDstFile.ReleaseBuffer(nDstLen);
    }

    if ( nSrcLen > 0 && nDstLen > 0 ) {
        SHFILEOPSTRUCT FileOp;
        FileOp.hwnd                    = GetSafeHwnd();
        FileOp.wFunc                = FO_COPY;

        if ( nID == ID_POPUP_COPY_SEL_FILES_SRC2DST ) {
            FileOp.pFrom            = strSrcFile;
            FileOp.pTo                = strDstFile;
        }
        else { // ID_POPUP_COPY_SEL_FILES_DST2SRC
            FileOp.pFrom            = strDstFile;
            FileOp.pTo                = strSrcFile;
        }
        FileOp.fFlags                = FOF_MULTIDESTFILES;
        FileOp.hNameMappings        = NULL;
        FileOp.lpszProgressTitle    = NULL;
        SHFileOperation( &FileOp );
    }

    UpdateFileListStatus(dwa);
}

void CDirCompDlg::UpdateFileListStatus(CDWordArray &dwa)
{
    for ( int i = dwa.GetSize()-1; i >= 0; i-- ) {
        ListCtrlData* p = (ListCtrlData*)m_lcFiles.GetItemData(dwa[i]);
        if ( p ) {
            CString src(p->src); src += p->name;
            CString dst(p->dst); dst += p->name;
            CFileStatus statusSrc;
            CFileStatus statusDst;
            BOOL bSrcRet = CFile::GetStatus( src, statusSrc );
            BOOL bDstRet = CFile::GetStatus( dst, statusDst );
            if ( FALSE == bSrcRet && FALSE == bDstRet )
            {
                delete p;
                m_lcFiles.DeleteItem(dwa[i]);
            }
            else if ( FALSE == bSrcRet ) {
                m_lcFiles.SetItemText( dwa[i], COL_SRC_NAME, _T("") );
                m_lcFiles.SetItemText( dwa[i], COL_SRC_SIZE, _T("") );
                m_lcFiles.SetItemText( dwa[i], COL_SRC_MODIFIED, _T("") );
                m_lcFiles.SetItemText( dwa[i], COL_EQUAL_SIGN, _T("?") );
                p->comp = COMP_SRC_INVALID;
            }
            else if ( FALSE == bDstRet ) {
                m_lcFiles.SetItemText( dwa[i], COL_DST_NAME, _T("") );
                m_lcFiles.SetItemText( dwa[i], COL_DST_SIZE, _T("") );
                m_lcFiles.SetItemText( dwa[i], COL_DST_MODIFIED, _T("") );
                m_lcFiles.SetItemText( dwa[i], COL_EQUAL_SIGN, _T("?") );
                p->comp = COMP_DST_INVALID;
            }
            else {
                CString strTmp;
                m_lcFiles.SetItemText( dwa[i], COL_SRC_NAME, p->name );
                strTmp.Format(_T("%dKB"), (statusSrc.m_size + 1023)>>10);
                m_lcFiles.SetItemText( dwa[i], COL_SRC_SIZE, strTmp );
                m_lcFiles.SetItemText( dwa[i], COL_SRC_MODIFIED, statusSrc.m_mtime.Format("%c") );
                m_lcFiles.SetItemText( dwa[i], COL_EQUAL_SIGN, _T("?") );
                m_lcFiles.SetItemText( dwa[i], COL_DST_NAME, p->name );
                strTmp.Format(_T("%dKB"), (statusDst.m_size + 1023)>>10);
                m_lcFiles.SetItemText( dwa[i], COL_DST_SIZE, strTmp );
                m_lcFiles.SetItemText( dwa[i], COL_DST_MODIFIED, statusDst.m_mtime.Format("%c") );
                p->comp = COMP_IDENTICAL;
            }
        }
    }
}

DWORD WINAPI CDirCompDlg::FileCountThreadFunc2(LPVOID param)
{
    CDirCompDlg* pto = (CDirCompDlg*)param;
    return pto->FileCountThreadMemberFunc2();
}

DWORD CDirCompDlg::FileCountThreadMemberFunc2()
{
    m_ctrlFileCountProgress.SetPos(0);

    POSITION pos = m_lcFiles.GetFirstSelectedItemPosition();
    while ( NULL != pos ) {
        int nItem = m_lcFiles.GetNextSelectedItem(pos);
        ListCtrlData* p = (ListCtrlData*)m_lcFiles.GetItemData(nItem);
        if (p) m_dwTotalNumberOfFiles++;
    }
    m_bFileCountCompleted = TRUE;
    return m_dwTotalNumberOfFiles;
}

DWORD WINAPI CDirCompDlg::ThreadFunc2(LPVOID param)
{
    Sleep(500);
    CDirCompDlg* pto = (CDirCompDlg*)param;
    return pto->ThreadMemberFunc2();
}

DWORD CDirCompDlg::ThreadMemberFunc2()
{
    m_dwIdenticalFiles = 0;
    m_dwDifferentFiles = 0;

    POSITION pos = m_lcFiles.GetFirstSelectedItemPosition();
    while ( NULL != pos ) {
        // update file completed progress
        if ( m_bFileCountCompleted && m_dwTotalNumberOfFiles > 0 ) {
            int nIndex = ( m_dwIdenticalFiles + m_dwDifferentFiles + 1 ) * 1000 / m_dwTotalNumberOfFiles;
            if ( m_ctrlFileCountProgress.GetPos() != nIndex ) {
                m_ctrlFileCountProgress.SetPos(nIndex);
            }
        }

        if( WaitForSingleObject( m_hStopEvent, 0) != WAIT_TIMEOUT) {
            return -1;
        }

        int nItem = m_lcFiles.GetNextSelectedItem(pos);
        ListCtrlData* p = (ListCtrlData*)m_lcFiles.GetItemData(nItem);

        if (!p) continue;

        CString strSrc(p->src); strSrc += p->name;
        CString strDst(p->dst); strDst += p->name;

        int nRet = CompareFile( strSrc, strDst );

        CString strTmp;
        CFileStatus status;

        switch ( nRet ) {
        case COMP_IDENTICAL:
        case COMP_GREATER:
        case COMP_LESS:
            {
                m_lcFiles.SetItemText( nItem, COL_SRC_NAME, p->name );
                CFile::GetStatus( strSrc, status );
                strTmp.Format(_T("%dKB"), (status.m_size + 1023)>>10);
                m_lcFiles.SetItemText( nItem, COL_SRC_SIZE, strTmp );
                m_lcFiles.SetItemText( nItem, COL_SRC_MODIFIED, status.m_mtime.Format("%c") );

                m_lcFiles.SetItemText( nItem, COL_DST_NAME, p->name);
                CFile::GetStatus( strDst, status );
                strTmp.Format(_T("%dKB"), (status.m_size + 1023)>>10);
                m_lcFiles.SetItemText( nItem, COL_DST_SIZE, strTmp );
                m_lcFiles.SetItemText( nItem, COL_DST_MODIFIED, status.m_mtime.Format("%c") );
            }
            break;

        case COMP_DST_INVALID:
            {
                m_lcFiles.SetItemText( nItem, COL_SRC_NAME, p->name );
                CFile::GetStatus( strSrc, status );
                strTmp.Format(_T("%dKB"), (status.m_size + 1023)>>10);
                m_lcFiles.SetItemText( nItem, COL_SRC_SIZE, strTmp );
                m_lcFiles.SetItemText( nItem, COL_SRC_MODIFIED, status.m_mtime.Format("%c") );
            }
            break;

        case COMP_SRC_INVALID:
            {
                m_lcFiles.SetItemText( nItem, COL_DST_NAME, p->name);
                CFile::GetStatus( strDst, status );
                strTmp.Format(_T("%dKB"), (status.m_size + 1023)>>10);
                m_lcFiles.SetItemText( nItem, COL_DST_SIZE, strTmp );
                m_lcFiles.SetItemText( nItem, COL_DST_MODIFIED, status.m_mtime.Format("%c") );
            }
            break;

        case COMP_BOTH_INVALID:
        case COMP_CANCEL:
            {
                m_lcFiles.SetItemText( nItem, COL_SRC_NAME, p->name );
                m_lcFiles.SetItemText( nItem, COL_SRC_SIZE, _T("") );
                m_lcFiles.SetItemText( nItem, COL_SRC_MODIFIED, _T("") );
                m_lcFiles.SetItemText( nItem, COL_DST_NAME, p->name );
                m_lcFiles.SetItemText( nItem, COL_DST_SIZE, _T("") );
                m_lcFiles.SetItemText( nItem, COL_DST_MODIFIED, _T("") );
            }
            break;
        }

        switch( nRet ) {
        case COMP_IDENTICAL:
            m_lcFiles.SetItemText( nItem, COL_EQUAL_SIGN, _T("="));
            m_dwIdenticalFiles++;
            break;
        case COMP_GREATER:
        case COMP_DST_INVALID:
            m_lcFiles.SetItemText( nItem, COL_EQUAL_SIGN, _T(">"));
            m_dwDifferentFiles++;
            break;
        case COMP_LESS:
        case COMP_SRC_INVALID:
            m_lcFiles.SetItemText( nItem, COL_EQUAL_SIGN, _T("<"));
            m_dwDifferentFiles++;
            break;
        case COMP_BOTH_INVALID:
            m_lcFiles.SetItemText( nItem, COL_EQUAL_SIGN, _T("X"));
            m_dwDifferentFiles++;
            break;
        case COMP_CANCEL:
            m_lcFiles.SetItemText( nItem, COL_EQUAL_SIGN, _T("!"));
            m_dwDifferentFiles++;
            break;
        }

        if ( COMP_CANCEL == nRet ) break;
    }

    CString s;
    s.Format(_T(" %d files compared."), m_dwIdenticalFiles + m_dwDifferentFiles );
    m_sttFileComparing.SetWindowText(s);
    PostMessage( UM_END_OF_COMPARE );
    return 0;
}

void CDirCompDlg::OnPopupCompSelFiles()
{
    if ( NULL == m_lcFiles.GetFirstSelectedItemPosition() ) {
        AfxMessageBox(_T("No selected files!") );
        return;
    }

    m_cbSrcFolder.EnableWindow(FALSE);
    m_btnSrcBrowser.EnableWindow(FALSE);
    m_cbDstFolder.EnableWindow(FALSE);
    m_btnDstBrowser.EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT_INCLUDE)->EnableWindow(FALSE);
    GetDlgItem(IDC_CHECK_INCLUDE_SUBFOLDER)->EnableWindow(FALSE);
    m_btnStart.EnableWindow(FALSE);
    m_lcFiles.EnableWindow(FALSE);
    m_btnStop.EnableWindow();

    m_dwTotalNumberOfFiles = 0;
    m_bFileCountCompleted = FALSE;

    m_hFileCountThread = (HANDLE)_beginthreadex( NULL, 0,
        (PBEGINTHREADEX_THREADFUNC)FileCountThreadFunc2, (LPVOID)this, 0,
        (PBEGINTHREADEX_THREADID)&m_dwFileCountThreadID);

    if( m_hFileCountThread <= 0)
        AfxMessageBox(_T("Error to create m_hFileCountThread"));
    else {
        TRACE(_T("The thread %x has created\n"), m_dwFileCountThreadID);
    }

    m_hThread = (HANDLE)_beginthreadex( NULL, 0,
        (PBEGINTHREADEX_THREADFUNC)ThreadFunc2, (LPVOID)this, 0,
        (PBEGINTHREADEX_THREADID)&m_dwThreadID);

    if( m_hThread <= 0)
        AfxMessageBox(_T("Error to create m_hThread"));
    else {
        TRACE(_T("The thread %x has created\n"), m_dwThreadID);
    }
}

void CDirCompDlg::OnNMDblclkListFiles(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pNMHDR;
    *pResult = 0;

    int nItem = lpnmitem->iItem;
    int nSubItem = lpnmitem->iSubItem;

    if ( nItem < 0 )
        return;

    CString srcName = m_lcFiles.GetItemText(nItem,COL_SRC_NAME);
    CString dstName = m_lcFiles.GetItemText(nItem,COL_DST_NAME);

    if ( nSubItem == COL_SRC_NAME || nSubItem == COL_DST_NAME ) {
        if ( srcName.Find(_T("Folder:")) >= 0 ) {
            int nFirst = _tcslen(_T("Folder: [ "));
            int nCount = srcName.GetLength() - nFirst - _tcslen(_T(" ]"));
            CString strFolderName = ((nSubItem == COL_SRC_NAME) ? m_sSrcDir : m_sDstDir)
                + _T("\\") + srcName.Mid( nFirst, nCount );
            int nRet = (int)ShellExecute( GetSafeHwnd(), _T("open"), strFolderName, NULL, strFolderName, SW_SHOW );
            if ( nRet <= 32 )
                ShowErrorDialog();
        }
        else {
            CString strFolderName;
            int nFolderIndex = nItem;
            for (; nFolderIndex >= 0; nFolderIndex-- ) {
                strFolderName = m_lcFiles.GetItemText(nFolderIndex,COL_SRC_NAME);
                if ( strFolderName.Find(_T("Folder:")) >= 0 ) {
                    break;
                }
            }
            if ( nFolderIndex < 0 ) {
                strFolderName = ((nSubItem == COL_SRC_NAME) ? m_sSrcDir : m_sDstDir);
            }
            else {
                int nFirst = _tcslen(_T("Folder: [ "));
                int nCount = strFolderName.GetLength() - nFirst - _tcslen(_T(" ]"));
                strFolderName = ((nSubItem == COL_SRC_NAME) ? m_sSrcDir : m_sDstDir)
                    + _T("\\") + strFolderName.Mid( nFirst, nCount );
            }
            CString strPathName = strFolderName + _T("\\") + srcName;
            int nRet = (int)ShellExecute( GetSafeHwnd(), _T("open"), strPathName, NULL, strFolderName, SW_SHOW );
            if ( nRet <= 32 )
                ShowErrorDialog();
        }
    }

}
