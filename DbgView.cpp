// DbgView.cpp : implementation file
//

#include "stdafx.h"
#include "Editor.h"
#include "DbgView.h"
#include "ParentWnd.h"
#include "MFCOutput.h"

class CDbgView : public CEditor
{
public:
	CDbgView(CParentWnd* pParentWnd);
	virtual ~CDbgView();
	void AppendText(LPCTSTR szMsg);

private:
	ULONG HandleChange(UINT nID);
	void  OnEditAction1();
	void  OnEditAction2();

	void OnOK();
	void OnCancel();
	virtual BOOL CheckAutoCenter();
	CParentWnd* m_lpNonModalParent;
	CWnd* m_hwndCenteringWindow;
	BOOL bPaused;
};

CDbgView* g_DgbView = NULL;

// Displays the debug viewer - only one may exist at a time
void DisplayDbgView(CParentWnd* pParentWnd)
{
	if (!g_DgbView) g_DgbView = new CDbgView(pParentWnd);
}

void OutputToDbgView(LPCTSTR szMsg)
{
	if (!g_DgbView) return;
	g_DgbView->AppendText(szMsg);
} // OutputToDbgView

enum __DbgViewFields
{
	DBGVIEW_TAGS,
	DBGVIEW_VIEW,
};

static TCHAR* CLASS = _T("CDbgView");

CDbgView::CDbgView(CParentWnd* pParentWnd):
CEditor(pParentWnd,IDS_DBGVIEW,IDS_DBGVIEWPROMPT,0,CEDITOR_BUTTON_OK|CEDITOR_BUTTON_ACTION1|CEDITOR_BUTTON_ACTION2,IDS_CLEAR,IDS_PAUSE)
{
	TRACE_CONSTRUCTOR(CLASS);
	CreateControls(2);
	InitSingleLine(DBGVIEW_TAGS,IDS_REGKEY_DEBUG_TAG,NULL,false);
	SetHex(DBGVIEW_TAGS,GetDebugLevel());
	InitMultiLine(DBGVIEW_VIEW,NULL,NULL,true);
	bPaused = false;

	HRESULT hRes = S_OK;
	m_lpszTemplateName = MAKEINTRESOURCE(IDD_BLANK_DIALOG);

	m_lpNonModalParent = pParentWnd;
	if (m_lpNonModalParent) m_lpNonModalParent->AddRef();

	m_hwndCenteringWindow = GetActiveWindow();

	HINSTANCE hInst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
	HRSRC hResource = NULL;
	EC_D(hResource,::FindResource(hInst, m_lpszTemplateName, RT_DIALOG));
	HGLOBAL hTemplate = NULL;
	EC_D(hTemplate,LoadResource(hInst, hResource));
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);
	EC_B(CreateDlgIndirect(lpDialogTemplate, m_lpNonModalParent, hInst));
} // CDbgView::CDbgView

CDbgView::~CDbgView()
{
	TRACE_DESTRUCTOR(CLASS);
	if (m_lpNonModalParent) m_lpNonModalParent->Release();
	g_DgbView = NULL;
} // CDbgView::~CDbgView

void CDbgView::OnOK()
{
	ShowWindow(SW_HIDE);
	delete this;
} // CDbgView::OnOK

void CDbgView::OnCancel()
{
	OnOK();
} // CDbgView::OnCancel

// MFC will call this function to check if it ought to center the dialog
// We'll tell it no, but also place the dialog where we want it.
BOOL CDbgView::CheckAutoCenter()
{
	// We can make the debug viewer wider - OnSize will fix the height for us
	SetWindowPos(NULL,0,0,800,0,NULL);
	CenterWindow(m_hwndCenteringWindow);
	return false;
} // CDbgView::CheckAutoCenter

ULONG CDbgView::HandleChange(UINT nID)
{
	ULONG i = CEditor::HandleChange(nID);

	if ((ULONG) -1 == i) return (ULONG) -1;

	switch (i)
	{
	case (DBGVIEW_TAGS):
		{
			ULONG ulTag = GetHexUseControl(DBGVIEW_TAGS);
			SetDebugLevel(ulTag); return true;
		}
		break;

	default:
		break;
	}

	return i;
} // CDbgView::HandleChange

// Clear
void CDbgView::OnEditAction1()
{
	ClearView(DBGVIEW_VIEW);
} // CDbgView::OnEditAction1

// Pause
void CDbgView::OnEditAction2()
{
	bPaused = !bPaused;
} // CDbgView::OnEditAction2

void CDbgView::AppendText(LPCTSTR szMsg)
{
	if (bPaused) return;
	AppendString(DBGVIEW_VIEW,szMsg);
} // CDbgView::AppendText