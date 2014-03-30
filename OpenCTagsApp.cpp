/////////////////////////////////////////////////////////////////////////////
// Name:        OpenCTagsMain.cpp
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
#include "resource.h"

#include <direct.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <process.h>
#include <commdlg.h>

#include "PluginInterface.h"
#include ".\openctagsapp.h"
//Constants
#define OCT_WINDOWNAME "OpenCTags by RJ : version 1.0"

OpenCTagsMain theApp;

//struct SCtagsState GCTagsState;


OpenCTagsMain::OpenCTagsMain()
{
	m_pLstFileIndex = NULL;
	m_pCurFIndex = NULL;

	m_szLogFile[0] = '\0';
	m_bDebug = 0;

//	memset(&GCTagsState, 0, sizeof(struct SCtagsState));
}

// 'Main program' equivalent: the program execution "starts" here
bool OpenCTagsMain::OnInit(char *szTag, char *szCurPath, POINT pt)
{
	strcpy(m_szTag, szTag);
	strcpy(m_szCurPath, szCurPath);

	if (!SetCurFIndex(szCurPath)) {
		TCHAR szMessage[_MAX_PATH+256];
		_stprintf(szMessage, _T("'tags' file not found on path %hS.\n") 
			_T("Launch the CTags command and choose to save a 'tags' file in a parent directory of the current file path"), szCurPath);
		::MessageBox(nppData._nppHandle, szMessage, _T("OpenCTagsForNpp"), MB_ICONEXCLAMATION);
		return (true);
	}
	m_pMainLW = new octListCtrl;
	m_pMainLW->Create(pt);

	m_pMainLW->SetListFromTag(m_pCurFIndex, szTag);
	ShowWindow(m_pMainLW->m_hWndParent, SW_SHOW);
	UpdateWindow(m_pMainLW->m_hWndParent);
	SetFocus(m_pMainLW->m_hWnd);
    
	return (true);
}

int OpenCTagsMain::OnExit()
{
	if (m_pLstFileIndex) {
		octSLstFileIndex *pToDel = m_pLstFileIndex, *pNext = NULL;
		for (;pToDel;pToDel = pNext) {
			pNext = pToDel->m_pNext;
			delete pToDel;
			pToDel = NULL;
		}
		m_pLstFileIndex = NULL;
	}
	return 0;
}

octCFileIndex * OpenCTagsMain::FindFIndexForFile(const char *szFile)
{
	struct stat ffstat;
	int result = 0;
	octCFileIndex *pFIndex = NULL;
	octSLstFileIndex *pCurLst;
	for (pCurLst = m_pLstFileIndex; pCurLst; pCurLst=pCurLst->m_pNext) {
		if (stricmp(szFile,pCurLst->m_pCFileIndex->m_szFileName) == 0) {
			pFIndex = pCurLst->m_pCFileIndex;
			if ((result = stat(szFile, &ffstat))==0 && ffstat.st_mtime != pCurLst->m_pCFileIndex->m_tmModified) {
				DelAFIndex(pFIndex);
				pFIndex = NULL;
			}
			break;
		}
	}
	return pFIndex;
}


int	OpenCTagsMain::AddAFIndex(octCFileIndex *pFIndex) {
	if (!m_pLstFileIndex) {
		m_pLstFileIndex = new octSLstFileIndex (pFIndex);
		return 1;
	}
	for (octSLstFileIndex *pLstCurr = m_pLstFileIndex; pLstCurr->m_pNext; pLstCurr=pLstCurr->m_pNext);
	pLstCurr->m_pNext = new octSLstFileIndex(pFIndex);
	return 1;
}

int	OpenCTagsMain::DelAFIndex(octCFileIndex *pFIndex) {
	if (!m_pLstFileIndex) {
		return 0;
	}
	octSLstFileIndex *pToDel = NULL, *pPrev = NULL;
	for (octSLstFileIndex *pLstCurr = m_pLstFileIndex; pLstCurr; pPrev = pLstCurr, pLstCurr=pLstCurr->m_pNext) {
		if (pLstCurr->m_pCFileIndex == pFIndex) {
			pToDel = pLstCurr;
			break;
		}
	}
	if (!pToDel) {
		return 0;
	}
	if (pPrev) {
		pPrev->m_pNext = pToDel->m_pNext;
	} else {
		m_pLstFileIndex = pToDel->m_pNext;
	}
	delete pToDel;
	return 1;
}

char *OpenCTagsMain::GetTagsFilePath(const char *szCurPath) {
	static char szTagPath[_MAX_PATH];
	strcpy(szTagPath, szCurPath);
	int nPath = (int)strlen(szCurPath);
	for (char *pChar = szTagPath + nPath; pChar > szTagPath; pChar--) {
		if (*pChar == '\\' || *pChar =='/') {
			*(pChar+1) = '\0';
			strcat(szTagPath, "tags");
			FILE *pFile = fopen(szTagPath, "r");
			if (pFile) {
				fclose(pFile);
				break;
			}
		}
	}
	if (pChar <= szTagPath) {
WriteLogMessage("Could not find tags file in directory [%s] or in a parent directory\n", szCurPath);
		return (NULL);
	}
	return szTagPath;
}

octCFileIndex * OpenCTagsMain::SetCurFIndex(const char *szCurPath)
{
	char *szTagPath;
	szTagPath = GetTagsFilePath(szCurPath);
	if (!szTagPath) return NULL;

	m_pCurFIndex = NULL;
WriteLogMessage("Will use the file [%s]\n", szTagPath);
	m_pCurFIndex = FindFIndexForFile(szTagPath);
	if (!m_pCurFIndex) {
		m_pCurFIndex = new octCFileIndex();
		//MessageBox(NULL, wxString::Format("I'm opening %s", szTagPath), "", MB_OK);
		if (!m_pCurFIndex->createFromFile(szTagPath)) {
			MessageBox (NULL, _T("Error while loading tags file"), _T("OpenCTags"), MB_OK | MB_ICONEXCLAMATION);
			delete m_pCurFIndex;
			m_pCurFIndex = NULL;
			return NULL;
		}
WriteLogMessage("Tag file opened succesfully\n");
		AddAFIndex(m_pCurFIndex);
	} else {
WriteLogMessage("Tag file already opened\n");
	}

	return (m_pCurFIndex);
}


int OpenCTagsMain::SetLogFile(char *szExePath) {
/*	wxFileName fileName(szExePath);
	strcpy(m_szLogFile, fileName.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR));
	strcat(m_szLogFile, "OpenCTags.log");
//Log file truncated if -d flag and mode server
	FILE * fileLog = fopen(m_szLogFile, "w");
	if (fileLog) fclose(fileLog);
*/
	return 1;
}

int OpenCTagsMain::WriteLogMessage(const char *szFormat, ...)
{
	if (!m_bDebug)
		return 0;
	FILE *fileLog = fopen(m_szLogFile, "at");
	if (!fileLog) return 0;
	va_list marker;
	va_start ( marker , szFormat);
	vfprintf(fileLog, szFormat, marker);
	va_end(marker);
	fclose(fileLog);
	return 1;
}


int OpenCTagsMain::insertCurrentCall(void) {
	if (m_pCurFIndex)
		m_pCurFIndex->m_lstCalls.insertCall(m_szCurPath, m_nLineNumber);
	return 1;
}

int OpenCTagsMain::prevTag(void)
{
	char *szPath = getCurrentPath();
	octCFileIndex *pFileIndex = SetCurFIndex(szPath);
	if (!pFileIndex) return 0;
	pFileIndex->m_lstCalls.insertCall(szPath, m_nLineNumber);
	SOneCall *pCall = pFileIndex->m_lstCalls.getPrevCall();
	if (!pCall) return 0;
	GoToFileLine(pCall->pFileName, pCall->nLineNumber);
	return 1;
}

int OpenCTagsMain::nextTag(void)
{
	octCFileIndex *pFileIndex = SetCurFIndex(getCurrentPath());
	if (!pFileIndex) return 0;
	SOneCall *pCall = pFileIndex->m_lstCalls.getNextCall();
	if (!pCall) return 0;
	GoToFileLine(pCall->pFileName, pCall->nLineNumber);
	return 1;
}


int OpenCTagsMain::lancectags(void) {
/*	if (GCTagsState.nState != 0) {
		MessageBox(g_hSciCurrHandle, "CTags is running, please wait !!!", "OpenCTags", MB_OK);
		return 0;
	}*/
	OPENFILENAME ofn;
	static TCHAR szFilePath[_MAX_PATH];
	char * _szFilePath = getCurrentPath();
	mbstowcs(szFilePath, _szFilePath, _MAX_PATH - 1);
	static char szTagsPath[_MAX_PATH];
	char * _szTagsPath = GetTagsFilePath(_szFilePath);

	if (_szTagsPath) {
		strcpy(szTagsPath, _szTagsPath);
	} else {
		static TCHAR szNewTagsFile[_MAX_PATH];
		_tcscpy(szNewTagsFile, _T("tags"));

		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = g_hSciCurrHandle;
		ofn.hInstance = ghDllInstance;
		ofn.lpstrFile = szNewTagsFile;

		ofn.nMaxFile = _MAX_PATH+1;
		ofn.lpstrInitialDir = szFilePath;
		ofn.lpstrTitle = _T("Select the root of your project");
		if (GetSaveFileName(&ofn)) {
			wcstombs(szTagsPath, ofn.lpstrFile, _MAX_PATH-1);
		} else {
			return 0;
		}
	}
	int nLen = (int)strlen(szTagsPath);
	for (int i = nLen-1; i>=0;i--) {
		if (szTagsPath[i]=='\\' ||szTagsPath[i]=='/') {
			szTagsPath[i] = '\0';
			break;
		}
	}
/*	strcpy(GCTagsState.szDiretory, szTagsPath);
	GCTagsState.nState = 1;*/
	m_CTagsWnd.Create(szTagsPath);
//	_beginthread(createWndCTags, 0, &GCTagsState);
//	nppctags(&GCTagsState);
	return (1);
}
