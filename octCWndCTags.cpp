#include ".\octcwndctags.h"
#include "OpenCTagsApp.h"
#include "PluginInterface.h"

#include <commctrl.h>
//#include <psapi.h>

#define CLASS_CTAGSWND "CTagsForOpenCTags"


LRESULT APIENTRY CTagsWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CREATE:
			return theApp.m_CTagsWnd.OnCreateWnd(hwnd);
		case WM_COMMAND:
			{
				if ((HWND)lParam == theApp.m_CTagsWnd.m_hWndBt) {
					DWORD dwRet;
					switch (HIWORD(wParam)) {
						case BN_CLICKED:
							if (theApp.m_CTagsWnd.m_shEx.hProcess) {
								GetExitCodeProcess(theApp.m_CTagsWnd.m_shEx.hProcess, &dwRet);
								if (dwRet == STILL_ACTIVE) {
									theApp.m_CTagsWnd.m_ctState = isCanceled;
									TerminateProcess(theApp.m_CTagsWnd.m_shEx.hProcess, 0);
								}
							}
						default:
							break;
					}
				}
			}
/*		case WM_KEYDOWN:
			theApp.m_pMainLW->OnKeyDown((int)wParam);
			return 0;
		case WM_LBUTTONDBLCLK:
			theApp.m_pMainLW->OnDclick();
			return 0;*/
	}
	return DefWindowProc(hwnd,uMsg, wParam, lParam);
}
int gnPos = 0;


VOID CALLBACK CTagsTimerProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	gnPos+=2;
	gnPos %= 100;
	SendMessage(hwnd, PBM_SETPOS, gnPos, 0);

	DWORD dwRet;
	GetExitCodeProcess(theApp.m_CTagsWnd.m_shEx.hProcess, &dwRet);
	
	if (dwRet != STILL_ACTIVE) {
		static TCHAR szTagsFile[_MAX_PATH], szTagsNewFile[_MAX_PATH];
		_stprintf(szTagsFile, _T("%hS\\%s"), theApp.m_CTagsWnd.m_szDirectory, _T("tags"));
		_stprintf(szTagsNewFile, _T("%hS\\%s"), theApp.m_CTagsWnd.m_szDirectory, _T("tags.new"));
		SendMessage(theApp.m_CTagsWnd.m_hWnd, WM_CLOSE, 0, 0);
		memset(&theApp.m_CTagsWnd.m_shEx, 0, sizeof(SHELLEXECUTEINFO));
	
		if (theApp.m_CTagsWnd.m_ctState == isRunning) {
			theApp.m_CTagsWnd.m_ctState = isFinished;
			DeleteFile(szTagsFile);
			MoveFile(szTagsNewFile, szTagsFile);
		} else {
			DeleteFile(szTagsNewFile);
		} 
	}
}

octCWndCTags::octCWndCTags()
{
	m_hWnd = m_hWndBt = m_hWndPB = NULL;

	m_ctState = isNotStarted;
}

octCWndCTags::~octCWndCTags(void) {
}

HWND octCWndCTags::Create(const char *szDir) {

	if (m_ctState == isRunning) {
		MessageBox(g_hSciCurrHandle, _T("CTags.exe is still running, please wait"), _T("OpenCTags"), MB_OK);
		return NULL;
	}
	strcpy(m_szDirectory, szDir);

	WNDCLASS wndclass;
  wndclass.style          = CS_HREDRAW|CS_VREDRAW;
  wndclass.lpfnWndProc    = CTagsWndProc;
  wndclass.cbClsExtra     = 0;
  wndclass.cbWndExtra     = 0;
  wndclass.hInstance      = ghDllInstance;
  wndclass.hIcon          = NULL;
  wndclass.hCursor        = LoadCursor (NULL, IDC_ARROW);
  wndclass.hbrBackground  = (HBRUSH)GetStockObject (WHITE_BRUSH);
  wndclass.lpszMenuName   = NULL;
  wndclass.lpszClassName  = TEXT(CLASS_CTAGSWND);

	RegisterClass(&wndclass);

	RECT rc, rcCl;
	GetWindowRect(g_hSciCurrHandle, &rc);
	GetClientRect(g_hSciCurrHandle, &rcCl);
	int nWidth = rcCl.right - rcCl.left;
	if (nWidth > 500) nWidth = 500;
/*	pt.x = (LONG)::SendMessage(g_hSciCurrHandle, SCI_POINTXFROMPOSITION, 0, (LPARAM)currPos);
	pt.y = (LONG)::SendMessage(g_hSciCurrHandle, SCI_POINTYFROMPOSITION, 0, (LPARAM)currPos);*/

	return CreateWindow(
			wndclass.lpszClassName, 
			_T("CTags For OpenCTagsNpp"), 
			WS_POPUP | WS_VISIBLE,
			rc.left + rcCl.right - nWidth,
			rc.top + rcCl.bottom-rcCl.top-18,
			nWidth,
			18,
			g_hSciCurrHandle,
			NULL,
			ghDllInstance,
			NULL
	);


}
int octCWndCTags::OnCreateWnd(HWND hWnd)
{
	m_hWnd = hWnd;

	static TCHAR szPath[_MAX_PATH], szDirectory[_MAX_PATH], szFileTags[_MAX_PATH], szFileTagsNew[_MAX_PATH];
	GetModuleFileName(ghDllInstance, szPath, _MAX_PATH-1);
	size_t nPathSize = _tcslen(szPath);
	for (size_t i = nPathSize - 1 ; i>=0; i--) {
		if (szPath[i]=='\\' || szPath[i]=='/') {
			szPath[i+1] = '\0';
			break;
		}
	}
	_tcscat(szPath, _T("ctags.exe"));

	memset(&m_shEx, 0, sizeof(m_shEx));
	m_shEx.cbSize = sizeof(m_shEx);
	m_shEx.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
	m_shEx.hwnd = m_hWnd;
	m_shEx.lpVerb = _T("open");
	m_shEx.lpFile = szPath;
	m_shEx.lpParameters = _T("-R -f tags.new --extra=+q *");
	mbstowcs(szDirectory, m_szDirectory, _MAX_PATH-1);
	m_shEx.lpDirectory = szDirectory;
	m_shEx.nShow = SW_HIDE;
	if (!ShellExecuteEx(&m_shEx)) {
		static TCHAR lszMessage[2048];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, lszMessage, 1024, NULL);
		MessageBox(g_hSciCurrHandle, lszMessage, _T("CTags Execution"), MB_OK | MB_ICONEXCLAMATION);
		SendMessage(m_hWnd, WM_CLOSE, 0, 0);
		m_ctState = isError;
		return 0;
	}
	m_ctState = isRunning;
	
	_stprintf(szFileTags, _T("%hS\\%s"), szDirectory, _T("tags"));
	_stprintf(szFileTagsNew, _T("%hS\\%s"), szDirectory, _T("tags.new"));
	CopyFile(szFileTags, szFileTagsNew, FALSE);
	
	RECT rc;
	GetClientRect(hWnd, &rc);
	InitCommonControls();
	m_hWndPB = CreateWindowEx(0, PROGRESS_CLASS, _T("Progress Bar"), 
			WS_CHILD|WS_VISIBLE, 0, 0, 
			rc.right - rc.left - 50, rc.bottom - rc.top, m_hWnd, 
			NULL, ghDllInstance, NULL);
	
	m_hWndBt = CreateWindowEx(0, _T("BUTTON"), _T("Cancel"), 
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT, rc.right - 50, 0, 
			50, rc.bottom - rc.top, m_hWnd, 
			NULL, ghDllInstance, NULL);

/*	HFONT hFont = (HFONT)GetStockObject(SYSTEM_FONT);//(HFONT)SendMessage(hWndBtn, WM_GETFONT, 0, 0);
	LOGFONT lgFont;
	GetObject((HFONT)hFont, sizeof(lgFont), &lgFont);
	lgFont.lfWeight = FW_NORMAL;
	HFONT hFont2 = CreateFontIndirect(&lgFont);
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont2, 0);*/
//	Shell
	SetTimer(m_hWndPB, 100, 20, CTagsTimerProc);
	return 1;
}
