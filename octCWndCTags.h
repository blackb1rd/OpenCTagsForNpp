#pragma once

#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include "PluginInterface.h"

extern HWND	g_hSciCurrHandle;
extern NppData nppData;

enum SCTagsState {
	isNotStarted = 0,
	isRunning = 1,
	isFinished = 2,
	isCanceled = 3,
	isError = 4,
};

class octCWndCTags
{
public:
	octCWndCTags();
	~octCWndCTags(void);
	HWND Create(const char *szDir);
	int OnCreateWnd(HWND hWnd);

	HWND m_hWnd;
	HWND m_hWndPB;
	HWND m_hWndBt;

	char m_szDirectory[_MAX_PATH];
	SHELLEXECUTEINFO m_shEx;

	enum SCTagsState m_ctState;
};
