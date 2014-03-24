// octCFileIndex.h: interface for the octCFileIndex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OCTCFILEINDEX_H__BAF299BB_8201_473B_962B_A4F3C36C6AB3__INCLUDED_)
#define AFX_OCTCFILEINDEX_H__BAF299BB_8201_473B_962B_A4F3C36C6AB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define OCT_BLOCK_SIZE 256

#include "wxprec.h"

struct octSFileIndex {
	char *pline;
	char *pfilename;
	char *ptag;
	char *padress;
	char *pextra;
	char kind;
	int  nIndex;
};

struct SOneCall {
	char pFileName[_MAX_PATH];
	int   nLineNumber;
	struct SOneCall *pNextCall;
	struct SOneCall *pPrevCall;
};

class octCCalls {
public:
	octCCalls();
	~octCCalls();
	
	struct SOneCall * m_lstCalls;
	struct SOneCall * m_pCurrCall;

	int insertCall(const char *pFileName, const int nLineNumber);
	SOneCall * getPrevCall();
	SOneCall * getNextCall();
};

class octCFileIndex  
{
public:
	octCFileIndex();
	~octCFileIndex();

	int createFromFile(char *szFileName);
	int createFromText(char * pTexte = NULL);
	int addLine(char *&pLine);
	int tabFound(char *&pNextChar);
	
	struct octSFileIndex * findExactTag(const char *szTag);
	struct octSFileIndex * findFirstTag(const char *szTag);
	struct octSFileIndex * findNextTag();
	struct octSFileIndex ** findAllForTag(const char *szTag, int &nbFound);
	
	char m_szFileName[_MAX_PATH];
	int  m_nLines;
	int  m_nBlocks;
	struct octSFileIndex *m_pFileLines;
	char *m_pFileContent;
	time_t m_tmModified;
	octCCalls m_lstCalls;
private:
	char m_szTagToSearch[256];
	struct octSFileIndex *m_pCurrFound;
	int  m_nFounds;
};

int GetFileLineNumber(octSFileIndex *pSIndex, char *retFileName);

#endif // !defined(AFX_OCTCFILEINDEX_H__BAF299BB_8201_473B_962B_A4F3C36C6AB3__INCLUDED_)
