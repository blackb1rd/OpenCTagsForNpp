// octCFileIndex.cpp: implementation of the octCFileIndex class.
//
//////////////////////////////////////////////////////////////////////

#include "octCFileIndex.h"
#include "OpenCTagsApp.h"

#include <sys/stat.h>


int sIndCompare( const void *arg1, const void *arg2 ) {
	struct octSFileIndex *pS1 = (struct octSFileIndex *)arg1, *pS2 = (struct octSFileIndex *)arg2;
	if (!pS1->ptag) return -1;
	if (!pS2->ptag) return 1;
	return strcmp(pS1->ptag, pS2->ptag);
}

int sIndCompareFirst( const void *arg1, const void *arg2 ) {
	struct octSFileIndex *pS1 = (struct octSFileIndex *)arg1, *pS2 = (struct octSFileIndex *)arg2;
	if (!pS1->ptag) return -1;
	if (!pS2->ptag) return 1;
	return strncmp(pS1->ptag, pS2->ptag, strlen(pS1->ptag));
}

int strreplace(char *AChaine, const char *AOld, const char *ARepl)
{
	size_t nOldLen, nReplLen, nSourceLen, nRetourLen;
	int nCount;
	char *pStart, *pTarget, *pTargetRet;
	char *LSource;

	nOldLen = strlen(AOld);
	if (nOldLen == 0) {
		return (0);
	}
	nReplLen = strlen(ARepl);
	nSourceLen = strlen(AChaine);

	nCount = 0;
	pStart = AChaine;

	while ((pTarget = strstr(pStart, AOld)) != NULL)
	{
		nCount++;
		pStart = pTarget + nOldLen;
	}
		
	if (nCount <= 0) {
		return (nCount);
	}
	
	LSource = (char *)malloc((nSourceLen + 1)*sizeof(char));
	strcpy(LSource, AChaine);

	nRetourLen =  nSourceLen + (nReplLen-nOldLen)*nCount;
	memset(AChaine, 0, nRetourLen + 1);
	
	pStart = LSource;
	pTargetRet = AChaine;

	while ((pTarget = strstr(pStart, AOld)) != NULL) {
		memcpy(pTargetRet, pStart, pTarget - pStart);
		pTargetRet+=pTarget - pStart;
		memcpy(pTargetRet, ARepl, nReplLen);
		pTargetRet += nReplLen;

		pStart = pTarget + nOldLen;
	}

	memcpy(pTargetRet, pStart, nSourceLen - (pStart - LSource));

	free(LSource);
	return (nCount);

}

//////////////////////////////////////////////////////////////////////
// Class octCCalls
//////////////////////////////////////////////////////////////////////
octCCalls::octCCalls() {
	m_lstCalls = NULL; 
	m_pCurrCall = NULL;
}
octCCalls::~octCCalls() {
	SOneCall *pTempCall = NULL, *pNextCall = NULL;
	for (pTempCall=m_lstCalls; pTempCall; pTempCall = pNextCall) {
		pNextCall = pTempCall->pNextCall;
		free(pTempCall);
	}
	m_pCurrCall = NULL;
	m_lstCalls = NULL;
}

int octCCalls::insertCall(const char *pFileName, const int nLineNumber) {
	if (m_pCurrCall &&
			stricmp(pFileName, m_pCurrCall->pFileName) == 0 && 
			nLineNumber == m_pCurrCall->nLineNumber)
		return 0;
	
	SOneCall *pNewCall = new SOneCall;
	strcpy(pNewCall->pFileName, pFileName);
	pNewCall->nLineNumber = nLineNumber;
	pNewCall->pNextCall = NULL;
	pNewCall->pPrevCall = NULL;
	if (m_lstCalls) {
		pNewCall->pNextCall = m_pCurrCall->pNextCall;
		pNewCall->pPrevCall = m_pCurrCall;
		m_pCurrCall->pNextCall = pNewCall;
	} else {
		m_lstCalls = pNewCall;
	}
	m_pCurrCall = pNewCall;
	return 1;
}

struct SOneCall * octCCalls::getNextCall() {
	if (!m_pCurrCall) return NULL;
	if (!m_pCurrCall->pNextCall) return NULL;
	m_pCurrCall = m_pCurrCall->pNextCall;
	return m_pCurrCall;
}

struct SOneCall * octCCalls::getPrevCall() {
	if (!m_pCurrCall) return NULL;
	if (!m_pCurrCall->pPrevCall) return NULL;
	m_pCurrCall = m_pCurrCall->pPrevCall;
	return m_pCurrCall;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


octCFileIndex::octCFileIndex()
{
	m_nLines = 0;
	m_nBlocks = 0;
	strcpy(m_szFileName, "");
	m_pFileContent = NULL;
	m_pFileLines = NULL;
}

octCFileIndex::~octCFileIndex()
{
	if (m_pFileLines) free (m_pFileLines);
	m_pFileLines = NULL;

	if (m_pFileContent) free( m_pFileContent);
	m_pFileContent = NULL;
}

int octCFileIndex::createFromFile(char *szFileName)
{
	m_nLines = 0;
	m_nBlocks = 0;
	FILE *pFile = fopen(szFileName, "rb");
	if (!pFile) {
		return 0;
	}

	fseek(pFile, 0, SEEK_END);
	long lFin = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	m_pFileContent = (char *)malloc((1+lFin)*sizeof(char));
	if (!m_pFileContent) {
		MessageBox(NULL, _T("Out of memory while reading file"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	fread(m_pFileContent, 1, lFin, pFile);

	fclose(pFile);
	strcpy(m_szFileName, szFileName);
//Write down the modification time of the file
	{
		int result = 0;
		struct stat ffstat;
		if ((result = stat(szFileName, &ffstat)) == 0) {
			m_tmModified = ffstat.st_mtime;
		} else {
			m_tmModified = 0;
		}
	}
	return createFromText();
}

int octCFileIndex::createFromText(char * pTexte)
{
	if (pTexte) m_pFileContent = strdup(pTexte);

	char *pFound = m_pFileContent;
	addLine(pFound);
	while ((pFound = strpbrk(pFound, "\x00A\x009")) != NULL) {
		switch (*pFound) {
			case 0x0A :
				addLine(pFound);
				break;
			case 0x09:
				tabFound(pFound);
				break;
			default:
				pFound++;
				break;
		}
	}
//	MessageBox(NULL, wxString::Format("Found %d lines in %s", m_nLines, m_szFileName), "", MB_OK);
	return (1);
}

int octCFileIndex::addLine(char *&pLine)
{
	while (*pLine == 0x0A || *pLine == 0x0D) {*pLine='\0'; pLine++;}
	if (!*pLine) return (0);
	m_nLines++;
	if (m_nLines > m_nBlocks * OCT_BLOCK_SIZE) {
		m_nBlocks++;
		m_pFileLines = (octSFileIndex *)realloc(m_pFileLines, m_nBlocks * OCT_BLOCK_SIZE * sizeof(struct octSFileIndex));
	}
	memset(&m_pFileLines[m_nLines - 1], 0, sizeof (struct octSFileIndex));
	m_pFileLines[m_nLines - 1].pline = pLine;
	m_pFileLines[m_nLines - 1].ptag = pLine;
	m_pFileLines[m_nLines - 1].nIndex = m_nLines - 1;
	return (1);
}

int octCFileIndex::tabFound(char *&pNextChar) {
	struct octSFileIndex *pCurIndex = &m_pFileLines[m_nLines - 1];
	if (pCurIndex->pextra) {
		pNextChar++;
		return 1;
	}
//It is possible that adress has a \t
	char *pFound = NULL;
	if (pCurIndex->padress) {
		int nDelimsFounds = 0;
		for (pFound = pCurIndex->padress;(pFound = strchr(pFound, '/')) != NULL && pFound < pNextChar; pFound++) {
			if (pFound > pCurIndex->padress && *(pFound-1)=='\\') continue;
			nDelimsFounds++;
		}
		if ((nDelimsFounds % 2) != 0) {
			pNextChar++;
			return 1;
		}
	}
	*pNextChar='\0';
	pNextChar++;
	if (!pCurIndex->pfilename) {
		pCurIndex->pfilename = pNextChar;
	} else if (!pCurIndex->padress ) {
		pCurIndex->padress = pNextChar;
	} else {
		pCurIndex->pextra = pNextChar;
		if (!pCurIndex->kind) {
			pFound = strpbrk(pNextChar, "\x00A\x009:");
			if (pFound) {
				if (*pFound == ':' && strncmp(pNextChar, "kind", 4)==0)
					pCurIndex->kind = *(pFound+1);
				else
					pCurIndex->kind = *pNextChar;
			}
		}
	}
	return (1);
}


struct octSFileIndex * octCFileIndex::findExactTag(const char *szTag)
{
	if (!m_pFileLines || !m_nLines) return NULL;
	struct octSFileIndex *pFound = NULL, sToSearch;
	memset(&sToSearch, 0, sizeof(struct octSFileIndex));
	sToSearch.ptag = (char *)szTag;
	pFound = (struct octSFileIndex *)bsearch(&sToSearch, m_pFileLines, m_nLines, sizeof(struct octSFileIndex), sIndCompare);

	return (pFound);
}

struct octSFileIndex * octCFileIndex::findFirstTag(const char *szTag) {
	m_nFounds = 0;
	if (!m_pFileLines || !m_nLines) return NULL;
	struct octSFileIndex *pFound = NULL, sToSearch;
	memset(&sToSearch, 0, sizeof(struct octSFileIndex));
	sToSearch.ptag = (char *)szTag;

	pFound = (struct octSFileIndex *)bsearch(&sToSearch, m_pFileLines, m_nLines, sizeof(struct octSFileIndex), sIndCompareFirst);
	if (!pFound) {
		strcpy(m_szTagToSearch, "");
		m_pCurrFound = NULL;
		return NULL;
	}
	strcpy(m_szTagToSearch, szTag);
//I'll search the first
	for (int i = pFound->nIndex - 1; i >= 0 && strncmp(szTag, m_pFileLines[i].ptag, strlen(szTag)) ==0; i--) {
		pFound = &m_pFileLines[i];
	}
	m_pCurrFound = pFound;
	return pFound;
}

struct octSFileIndex * octCFileIndex::findNextTag() {
	if (!m_pCurrFound || !*m_szTagToSearch) return NULL;
	if (m_pCurrFound->nIndex + 1 < m_nLines &&
			strncmp(m_szTagToSearch, m_pFileLines[m_pCurrFound->nIndex + 1].ptag, strlen(m_szTagToSearch)) == 0) {
		m_pCurrFound = &m_pFileLines[m_pCurrFound->nIndex + 1];
		m_nFounds++;
		return m_pCurrFound;
	}
	strcpy(m_szTagToSearch, "");
	m_pCurrFound = NULL;
	return NULL;
}

struct octSFileIndex ** octCFileIndex::findAllForTag(const char *szTag, int &nbFound){
	octSFileIndex **lstSIndex = NULL;
	nbFound = 0;
	if (*szTag == '\0') return NULL;
	for (octSFileIndex *pSIndex = findFirstTag(szTag); pSIndex; pSIndex = findNextTag()) {
		nbFound++;
		lstSIndex = (octSFileIndex **)realloc(lstSIndex, nbFound*sizeof(octSFileIndex *));
		lstSIndex[nbFound - 1] = pSIndex;
	}
	return lstSIndex;
}

struct octSAdresses {
	char *padress;
	char tadress; //n - line number; r - regular expression; c - comments
};
//External Function 
int GetFileLineNumber(octSFileIndex *pSIndex, char * retFileName) {
	struct octSAdresses lstAdresses[20];
	int  nAdresses = 0;

	char padress[1024];
	strcpy(padress,pSIndex->padress);
	size_t nSzLen = strlen(padress);
	char *pFound = padress;

	char chRegExp = '\0';
	while (1) {
		if (*pFound=='/' || *pFound == '?') {
			chRegExp = *pFound;
			pFound++;
			lstAdresses[nAdresses].tadress = 'r';
		} else if (*pFound == '"'){
			pFound++;
			lstAdresses[nAdresses].tadress = 'c';
		} else {
			lstAdresses[nAdresses].tadress = 'n';
		}
		lstAdresses[nAdresses++].padress = pFound;
		for (;(pFound = strchr(pFound, ';')) != NULL; pFound++) {
			if (!chRegExp || *(pFound - 1) == chRegExp) break;
		}
		if (!pFound) {
			pFound = padress + nSzLen - 1;
			if (chRegExp && *pFound == chRegExp) *pFound = '\0';
			break;
		}
		if (chRegExp && *(pFound - 1) == chRegExp) *(pFound - 1) = '\0';
		pFound++;
	}

	char szFileName[MAX_PATH];
	strcpy(szFileName, theApp.m_pCurFIndex->m_szFileName);
	nSzLen = strlen(szFileName);
	for (pFound = szFileName + nSzLen ; pFound > szFileName; pFound--) {
		if (*pFound == '\\' || *pFound == '/') {
			*(pFound+1) = '\0';
			break;
		}
	}
	strcat(szFileName, pSIndex->pfilename);

	FILE *pFile = NULL;
	pFile = fopen(szFileName, "rt");
	int cLineNumber = -1;

	char szAdress[1024];
	char lineText[1024];
	int nLine = 0;
	size_t nLen = 0;
	for (int i = 0; i < nAdresses; i++) {
		if (lstAdresses[i].tadress == 'r') {
			int bFromStart = 0;
			if (lstAdresses[i].padress[0]=='^') {
				bFromStart = 1;
				strcpy(szAdress, lstAdresses[i].padress+1);
				nLen = strlen(szAdress);
				if (!nLen) continue;
				if (szAdress[nLen-1]=='$')
					szAdress[nLen-1]='\0';
			} else {
				strcpy(szAdress, lstAdresses[i].padress);
			}
			strreplace(szAdress, "\\/", "/");	
			//strreplace(szAdress, "(", "\\(");strreplace(szAdress, ")", "\\)");
			for (;fgets(lineText, 1024, pFile); nLine++) {
//Delete \r \n caracteres at the end of the line
				for (nLen = strlen(lineText);nLen > 0 && strchr("\r\n", lineText[nLen-1]);nLen--) {
					lineText[nLen-1]='\0';
				}
				
/*				match_results::backref_type br = pat.match( lineText, results );
				if (br.matched) {*/
				if ((bFromStart && strcmp(szAdress, lineText)==0) ||
					(!bFromStart && strstr(lineText, szAdress))) {
					cLineNumber = nLine;
					break;
				}

			}
		} else if (cLineNumber == -1 && lstAdresses[i].tadress == 'n') {
			cLineNumber = atoi(lstAdresses[i].padress);
			for (int j = 0; j < cLineNumber && fgets(lineText, 1024, pFile); j++);
		}
	}
	if (pFile) fclose(pFile);
	strcpy(retFileName, szFileName);
	return cLineNumber;
}
