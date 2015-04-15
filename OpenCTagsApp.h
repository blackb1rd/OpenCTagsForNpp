/////////////////////////////////////////////////////////////////////////////
// Name:        OpenCTagsApp.h
// Purpose:
// Author:
// Modified by:
// Created:
// RCS-ID:
// Copyright:   (C)2005
// Licence:     wxWindows
// Generated with wxWinWizard by KoanSoftware.com
/////////////////////////////////////////////////////////////////////////////

#ifndef OpenCTagsApp_H
#define OpenCTagsApp_H

#include "OpenCTagsFrm.h"
#include "octCWndCTags.h"

extern HINSTANCE ghDllInstance;

char *getCurrentWord();
int replaceCurrentWord(char *text);
char * getCurrentPath();
int GoToFileLine(const char *pFilePath, int nLineNumber);

#define OPENCTAGS_WIDTH 500
#define OPENCTAGS_HEIGHT 300
#define OPENCTAGS_POSX 30

// Define a new application type, each program should derive a class from wxApp
class OpenCTagsMain
{
public:
    OpenCTagsMain();
		~OpenCTagsMain() {};

//My Treatements
	octCFileIndex * SetCurFIndex(const char *szPath);
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
  bool OnInit(char *szTag, char *szCurPath, POINT pt);
	int OnExit();

	octListCtrl *m_pMainLW;
	octCWndCTags m_CTagsWnd;
	octCFileIndex *m_pCurFIndex;
	char m_szTag[256];
	char m_szCurPath[_MAX_PATH];
	char m_szLogFile[_MAX_PATH];
	int  m_nLineNumber;
private:
	octSLstFileIndex *m_pLstFileIndex;

//fonctions
	octCFileIndex * FindFIndexForFile(const char *szFile);
	int	AddAFIndex(octCFileIndex *pFIndex);
	int	DelAFIndex(octCFileIndex *pFIndex);
	int SetLogFile(char *szExePath);
	char *GetTagsFilePath(const char *szCutPath);
	bool m_bDebug;
public:
	int WriteLogMessage(const char * szFormat, ...);
	void OpenFileWithParams(char * params);
	int insertCurrentCall(void);
	int prevTag(void);
	int nextTag(void);
	int lancectags(void);
};

extern OpenCTagsMain theApp;

//extern "C" void nppctags(void *);

#endif // OpenCTagsApp_H


