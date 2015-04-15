/////////////////////////////////////////////////////////////////////////////
// Name:        OpenCTagsFrm.h
// Purpose:
// Author:
// Modified by:
// Created:
// RCS-ID:
// Copyright:   (C)2005
// Licence:     wxWindows
// Generated with wxWinWizard by KoanSoftware.com
/////////////////////////////////////////////////////////////////////////////

#ifndef OpenCTagsFrame_H
#define OpenCTagsFrame_H

#include "octCFileIndex.h"

#include "resource.h"
// Define a new frame type: this is going to be our main frame
class octSLstFileIndex {
public:
	octSLstFileIndex(octCFileIndex *pFIndex) {m_pCFileIndex = pFIndex; m_pNext = NULL;}
	~octSLstFileIndex() {
		if (m_pCFileIndex) delete m_pCFileIndex;
	}

	octCFileIndex *m_pCFileIndex;
	octSLstFileIndex *m_pNext;
};

class octListCtrl  {
public :
	octListCtrl ();

	//ListView Layer
	int Create(POINT pt);
	int CreateListView(HWND hwndParent);
	LPARAM GetItemData(int item);
	HWND GetHWnd() {return m_hWnd;}
	int GetIconIndex(char kind);

	//Specific
	void OpenOneTag(octSFileIndex *pSIndex);
	int SetListFromTag(octCFileIndex *pFIndex, char * szTag);

	// event handlers (these functions should _not_ be virtual)
	void OnKeyDown(int keyCode);
	void OnKillFocus();
	void OnDclick();

	//variables
	WNDPROC m_oldWP;
	HWND m_hWndParent;
	HWND m_hWnd;

	bool m_bIsClosing;
	bool m_bOneTagOpened;
private:
};
#endif // OpenCTagsFrame_H
