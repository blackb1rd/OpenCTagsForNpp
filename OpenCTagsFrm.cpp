/////////////////////////////////////////////////////////////////////////////
// Name:        OpenCTagsFrm.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     
// RCS-ID:      
// Copyright:   (C)2005 
// Licence:     wxWindows
// Generated with wxWinWizard by KoanSoftware.com
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
#include "wxprec.h"


#include "OpenCTagsApp.h"
#include "OpenCTagsFrm.h"
#include "PluginInterface.h"

#include <commctrl.h>

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------
// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = 1,
    Minimal_About
};

#define IDI_LISTCTRL 100
#define IDI_TEXTCTRL 101

#define LV_SZMAX 1024
// Constants for icons hendling
const char* cstKinds = "cdefFgmpstuv";
const UINT cstIcons[] = {
	IDI_ICONC,
	IDI_ICOND,
	IDI_ICONE,
	IDI_ICONF,
	IDI_ICONFF,
	IDI_ICONG,
	IDI_ICONM,
	IDI_ICONP,
	IDI_ICONS,
	IDI_ICONT,
	IDI_ICONU,
	IDI_ICONV
};
/*
*/

LRESULT APIENTRY LViewWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CREATE:
			return theApp.m_pMainLW->CreateListView(hwnd);
		case WM_KEYDOWN:
			theApp.m_pMainLW->OnKeyDown((int)wParam);
			return 0;
		case WM_LBUTTONDBLCLK:
			theApp.m_pMainLW->OnDclick();
			return 0;
		case WM_KILLFOCUS:
			if (GetFocus() != theApp.m_pMainLW->m_hWnd && GetFocus() != theApp.m_pMainLW->m_hWndParent)
				theApp.m_pMainLW->OnKillFocus();
			return 0;
		case WM_NOTIFY:
			{
				LPNMHDR pnmh = (LPNMHDR)lParam;
				switch (pnmh->code) {
				case NM_KILLFOCUS :
					PostMessage(theApp.m_pMainLW->m_hWndParent, WM_KILLFOCUS,NULL, NULL);
					return 0;
				case LVN_KEYDOWN:
					{
						LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
						switch (pnkd->wVKey) {
						case VK_TAB:
						case VK_RETURN:
						case VK_UP:
						case VK_DOWN:
						case VK_ESCAPE:
						case VK_SPACE:
							theApp.m_pMainLW->OnKeyDown(pnkd->wVKey);
							return DefWindowProc(hwnd,uMsg, wParam, lParam);
						}
						PostMessage(g_hSciCurrHandle, WM_KEYDOWN, (WPARAM)pnkd->wVKey, (LPARAM)0);
					}
					break;
				case NM_DBLCLK:
					theApp.m_pMainLW->OnDclick();
					return 0;
				}
			}
	}
//	CallWindowProc(theApp.m_pMainLW->m_oldWP, hwnd, uMsg, wParam, lParam);
	return DefWindowProc(hwnd,uMsg, wParam, lParam);
}

octListCtrl::octListCtrl () {
	m_hWnd = NULL;
	m_hWndParent = NULL;
	m_bIsClosing = false;
	m_bOneTagOpened = false;
}

int octListCtrl::Create(POINT pt) {
	INITCOMMONCONTROLSEX icex;

// Ensure that the common control DLL is loaded. 

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex); 
	
	WNDCLASS       wndclass;
  wndclass.style          = CS_HREDRAW|CS_VREDRAW;
  wndclass.lpfnWndProc    = LViewWndProc;
  wndclass.cbClsExtra     = 0;
  wndclass.cbWndExtra     = 0;
  wndclass.hInstance      = ghDllInstance;
  wndclass.hIcon          = NULL;
  wndclass.hCursor        = LoadCursor (NULL, IDC_ARROW);
  wndclass.hbrBackground  = (HBRUSH)GetStockObject (WHITE_BRUSH);
  wndclass.lpszMenuName   = NULL;
  wndclass.lpszClassName  = TEXT("OpenCTagsNPP");

  RegisterClass (&wndclass);

	m_hWndParent = CreateWindow(wndclass.lpszClassName,
                TEXT("OpenCTags for Notepad++"),
                WS_POPUP,
                pt.x,
                pt.y,
                OPENCTAGS_WIDTH,
                OPENCTAGS_HEIGHT,
                g_hSciCurrHandle,
                NULL,
                ghDllInstance,
                NULL);

//	::SendMessage(g_hSciCurrHandle, SCI_BEGINUNDOACTION, 0, 0);
	return m_hWndParent ? 1 : 0;
}
// Create the list-view window in report view with label 
// editing enabled.
int octListCtrl::CreateListView(HWND hwndParent) {
//	GetClientRect (hWndParent, &rcl); 
	DWORD dwErr;
	RECT rct;
	GetClientRect(hwndParent, &rct);
	m_hWnd = CreateWindow ( WC_LISTVIEW, TEXT("OpenCTags ListView"), 
        WS_CHILD | LVS_REPORT | LVS_SINGLESEL |LVS_NOCOLUMNHEADER |WS_VISIBLE|WS_BORDER, 
        0, 0, rct.right - rct.left, rct.bottom - rct.top, 
        hwndParent, NULL, ghDllInstance, NULL);
	if (!m_hWnd) {
		dwErr = GetLastError();
		return 0;
	}
	ListView_SetExtendedListViewStyle(m_hWnd, LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);
	TCHAR szText[256];
//	ListView_SetBkColor(m_hWnd, GetSysColor(COLOR_INFOBK));
	LVCOLUMN lvcol;
	lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvcol.fmt = LVCFMT_LEFT;
	lvcol.cx = 120;
	lvcol.pszText = szText;
	_tcscpy(szText,_T("Tag"));
	lvcol.iSubItem = 0;
	dwErr = ListView_InsertColumn(m_hWnd, 0, &lvcol);

	_tcscpy(szText,_T("Line"));
	lvcol.iSubItem = 1;
	dwErr = ListView_InsertColumn(m_hWnd, 1, &lvcol);

	_tcscpy(szText,_T("File"));
	lvcol.iSubItem = 2;
	dwErr = ListView_InsertColumn(m_hWnd, 2, &lvcol);
	HIMAGELIST hImgList = ImageList_LoadImage(ghDllInstance, MAKEINTRESOURCE(IDR_ICONS),16,0,CLR_DEFAULT,IMAGE_BITMAP,LR_DEFAULTCOLOR);
	int nImC = ImageList_GetImageCount(hImgList);
	if (!ListView_SetImageList(m_hWnd, hImgList, LVSIL_SMALL)) {
		dwErr = GetLastError();
	}
	ListView_SetBkColor(m_hWnd, GetSysColor(COLOR_INFOBK));
	ListView_SetTextBkColor(m_hWnd, GetSysColor(COLOR_INFOBK));
	return (1);
}

/*ListView layer*/
LPARAM octListCtrl::GetItemData(int item) {
	LVITEM lvitem;
	lvitem.mask = LVIF_PARAM;
	lvitem.iItem = item;
	ListView_GetItem(m_hWnd, &lvitem);
	return lvitem.lParam;
}

int octListCtrl::GetIconIndex(char kind)
{
	char *pFound = strchr(cstKinds,kind);
	if (!pFound) return -1;
	int index = (int)(pFound - cstKinds);

	HIMAGELIST hImgList = ListView_GetImageList(m_hWnd, LVSIL_SMALL);
	if (index >= ImageList_GetImageCount(hImgList)) return -1;
	return index;
}
//List of tags treatement
int octListCtrl::SetListFromTag(octCFileIndex *pFIndex, char * szTag) {
	int nItems = ListView_GetItemCount(m_hWnd);
	bool bNewListe = true;
	if (nItems > 0) {
		octSFileIndex *pSIndex = (octSFileIndex *)GetItemData(0);
		if (pSIndex->ptag[0] == szTag[0]) bNewListe = false;
	}

	if (bNewListe) {
		ListView_DeleteAllItems(m_hWnd);
		char *pFound = NULL, szFirstLetter[2];
		static TCHAR szText[LV_SZMAX];
		int i = 0;
		LVITEM lvItem;
		memset(&lvItem, 0, sizeof(LVITEM));
		lvItem.pszText = szText;
		nItems = 0;
		
		szFirstLetter[0] = szTag[0]; szFirstLetter[1] = '\0';
		
		for (octSFileIndex *pSIndex = pFIndex->findFirstTag(szFirstLetter); 
				pSIndex; 
				pSIndex = pFIndex->findNextTag() ) {

			lvItem.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
			mbstowcs(lvItem.pszText, pSIndex->ptag, LV_SZMAX-1);
			lvItem.iImage = GetIconIndex(pSIndex->kind);
			lvItem.iItem = nItems;
			lvItem.iSubItem = 0;
			lvItem.lParam = (LPARAM)pSIndex;
			lvItem.iItem = ListView_InsertItem(m_hWnd, &lvItem);
			DWORD dwError = GetLastError();
	//Strings having /^function any()$/;" must be displayed as function any()		
			char *strAdress = strdup(pSIndex->padress);
			int iDeb=0, iLen=(int)strlen(strAdress);
			if (iLen > 0 && strAdress[0] == '/') iDeb++;
			if (iLen > 1 && strAdress[1] == '^') iDeb++;
			iLen = (int)(strchr(strAdress, ';') - strAdress);
			if (iLen<=0) iLen = (int)strlen(strAdress);
			if (iLen > 0 && strAdress[iLen - 1] == '/') iLen--;
			if (iLen > 0 && strAdress[iLen - 1] == '$') iLen--;
			strcpy(strAdress,pSIndex->padress+iDeb);
			strAdress[iLen-iDeb] = '\0';
			lvItem.mask = LVIF_TEXT;
			
			lvItem.iSubItem = 1;
			mbstowcs(lvItem.pszText, strAdress, LV_SZMAX-1);
			ListView_SetItem(m_hWnd, &lvItem);

			lvItem.iSubItem = 2;
			mbstowcs(lvItem.pszText, pSIndex->pfilename, LV_SZMAX-1);
			ListView_SetItem(m_hWnd, &lvItem);

			free(strAdress);
			nItems++;
		}
		nItems = ListView_GetItemCount(m_hWnd);
		if (nItems) {
			ListView_SetColumnWidth(m_hWnd,0,LVSCW_AUTOSIZE);
			ListView_SetColumnWidth(m_hWnd,0,100);
			ListView_SetColumnWidth(m_hWnd,0,LVSCW_AUTOSIZE);
		}
	}
	if (nItems > 0) {
		octSFileIndex * pSFirstDansListe = (octSFileIndex *)GetItemData(0), * pSIndex = pFIndex->findFirstTag(szTag);
		int itemSelected = 0;
		if (pSIndex && pSIndex->nIndex >= pSFirstDansListe->nIndex &&
				(itemSelected = pSIndex->nIndex - pSFirstDansListe->nIndex) < nItems ) {
			
			ListView_SetItemState(m_hWnd, itemSelected, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			ListView_EnsureVisible(m_hWnd, itemSelected, FALSE);
		} else {
//Deselect Items;
			int iIndexSel = -1;
			while ((iIndexSel = ListView_GetNextItem(m_hWnd, iIndexSel, LVNI_SELECTED)) >= 0) {
				ListView_SetItemState(m_hWnd, iIndexSel, 0, LVIS_SELECTED);
			}
		}
	}

	return (nItems);
}

void octListCtrl::OpenOneTag(octSFileIndex *pSIndex) {
	char szFileName[_MAX_PATH];
	int nLineNumber = GetFileLineNumber(pSIndex, szFileName);
	if (nLineNumber >= 0) {
		m_bOneTagOpened = true;
		GoToFileLine(szFileName, nLineNumber);
		theApp.insertCurrentCall();
	}
}

void octListCtrl::OnDclick() {
	long itemSel = -1;
	itemSel = ListView_GetNextItem(m_hWnd, itemSel, LVNI_SELECTED); 
	if (itemSel == -1) return;

	octSFileIndex *pSIndex = (octSFileIndex *)GetItemData(itemSel);
	OpenOneTag(pSIndex);
	SetFocus(g_hSciCurrHandle);
}

void octListCtrl::OnKeyDown(int keyCode) {
	if (keyCode == VK_TAB || keyCode == VK_SPACE || keyCode == VK_RETURN) {
		long itemSel = -1;
		itemSel = ListView_GetNextItem(m_hWnd, itemSel, LVNI_SELECTED); 
		if (itemSel == -1) return;

		octSFileIndex *pSIndex = (octSFileIndex *)GetItemData(itemSel);
		m_bIsClosing = true;
		if (keyCode == VK_SPACE) {
			OpenOneTag(pSIndex);
		} else {
			replaceCurrentWord(pSIndex->ptag);
		}
		SetFocus(g_hSciCurrHandle);
	} else if (keyCode == VK_ESCAPE) {
		m_bIsClosing = true;
		SetFocus(g_hSciCurrHandle);
	}
}

void octListCtrl::OnKillFocus(){
/*	::SendMessage(g_hSciCurrHandle, SCI_ENDUNDOACTION, 0, 0);
	if (m_bOneTagOpened) {
		::SendMessage(g_hSciCurrHandle, SCI_UNDO, 0, 0);
	}
*/
	DestroyWindow(m_hWndParent);
}
