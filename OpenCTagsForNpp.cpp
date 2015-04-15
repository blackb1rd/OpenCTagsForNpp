//this file is part of notepad++
//Copyright (C)2003 Don HO ( donho@altern.org )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include "wxprec.h"
#include "PluginInterface.h"
#include "Scintilla.h"
#include "OpenCTagsApp.h"

// This is the name which will be displayed in Plugins Menu
const TCHAR PLUGIN_NAME[] = _T("OpenCTags");

HINSTANCE ghDllInstance=NULL;
static char gs_currWord[512];

NppData nppData;
HWND g_hSciCurrHandle;
// The number of functionality
const int nbFunc = 4;
// Declaration of functionality (FuncItem) Array
FuncItem funcItem[nbFunc];

static ShortcutKey shctOCT[] = {
	{false, true, false, VK_SPACE},
	{false, true, false, 'N'},
	{false, true, false, 'P'},
	{false, true, false, 'C'}
};
// Here're the declaration of functions for 3 functionalitis
void opentag();
void nexttag();
void prevtag();
void lancectags();

BOOL APIENTRY DllMain( HINSTANCE hModule,
                       DWORD  reasonForCall,
                       LPVOID lpReserved)
{
	ghDllInstance = hModule;
    switch (reasonForCall)
    {
			// Here we initialize the FuncItem Array.
			// AWARE : no need to initialize _cmdID field.
			// This filed will be initialized by Notepad++ plugins system
      case DLL_PROCESS_ATTACH:
				funcItem[0]._pFunc = opentag;
				funcItem[1]._pFunc = nexttag;
				funcItem[2]._pFunc = prevtag;
				funcItem[3]._pFunc = lancectags;

				_tcscpy(funcItem[0]._itemName, _T("Open Tag"));
				_tcscpy(funcItem[1]._itemName, _T("Next Tag"));
				_tcscpy(funcItem[2]._itemName, _T("Prev Tag"));
				_tcscpy(funcItem[3]._itemName, _T("CTags"));

				for (int i = 0; i < nbFunc; i++) {
				// if you want your menu item is checked while the launch of program
				// set it as true, otherwise set it as false
					funcItem[i]._init2Check = false;
					funcItem[i]._pShKey = &shctOCT[i];
				}

        break;

      case DLL_PROCESS_DETACH:
        break;

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}

/*
 *--------------------------------------------------
 * The 4 extern functions are mandatory
 * They will be called by Notepad++ plugins system
 *--------------------------------------------------
*/

// The setInfo function gets the needed infos from Notepad++ plugins system
extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
}

// The getName function tells Notepad++ plugins system its name
extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return PLUGIN_NAME;
}

// The getFuncsArray function gives Notepad++ plugins system the pointer FuncItem Array
// and the size of this array (the number of functions)
extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return funcItem;
}

// If you don't need get the notification from Notepad++,
// just let it be empty.
extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	static TCHAR szMessage[512];
	if (!(theApp.m_pMainLW && IsWindow(theApp.m_pMainLW->m_hWnd))) return;
	switch (notifyCode->nmhdr.code) {
		case SCN_MODIFIED:
			if ((notifyCode->modificationType & SC_MOD_INSERTTEXT
					|| notifyCode->modificationType & SC_MOD_DELETETEXT)
					&& notifyCode->linesAdded == 0
					&& !theApp.m_pMainLW->m_bIsClosing) {

						_stprintf(szMessage,
											_T("SCN_MODIFIED avec mode = %x,length [%d], linesAdded [%d]\n"),
											notifyCode->modificationType,
											notifyCode->length, notifyCode->linesAdded);
						OutputDebugString(szMessage);

						theApp.m_pMainLW->SetListFromTag(theApp.m_pCurFIndex, getCurrentWord());
			}
		break;
	}
}

// Here you can process the Npp Messages
// I will make the messages accessible little by little, according to the need of plugin development.
// Please let me know if you need to access to some messages :
// http://sourceforge.net/forum/forum.php?forum_id=482781
//
extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{/*
	if (Message == WM_MOVE)
	{
		::MessageBox(NULL, "move", "", MB_OK);
	}
*/
	return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode() {
	return TRUE;
}
#endif //UNICODE


HWND getCurrentHScintilla(int which)
{
	return (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;
};

//----------------------------------------------------------//
// Here're the definition of functions for 3 functionalities
//----------------------------------------------------------//

void opentag()
{
	int currentEdit;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
	g_hSciCurrHandle = getCurrentHScintilla(currentEdit);


	POINT pt;
	RECT rctScWnd, rctDesktop;

	int currPos = (int)::SendMessage(g_hSciCurrHandle, SCI_GETCURRENTPOS, 0, 0);

	theApp.m_nLineNumber = (int)::SendMessage(g_hSciCurrHandle, SCI_LINEFROMPOSITION, (WPARAM)currPos, 0);
	int nTextHeight = (int)::SendMessage(g_hSciCurrHandle, SCI_TEXTHEIGHT, (WPARAM)theApp.m_nLineNumber,0);

	GetWindowRect(g_hSciCurrHandle,&rctScWnd);
	GetWindowRect(GetDesktopWindow(), &rctDesktop);
	pt.x = (LONG)::SendMessage(g_hSciCurrHandle, SCI_POINTXFROMPOSITION, 0, (LPARAM)currPos);
	pt.y = (LONG)::SendMessage(g_hSciCurrHandle, SCI_POINTYFROMPOSITION, 0, (LPARAM)currPos);

	pt.x+=rctScWnd.left - OPENCTAGS_POSX; pt.y+=rctScWnd.top;
	pt.y+=nTextHeight;

	if (pt.x < 0) pt.x = 0;
	else if (pt.x > rctDesktop.right - OPENCTAGS_WIDTH) pt.x = rctDesktop.right - OPENCTAGS_WIDTH;
	if (pt.y + OPENCTAGS_WIDTH > rctDesktop.bottom) pt.y -= OPENCTAGS_HEIGHT + nTextHeight;

	theApp.OnInit(getCurrentWord(), getCurrentPath(), pt);
//	::MessageBox(nppData._nppHandle, "Open Tag", "", MB_OK);
}

void nexttag()
{
	theApp.nextTag();
}

void prevtag()
{
	int currPos = (int)::SendMessage(g_hSciCurrHandle, SCI_GETCURRENTPOS, 0, 0);
	theApp.m_nLineNumber = (int)::SendMessage(g_hSciCurrHandle, SCI_LINEFROMPOSITION, (WPARAM)currPos, 0);

	theApp.prevTag();
}

void lancectags() {
	int currentEdit;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
	g_hSciCurrHandle = getCurrentHScintilla(currentEdit);
	theApp.lancectags();
}

char * getCurrentWord() {
	int wordStart=0, wordEnd=0, currPos = 0;
	currPos = (int)::SendMessage(g_hSciCurrHandle, SCI_GETCURRENTPOS, 0, 0);
	wordStart = (int)::SendMessage(g_hSciCurrHandle, SCI_WORDSTARTPOSITION, (WPARAM)currPos, true);
	wordEnd = (int)::SendMessage(g_hSciCurrHandle, SCI_WORDENDPOSITION, (WPARAM)currPos, true);

	struct TextRange tr = {{wordStart, wordEnd}, gs_currWord};
	::SendMessage(g_hSciCurrHandle, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
	return gs_currWord;
}

int replaceCurrentWord(char *text) {
	int wordStart=0, wordEnd=0, currPos = 0;
	currPos = (int)::SendMessage(g_hSciCurrHandle, SCI_GETCURRENTPOS, 0, 0);
	wordStart = (int)::SendMessage(g_hSciCurrHandle, SCI_WORDSTARTPOSITION, (WPARAM)currPos, true);
	wordEnd = (int)::SendMessage(g_hSciCurrHandle, SCI_WORDENDPOSITION, (WPARAM)currPos, true);

	::SendMessage(g_hSciCurrHandle, SCI_SETTARGETSTART, (WPARAM)wordStart, 0);
	::SendMessage(g_hSciCurrHandle, SCI_SETTARGETEND, (WPARAM)wordEnd, 0);

	::SendMessage(g_hSciCurrHandle, SCI_REPLACETARGET, (WPARAM)-1, (LPARAM)text);
	wordEnd = wordStart+(int)strlen(text);
	::SendMessage(g_hSciCurrHandle, SCI_SETSEL, (WPARAM)wordEnd, (LPARAM)wordEnd);
	return 1;
}

char * getCurrentPath () {
	static TCHAR szPath[_MAX_PATH];
	static char _szPath[_MAX_PATH];
	::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, 0, (LPARAM) szPath);
	wcstombs(_szPath, szPath, _MAX_PATH - 1);
	return _szPath;
}

int GoToFileLine(const char *pFilePath, int nLineNumber) {
	TCHAR szPath[_MAX_PATH];
	mbstowcs(szPath, pFilePath, _MAX_PATH - 1);
	if (::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)szPath)) {
	} else {
		TCHAR szMessage[_MAX_PATH*2];
		_stprintf(szMessage, _T("Can't open file %s"), szPath);
		MessageBox(g_hSciCurrHandle, szMessage, _T("OpenCTags"),MB_OK | MB_ICONERROR);
		return 0;
	}
	::SendMessage(g_hSciCurrHandle, SCI_GOTOLINE, (WPARAM)nLineNumber, 0);
//		int nVisibleLines = ::SendMessage(g_hSciCurrHandle, SCI_LINESONSCREEN, 0, 0);
	int nFirstVisible = (int)::SendMessage(g_hSciCurrHandle, SCI_GETFIRSTVISIBLELINE, 0, 0);
	if (nLineNumber > 5)
		::SendMessage(g_hSciCurrHandle, SCI_LINESCROLL, 0, nLineNumber - nFirstVisible - 5);
	return 1;
}
