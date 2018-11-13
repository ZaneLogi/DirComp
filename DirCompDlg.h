// DirCompDlg.h : header file
//

#if !defined(AFX_DIRCOMPDLG_H__3ADBA587_B7A0_11D3_9F6B_0000E203B5D8__INCLUDED_)
#define AFX_DIRCOMPDLG_H__3ADBA587_B7A0_11D3_9F6B_0000E203B5D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef unsigned (WINAPI *PBEGINTHREADEX_THREADFUNC)(LPVOID lpThreadParameter);
typedef unsigned *PBEGINTHREADEX_THREADID;

#define UM_END_OF_COMPARE    (WM_USER+20)

#include "GradientProgressCtrl.h"
#include "CustomDrawListCtrl.h"
#include <afxmt.h>
#include "afxwin.h"

#define COL_SRC_NAME        0
#define COL_SRC_SIZE        1
#define COL_SRC_MODIFIED    2
#define COL_EQUAL_SIGN        3
#define COL_DST_MODIFIED    4
#define COL_DST_SIZE        5
#define COL_DST_NAME        6

class CMyListCtrl : public CCustomDrawListCtrl
{
protected:
    virtual bool IsNotifyItemDraw() { return true; }
    virtual COLORREF TextColorForItem(int nItem, UINT nState, LPARAM lParam)
    {
        if ( !IsWindowEnabled() )
            return RGB(0,0,0);

        CString sign = GetItemText(nItem,COL_EQUAL_SIGN);
        CString src  = GetItemText(nItem,COL_SRC_NAME);
        if ( src.Find(_T("Folder:")) >= 0 ) {
            return RGB(255,255,255);
        }
        else {
            return ( sign != "=" ) ? RGB(255,0,0) : RGB(0,0,0);
        }
    }
    virtual COLORREF BkColorForItem(int nItem, UINT nState, LPARAM lParam)
    {
        if ( !IsWindowEnabled() )
            return RGB(160,160,160);

        CString sign = GetItemText(nItem,COL_EQUAL_SIGN);
        CString src  = GetItemText(nItem,COL_SRC_NAME);
        if ( src.Find(_T("Folder:")) >= 0 ) {
            return RGB(0,128,0);
        }
        else {
            return ( sign == "=" ) ? RGB(236,236,236) : RGB(255,236,236);
        }
    }
};

/////////////////////////////////////////////////////////////////////////////
// CDirCompDlg dialog

class CDirCompDlg : public CDialog
{
private:
    void UpdateFileListStatus( CDWordArray& dwa );
    void ClearListCtrl();
    void WaitForExit();
// Construction
public:
    CDirCompDlg(CWnd* pParent = NULL);    // standard constructor
    ~CDirCompDlg();
    static DWORD WINAPI FileCountThreadFunc(LPVOID param);
    static DWORD WINAPI ThreadFunc(LPVOID param);
    int CompareFile(CString sSrc, CString sDst);

    static DWORD WINAPI FileCountThreadFunc2(LPVOID param);
    static DWORD WINAPI ThreadFunc2(LPVOID param);

protected:
    DWORD    FileCountThreadMemberFunc();
    DWORD    SearchFolder( CString sParentDir );
    DWORD    ThreadMemberFunc();
    DWORD    CompareFolder( CString sParentDir );

    DWORD    FileCountThreadMemberFunc2();
    DWORD    ThreadMemberFunc2();

    HANDLE    m_hThread;
    DWORD    m_dwThreadID;
    HANDLE    m_hStopEvent;

    HANDLE    m_hFileCountThread;
    DWORD    m_dwFileCountThreadID;
    HANDLE    m_hFileCountStopEvent;

    CCriticalSection    m_cs;

    CString    m_sSrcDir;
    CString    m_sDstDir;

    BOOL    m_bFileCountCompleted;
    DWORD    m_dwTotalNumberOfFiles;
    DWORD    m_dwIdenticalFiles;
    DWORD    m_dwDifferentFiles;

    CRect    m_rcWindowOrig;
    CRect    m_rcClientOrig;

// Dialog Data
    //{{AFX_DATA(CDirCompDlg)
    enum { IDD = IDD_DIRCOMP_DIALOG };
    CGradientProgressCtrl    m_ctrlFileCountProgress;
    CMyListCtrl                m_lcFiles;
    CButton                    m_btnEqual;
    CButton                    m_btnNotEqual;
    CButton                    m_btnFindNextEqual;
    CButton                    m_btnFindNextNotEqual;
    CStatic                    m_sttFileComparing;
    CProgressCtrl            m_ctrlProgress;
    CButton                    m_btnStop;
    CButton                    m_btnStart;
    CComboBox                m_cbDstFolder;
    CComboBox                m_cbSrcFolder;
    CButton                    m_btnDstBrowser;
    CButton                    m_btnSrcBrowser;
    CString                    m_sInclude;
    BOOL                    m_bIncludeSubfolders;
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDirCompDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    //{{AFX_MSG(CDirCompDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBtnSrcBrowser();
    afx_msg void OnBtnDstBrowser();
    afx_msg void OnEditchangeCbSrcFolder();
    afx_msg void OnEditchangeCbDstFolder();
    afx_msg void OnBtnStart();
    afx_msg void OnBtnStop();
    virtual void OnCancel();
    afx_msg void OnBtnEqual();
    afx_msg void OnBtnNotEqual();
    afx_msg void OnBnClickedBtnFindNextEqual();
    afx_msg void OnBnClickedBtnFindNextNotEqual();
    afx_msg void OnSelchangeCbSrcFolder();
    afx_msg void OnSelchangeCbDstFolder();
    afx_msg LONG OnEndOfCompare( WPARAM, LPARAM);
    afx_msg void OnAppAbout();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
    afx_msg void OnRclickListFiles(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDestroy();
    afx_msg void OnPopupCompSelFiles();
    //}}AFX_MSG
    afx_msg void OnPopupDelSelFiles(UINT nID);
    afx_msg void OnPopupCopySelFiles(UINT nID);
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnNMDblclkListFiles(NMHDR *pNMHDR, LRESULT *pResult);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIRCOMPDLG_H__3ADBA587_B7A0_11D3_9F6B_0000E203B5D8__INCLUDED_)
