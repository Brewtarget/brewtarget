/*****************************************************************
 *                 Locate NSIS plugin v2.0                       *
 *                                                               *
 * 2011 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)  *
 *****************************************************************/


//Comment-out and recompile
#define LOCATE      //Compile with Locate function
#define GETSIZE     //Compile with GetSize function
#define RMDIR_EMPTY //Compile with RMDirEmpty function



#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "StackFunc.h"
#include "StrFunc.h"
#include "WideFunc.h"


//Include stack functions
#define StackInsertAfter
#define StackInsertBefore
#define StackInsertIndex
#define StackDelete
#define StackJoin
#define StackClear
#include "StackFunc.h"

//Include string functions
#define WideCharLower
#define xmemcpy
#define xstrlenA
#define xstrlenW
#define xstrcmpnW
#define xstrcmpiW
#define xstrcpynA
#define xstrcpynW
#define xatoiW
#define xatoi64W
#define xitoaW
#define xi64toaW
#define xuitoaW
#define dec2hexW
#define xprintfW
#define xstrstrW
#include "StrFunc.h"

//Include wide functions
#define FindFirstFileWide
#define FindNextFileWide
#define SetWindowTextWide
#define RemoveDirectoryWide
#include "WideFunc.h"

//Defines
#define NSIS_MAX_STRLEN    1024
#define IDC_STATIC_DETAILS 1006

#define LO_TIME_WRITE         1
#define LO_TIME_CREATION      2
#define LO_TIME_ACCESS        3

#define LO_ATTR_IGNORE        0
#define LO_ATTR_INCLUDE       1
#define LO_ATTR_EXCLUDE       2

#define LO_SORT_FILES_IGNORE  0
#define LO_SORT_FILES_NAME    1
#define LO_SORT_FILES_TYPE    2
#define LO_SORT_FILES_SIZE    3
#define LO_SORT_FILES_DATE    4

#define LO_SORT_DIRS_IGNORE   0
#define LO_SORT_DIRS_NAME     1
#define LO_SORT_DIRS_DATE     2

#define LO_GOTO_END           0
#define LO_GOTO_BEGIN         1
#define LO_GOTO_FINDFIRST     2
#define LO_GOTO_NEXTDIR       3
#define LO_GOTO_NEXTFILE      4

#define LO_BANNER_IGNORE      0
#define LO_BANNER_DETAILS     1
#define LO_BANNER_CALLBACK    2

#define GETSIZE_FUNC          1
#define RMDIR_EMPTY_FUNC      2

typedef struct _FILEITEM {
  struct _FILEITEM *next;
  struct _FILEITEM *prev;
  DWORD dwAttr;
  FILETIME ft;
  wchar_t wszName[MAX_PATH];
  wchar_t wszExt[MAX_PATH];
  __int64 nFileSize;
} FILEITEM;

typedef struct _DIRITEM {
  struct _DIRITEM *next;
  struct _DIRITEM *prev;
  DWORD dwAttr;
  FILETIME ft;
  wchar_t wszName[MAX_PATH];
} DIRITEM;

typedef struct {
  FILEITEM *first;
  FILEITEM *last;
  int nElements;
} STACKFILE;

typedef struct _HLOCATESTACK {
  STACKFILE hPathsStack;
  STACKFILE hDirsStack;
  STACKFILE hFilesStack;
  wchar_t wszCurPath[NSIS_MAX_STRLEN];
  wchar_t wszWildcard[MAX_PATH];
  wchar_t wszNames[NSIS_MAX_STRLEN];
  wchar_t wszExt[NSIS_MAX_STRLEN];
  wchar_t wszSearchPaths[NSIS_MAX_STRLEN];
  wchar_t wszExcludeFullPaths[NSIS_MAX_STRLEN];
  wchar_t wszExcludePathNames[NSIS_MAX_STRLEN];
  wchar_t *wpCurSearchPath;
  wchar_t *wpNextSearchPath;
  __int64 nSizeMoreThen;
  __int64 nSizeLessThen;
  __int64 nSize;
  FILETIME lpFileTimeAfter;
  FILETIME lpFileTimeBefore;
  int nDivisor;
  HANDLE hSearch;
  int nGoto;
  int nTime;
  int nReadOnly;
  int nArchive;
  int nHidden;
  int nSystem;
  int nSortFiles;
  int nSortDirs;
  int nBanner;
  BOOL bFindFiles;
  BOOL bFindDirs;
  BOOL bFindEmptyDirs;
  BOOL bGotoSubDir;
  BOOL bCompareSize;
  BOOL bCompareTime;
  BOOL bCompareAttr;
  BOOL bExtInclude;
  BOOL bExtExclude;
  BOOL bNamesInclude;
  BOOL bNamesExclude;
  BOOL bFullPathsExclude;
  BOOL bPathNamesExclude;
  BOOL bOutputReverse;
  BOOL bSortFilesReverse;
  BOOL bSortDirsReverse;
} HLOCATESTACK;

//ExDll
typedef struct _stack_t {
  struct _stack_t *next;
  wchar_t text[1];
} stack_t;

typedef struct
{
  int autoclose;
  int all_user_var;
  int exec_error;
  int abort;
  int exec_reboot;
  int reboot_called;
  int XXX_cur_insttype;
  int plugin_api_version;
  int silent;
  int instdir_error;
  int rtl;
  int errlvl;
  int alter_reg_view;
  int status_update;
} exec_flags_t;

typedef struct {
  exec_flags_t *exec_flags;
  int (__stdcall *ExecuteCodeSegment)(int, HWND);
  void (__stdcall *validate_filename)(wchar_t *);
} extra_parameters;

stack_t **g_stacktop;
wchar_t *g_variables;
unsigned int g_stringsize;
extra_parameters *g_pluginParms;
BOOL g_unicode=-1;

//Global variables
wchar_t wszBuf[NSIS_MAX_STRLEN];
wchar_t wszBuf2[NSIS_MAX_STRLEN];
wchar_t wszOptions[NSIS_MAX_STRLEN];
HANDLE hSearchDetails=0;

WIN32_FIND_DATAW wfd;
SYSTEMTIME lpSystemTime;
FILETIME lpLocalFileTime;

int nFunction;
__int64 nDirSizeGS;
int nFileSumGS;
int nDirSumGS;
int nBannerGS;
int nRemoveDirRM;
int nBannerRM;

//Funtions prototypes
FILEITEM* ItemPushNoSort(STACKFILE *hStack, int nBytes);
FILEITEM* ItemPushSortName(STACKFILE *hStack, const wchar_t *wpName, int nUpDown, int nBytes);
FILEITEM* ItemPushSortType(STACKFILE *hStack, const wchar_t *wpName, const wchar_t *wpExt, int nUpDown, int nBytes);
FILEITEM* ItemPushSortSize(STACKFILE *hStack, const wchar_t *wpName, __int64 nSize, int nUpDown, int nBytes);
FILEITEM* ItemPushSortDate(STACKFILE *hStack, const wchar_t *wpName, const FILETIME *ft, int nUpDown, int nBytes);
void ItemDelete(STACKFILE *hStack, void *lpFileItem);
void ItemFree(STACKFILE *hStack);
void ReturnDataFromStack(HLOCATESTACK *hLocateStack, FILEITEM *lpFileItem);
BOOL SearchFiles(const wchar_t *wpPath, const wchar_t *wpWildcard, BOOL subdir);
BOOL IsDirEmpty(const wchar_t *wpDir);
const wchar_t* GetFileExt(const wchar_t *wpFile);
void TrimPathBackslash(wchar_t *wszCurPath);
BOOL IsWord(const wchar_t *wpText, const wchar_t *wpWord);
int GetOptionsW(wchar_t *wpLine, wchar_t *wpOption, BOOL bSensitive, wchar_t *wszResult, int nMaxResult);

int popintegerWide();
void pushintegerWide(int integer);
int popstringAnsi(char *str, int len);
int popstringWide(wchar_t *str, int len);
void pushstringAnsi(const char *str);
void pushstringWide(const wchar_t *str);
void Initialize(int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra);


//Extern functions
#ifdef LOCATE
void __declspec(dllexport) _Open(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    HLOCATESTACK *hLocateStack;
    const wchar_t *wpOption;
    wchar_t *wpStrBegin;
    wchar_t *wpStrEnd;
    WORD wTimeDayAfterThen=1;
    WORD wTimeMonthAfterThen=1;
    WORD wTimeYearAfterThen=1601;
    WORD wTimeDayBeforeThen=1;
    WORD wTimeMonthBeforeThen=1;
    WORD wTimeYearBeforeThen=30827;

    if (!(hLocateStack=(HLOCATESTACK *)GlobalAlloc(GPTR, sizeof(HLOCATESTACK))))
      goto Error;
    hLocateStack->wpCurSearchPath=&hLocateStack->wszSearchPaths[0];
    hLocateStack->nSizeMoreThen=-1;
    hLocateStack->nSizeLessThen=-1;
    hLocateStack->nDivisor=1;
    hLocateStack->nTime=LO_TIME_WRITE;
    hLocateStack->bFindFiles=TRUE;
    hLocateStack->bFindDirs=TRUE;
    hLocateStack->bGotoSubDir=TRUE;

    popstringWide(hLocateStack->wszSearchPaths, NSIS_MAX_STRLEN);
    popstringWide(wszOptions, NSIS_MAX_STRLEN);

    if (GetOptionsW(wszOptions, L"/F=", FALSE, wszBuf, NSIS_MAX_STRLEN) && *wszBuf == L'0')
      hLocateStack->bFindFiles=FALSE;
    if (GetOptionsW(wszOptions, L"/D=", FALSE, wszBuf, NSIS_MAX_STRLEN) && *wszBuf == L'0')
      hLocateStack->bFindDirs=FALSE;
    if (GetOptionsW(wszOptions, L"/DE=", FALSE, wszBuf, NSIS_MAX_STRLEN) && *wszBuf == L'1')
    {
      hLocateStack->bFindEmptyDirs=TRUE;
      hLocateStack->bFindDirs=FALSE;
    }
    if (GetOptionsW(wszOptions, L"/S=", FALSE, wszBuf, NSIS_MAX_STRLEN))
    {
      if (*wszBuf)
      {
        hLocateStack->nSizeMoreThen=xatoi64W(wszBuf, &wpOption) * hLocateStack->nDivisor;
        if (*wpOption == L':')
          hLocateStack->nSizeLessThen=xatoi64W(++wpOption, &wpOption) * hLocateStack->nDivisor;
        hLocateStack->bCompareSize=TRUE;

        if (*wpOption == L',')
        {
          if (*++wpOption == L'K' || *wpOption == L'k')
            hLocateStack->nDivisor=1024;
          else if (*wpOption == L'M' || *wpOption == L'm')
            hLocateStack->nDivisor=1048576;
          else if (*wpOption == L'G' || *wpOption == L'g')
            hLocateStack->nDivisor=1073741824;
        }
      }
    }
    if (GetOptionsW(wszOptions, L"/T=", FALSE, wszBuf, NSIS_MAX_STRLEN))
    {
      if (*wszBuf)
      {
        wTimeDayAfterThen=(WORD)xatoiW(wszBuf, &wpOption);
        if (*wpOption == L'.')
          wTimeMonthAfterThen=(WORD)xatoiW(++wpOption, &wpOption);
        if (*wpOption == L'.')
          wTimeYearAfterThen=(WORD)xatoiW(++wpOption, &wpOption);

        if (*wpOption == L':')
        {
          wTimeDayBeforeThen=(WORD)xatoiW(++wpOption, &wpOption);
          if (*wpOption == L'.')
            wTimeMonthBeforeThen=(WORD)xatoiW(++wpOption, &wpOption);
          if (*wpOption == L'.')
            wTimeYearBeforeThen=(WORD)xatoiW(++wpOption, &wpOption);
        }

        if (*wpOption == L',')
        {
          if (*++wpOption == L'C' || *wpOption == L'c')
            hLocateStack->nTime=LO_TIME_CREATION;
          else if (*wpOption == L'A' || *wpOption == L'c')
            hLocateStack->nTime=LO_TIME_ACCESS;

          lpSystemTime.wHour=0;
          lpSystemTime.wMinute=0;
          lpSystemTime.wSecond=0;
          lpSystemTime.wDay=wTimeDayAfterThen;
          lpSystemTime.wMonth=wTimeMonthAfterThen;
          lpSystemTime.wYear=wTimeYearAfterThen;
          SystemTimeToFileTime(&lpSystemTime, &hLocateStack->lpFileTimeAfter);

          lpSystemTime.wHour=23;
          lpSystemTime.wMinute=59;
          lpSystemTime.wSecond=59;
          lpSystemTime.wDay=wTimeDayBeforeThen;
          lpSystemTime.wMonth=wTimeMonthBeforeThen;
          lpSystemTime.wYear=wTimeYearBeforeThen;
          SystemTimeToFileTime(&lpSystemTime, &hLocateStack->lpFileTimeBefore);

          hLocateStack->bCompareTime=TRUE;
        }
      }
    }

    if (GetOptionsW(wszOptions, L"/A=", FALSE, wszBuf, NSIS_MAX_STRLEN))
    {
      if (IsWord(wszBuf, L"READONLY"))
        hLocateStack->nReadOnly=LO_ATTR_INCLUDE;
      else if (IsWord(wszBuf, L"-READONLY"))
        hLocateStack->nReadOnly=LO_ATTR_EXCLUDE;

      if (IsWord(wszBuf, L"ARCHIVE"))
        hLocateStack->nArchive=LO_ATTR_INCLUDE;
      else if (IsWord(wszBuf, L"-ARCHIVE"))
        hLocateStack->nArchive=LO_ATTR_EXCLUDE;

      if (IsWord(wszBuf, L"HIDDEN"))
        hLocateStack->nHidden=LO_ATTR_INCLUDE;
      else if (IsWord(wszBuf, L"-HIDDEN"))
        hLocateStack->nHidden=LO_ATTR_EXCLUDE;

      if (IsWord(wszBuf, L"SYSTEM"))
        hLocateStack->nSystem=LO_ATTR_INCLUDE;
      else if (IsWord(wszBuf, L"-SYSTEM"))
        hLocateStack->nSystem=LO_ATTR_EXCLUDE;

      hLocateStack->bCompareAttr=TRUE;
    }
    if (GetOptionsW(wszOptions, L"/SF=", FALSE, wszBuf, NSIS_MAX_STRLEN))
    {
      if (!xstrcmpiW(wszBuf, L"NAME"))
        hLocateStack->nSortFiles=LO_SORT_FILES_NAME;
      else if (!xstrcmpiW(wszBuf, L"TYPE"))
        hLocateStack->nSortFiles=LO_SORT_FILES_TYPE;
      else if (!xstrcmpiW(wszBuf, L"SIZE"))
        hLocateStack->nSortFiles=LO_SORT_FILES_SIZE;
      else if (!xstrcmpiW(wszBuf, L"DATE"))
        hLocateStack->nSortFiles=LO_SORT_FILES_DATE;
    }
    else if (GetOptionsW(wszOptions, L"/-SF=", FALSE, wszBuf, NSIS_MAX_STRLEN))
    {
      if (!xstrcmpiW(wszBuf, L"NAME"))
        hLocateStack->nSortFiles=LO_SORT_FILES_NAME;
      else if (!xstrcmpiW(wszBuf, L"TYPE"))
        hLocateStack->nSortFiles=LO_SORT_FILES_TYPE;
      else if (!xstrcmpiW(wszBuf, L"SIZE"))
        hLocateStack->nSortFiles=LO_SORT_FILES_SIZE;
      else if (!xstrcmpiW(wszBuf, L"DATE"))
        hLocateStack->nSortFiles=LO_SORT_FILES_DATE;

      hLocateStack->bSortFilesReverse=TRUE;
    }
    if (GetOptionsW(wszOptions, L"/SD=", FALSE, wszBuf, NSIS_MAX_STRLEN))
    {
      if (!xstrcmpiW(wszBuf, L"NAME"))
        hLocateStack->nSortDirs=LO_SORT_DIRS_NAME;
      else if (!xstrcmpiW(wszBuf, L"DATE"))
        hLocateStack->nSortDirs=LO_SORT_DIRS_DATE;
    }
    else if (GetOptionsW(wszOptions, L"/-SD=", FALSE, wszBuf, NSIS_MAX_STRLEN))
    {
      if (!xstrcmpiW(wszBuf, L"NAME"))
        hLocateStack->nSortDirs=LO_SORT_DIRS_NAME;
      else if (!xstrcmpiW(wszBuf, L"DATE"))
        hLocateStack->nSortDirs=LO_SORT_DIRS_DATE;

      hLocateStack->bSortDirsReverse=TRUE;
    }
    if (GetOptionsW(wszOptions, L"/R=", FALSE, wszBuf, NSIS_MAX_STRLEN) && *wszBuf == L'1')
      hLocateStack->bOutputReverse=TRUE;
    if (!GetOptionsW(wszOptions, L"/M=", FALSE, hLocateStack->wszWildcard, NSIS_MAX_STRLEN))
      xstrcpynW(hLocateStack->wszWildcard, L"*.*", MAX_PATH);
    if (GetOptionsW(wszOptions, L"/N=", FALSE, hLocateStack->wszNames, NSIS_MAX_STRLEN) && *hLocateStack->wszNames)
      hLocateStack->bNamesInclude=TRUE;
    else if (GetOptionsW(wszOptions, L"/-N=", FALSE, hLocateStack->wszNames, NSIS_MAX_STRLEN) && *hLocateStack->wszNames)
      hLocateStack->bNamesExclude=TRUE;
    if (GetOptionsW(wszOptions, L"/X=", FALSE, hLocateStack->wszExt, NSIS_MAX_STRLEN))
      hLocateStack->bExtInclude=TRUE;
    else if (GetOptionsW(wszOptions, L"/-X=", FALSE, hLocateStack->wszExt, NSIS_MAX_STRLEN))
      hLocateStack->bExtExclude=TRUE;
    if (GetOptionsW(wszOptions, L"/-PF=", FALSE, hLocateStack->wszExcludeFullPaths, NSIS_MAX_STRLEN) && *hLocateStack->wszExcludeFullPaths)
      hLocateStack->bFullPathsExclude=TRUE;
    if (GetOptionsW(wszOptions, L"/-PN=", FALSE, hLocateStack->wszExcludePathNames, NSIS_MAX_STRLEN) && *hLocateStack->wszExcludePathNames)
      hLocateStack->bPathNamesExclude=TRUE;
    if (GetOptionsW(wszOptions, L"/G=", FALSE, wszBuf, NSIS_MAX_STRLEN) && *wszBuf == L'0')
      hLocateStack->bGotoSubDir=FALSE;
    if (GetOptionsW(wszOptions, L"/B=", FALSE, wszBuf, NSIS_MAX_STRLEN))
    {
      if (*wszBuf == L'1')
      {
        if (!hSearchDetails)
        {
          if (hSearchDetails=FindWindowExA(hwndParent, NULL, "#32770", NULL))
            hSearchDetails=GetDlgItem(hSearchDetails, IDC_STATIC_DETAILS);
        }
        hLocateStack->nBanner=LO_BANNER_DETAILS;
      }
      else if (*wszBuf == L'2') hLocateStack->nBanner=LO_BANNER_CALLBACK;
    }
    if (!hLocateStack->bFindFiles && !hLocateStack->bFindDirs && !hLocateStack->bFindEmptyDirs)
      goto Error;

    //Paths
    if (xstrstrW(hLocateStack->wpCurSearchPath, -1, L"|", 1, TRUE, &wpStrBegin, &wpStrEnd))
    {
      *wpStrBegin=L'\0';
      hLocateStack->wpNextSearchPath=wpStrEnd;
    }
    xstrcpynW(hLocateStack->wszCurPath, hLocateStack->wpCurSearchPath, MAX_PATH);
    TrimPathBackslash(hLocateStack->wszCurPath);

    if (!hLocateStack->wpNextSearchPath)
    {
      xprintfW(wszBuf, L"%s\\*.*", hLocateStack->wszCurPath);
      if ((hLocateStack->hSearch=FindFirstFileWide(wszBuf, &wfd)) == INVALID_HANDLE_VALUE)
        goto Error;
      FindClose(hLocateStack->hSearch);
    }

    pushintegerWide((int)hLocateStack);
    hLocateStack->nGoto=LO_GOTO_BEGIN;
    return;

    Error:
    if (hLocateStack) GlobalFree((HGLOBAL)hLocateStack);
    pushstringWide(L"0");
  }
}

void __declspec(dllexport) _Find(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    HLOCATESTACK *hLocateStack;
    STACKFILE hCurPathStack;
    DIRITEM *lpDirItem;
    FILEITEM *lpFileItem;
    const wchar_t *wpExt;
    wchar_t *wpStrBegin;
    wchar_t *wpStrEnd;
    BOOL bDirEmpty;
    int nUpDown;
    int i;

    if (!(hLocateStack=(HLOCATESTACK *)popintegerWide()))
      goto End;

    if (hLocateStack->nGoto == LO_GOTO_NEXTFILE) goto PopNextFile;
    else if (hLocateStack->nGoto == LO_GOTO_NEXTDIR) goto PopNextDir;
    else if (hLocateStack->nGoto == LO_GOTO_FINDFIRST) goto FindFirst;
    else if (hLocateStack->nGoto == LO_GOTO_BEGIN) goto Begin;
    else if (hLocateStack->nGoto == LO_GOTO_END) goto End;

    Begin:
    TrimPathBackslash(hLocateStack->wszCurPath);
    if (lpDirItem=(DIRITEM *)ItemPushNoSort(&hLocateStack->hPathsStack, sizeof(DIRITEM)))
      xstrcpynW(lpDirItem->wszName, hLocateStack->wszCurPath, MAX_PATH);

    while (hLocateStack->hPathsStack.nElements > 0)
    {
      lpDirItem=(DIRITEM *)hLocateStack->hPathsStack.first;
      xstrcpynW(hLocateStack->wszCurPath, lpDirItem->wszName, MAX_PATH);
      ItemDelete(&hLocateStack->hPathsStack, lpDirItem);

      if (hLocateStack->nBanner == LO_BANNER_DETAILS)
      {
        SetWindowTextWide(hSearchDetails, hLocateStack->wszCurPath);
      }
      else if (hLocateStack->nBanner == LO_BANNER_CALLBACK)
      {
        for (i=0; i < 5; ++i) pushstringWide(L"");
        pushstringWide(hLocateStack->wszCurPath);
        hLocateStack->nGoto=LO_GOTO_FINDFIRST;
        return;
      }

      FindFirst:
      xprintfW(wszBuf, L"%s\\%s", hLocateStack->wszCurPath, hLocateStack->wszWildcard);

      if ((hLocateStack->hSearch=FindFirstFileWide(wszBuf, &wfd)) != INVALID_HANDLE_VALUE)
      {
        do
        {
          if (wfd.cFileName[0] == L'.' && (wfd.cFileName[1] == L'\0' || (wfd.cFileName[1] == L'.' && wfd.cFileName[2] == L'\0'))) continue;

          hLocateStack->nSize=(wfd.nFileSizeHigh * ((__int64)MAXDWORD+1)) + wfd.nFileSizeLow;
          if (hLocateStack->nTime == LO_TIME_WRITE)
            FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &lpLocalFileTime);
          else if (hLocateStack->nTime == LO_TIME_CREATION)
            FileTimeToLocalFileTime(&wfd.ftCreationTime, &lpLocalFileTime);
          else if (hLocateStack->nTime == LO_TIME_ACCESS)
            FileTimeToLocalFileTime(&wfd.ftLastAccessTime, &lpLocalFileTime);

          if ((!hLocateStack->bCompareSize || (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || ((hLocateStack->nSizeMoreThen == -1 || hLocateStack->nSizeMoreThen <= hLocateStack->nSize) && (hLocateStack->nSizeLessThen == -1 || hLocateStack->nSizeLessThen >= hLocateStack->nSize))) &&
              (!hLocateStack->bCompareTime || (CompareFileTime(&lpLocalFileTime, &hLocateStack->lpFileTimeAfter) != -1 && CompareFileTime(&lpLocalFileTime, &hLocateStack->lpFileTimeBefore) != 1)) &&
              (!hLocateStack->bCompareAttr || (((hLocateStack->nReadOnly == LO_ATTR_IGNORE) || (((hLocateStack->nReadOnly == LO_ATTR_INCLUDE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) || ((hLocateStack->nReadOnly == LO_ATTR_EXCLUDE) && !(wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)))) &&
                                               ((hLocateStack->nArchive == LO_ATTR_IGNORE) || (((hLocateStack->nArchive == LO_ATTR_INCLUDE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)) || ((hLocateStack->nArchive == LO_ATTR_EXCLUDE) && !(wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)))) &&
                                               ((hLocateStack->nHidden == LO_ATTR_IGNORE) || (((hLocateStack->nHidden == LO_ATTR_INCLUDE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) || ((hLocateStack->nHidden == LO_ATTR_EXCLUDE) && !(wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)))) &&
                                               ((hLocateStack->nSystem == LO_ATTR_IGNORE) || (((hLocateStack->nSystem == LO_ATTR_INCLUDE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) || ((hLocateStack->nSystem == LO_ATTR_EXCLUDE) && !(wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)))))) &&
              (!hLocateStack->bNamesInclude || IsWord(hLocateStack->wszNames, wfd.cFileName)) &&
              (!hLocateStack->bNamesExclude || !IsWord(hLocateStack->wszNames, wfd.cFileName)))
          {
            if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
              //File
              if (hLocateStack->bFindFiles)
              {
                if (!(wpExt=GetFileExt(wfd.cFileName)))
                  wpExt=L"";

                if ((!hLocateStack->bExtInclude || ((*hLocateStack->wszExt && *wpExt && IsWord(hLocateStack->wszExt, wpExt)) ||
                                                    (!*hLocateStack->wszExt && !*wpExt))) &&
                    (!hLocateStack->bExtExclude || ((*hLocateStack->wszExt && *wpExt && !IsWord(hLocateStack->wszExt, wpExt)) ||
                                                    (!*hLocateStack->wszExt && *wpExt) ||
                                                    (*hLocateStack->wszExt && !*wpExt))))
                {
                  nUpDown=hLocateStack->bSortFilesReverse?-1:1;

                  if (hLocateStack->nSortFiles == LO_SORT_FILES_NAME)
                    lpFileItem=ItemPushSortName(&hLocateStack->hFilesStack, wfd.cFileName, nUpDown, sizeof(FILEITEM));
                  else if (hLocateStack->nSortFiles == LO_SORT_FILES_TYPE)
                    lpFileItem=ItemPushSortType(&hLocateStack->hFilesStack, wfd.cFileName, wpExt, nUpDown, sizeof(FILEITEM));
                  else if (hLocateStack->nSortFiles == LO_SORT_FILES_SIZE)
                    lpFileItem=ItemPushSortSize(&hLocateStack->hFilesStack, wfd.cFileName, hLocateStack->nSize, nUpDown, sizeof(FILEITEM));
                  else if (hLocateStack->nSortFiles == LO_SORT_FILES_DATE)
                    lpFileItem=ItemPushSortDate(&hLocateStack->hFilesStack, wfd.cFileName, &lpLocalFileTime, nUpDown, sizeof(FILEITEM));
                  else
                    lpFileItem=ItemPushNoSort(&hLocateStack->hFilesStack, sizeof(FILEITEM));

                  if (lpFileItem)
                  {
                    xstrcpynW(lpFileItem->wszName, wfd.cFileName, MAX_PATH);
                    xstrcpynW(lpFileItem->wszExt, wpExt, MAX_PATH);
                    lpFileItem->dwAttr=wfd.dwFileAttributes;
                    lpFileItem->ft.dwHighDateTime=lpLocalFileTime.dwHighDateTime;
                    lpFileItem->ft.dwLowDateTime=lpLocalFileTime.dwLowDateTime;
                    lpFileItem->nFileSize=hLocateStack->nSize;
                  }
                }
              }
            }
            else
            {
              //Directory
              if (!hLocateStack->bFindDirs && hLocateStack->bFindEmptyDirs)
              {
                xprintfW(wszBuf, L"%s\\%s", hLocateStack->wszCurPath, wfd.cFileName);
                bDirEmpty=IsDirEmpty(wszBuf);
              }
              else bDirEmpty=FALSE;

              if (hLocateStack->bFindDirs || (hLocateStack->bFindEmptyDirs && bDirEmpty == TRUE))
              {
                nUpDown=hLocateStack->bSortDirsReverse?-1:1;

                if (hLocateStack->nSortDirs == LO_SORT_DIRS_NAME)
                  lpDirItem=(DIRITEM *)ItemPushSortName(&hLocateStack->hDirsStack, wfd.cFileName, nUpDown, sizeof(DIRITEM));
                else if (hLocateStack->nSortDirs == LO_SORT_DIRS_DATE)
                  lpDirItem=(DIRITEM *)ItemPushSortDate(&hLocateStack->hDirsStack, wfd.cFileName, &lpLocalFileTime, nUpDown, sizeof(DIRITEM));
                else
                  lpDirItem=(DIRITEM *)ItemPushNoSort(&hLocateStack->hDirsStack, sizeof(DIRITEM));

                if (lpDirItem)
                {
                  xstrcpynW(lpDirItem->wszName, wfd.cFileName, MAX_PATH);
                  lpDirItem->dwAttr=wfd.dwFileAttributes;
                  lpDirItem->ft.dwHighDateTime=lpLocalFileTime.dwHighDateTime;
                  lpDirItem->ft.dwLowDateTime=lpLocalFileTime.dwLowDateTime;
                }
              }
            }
          }
        }
        while (FindNextFileWide(hLocateStack->hSearch, &wfd));

        FindClose(hLocateStack->hSearch);

        //Return result
        if (hLocateStack->bOutputReverse) goto PopFile;

        PopDir:
        if (hLocateStack->bFindDirs || hLocateStack->bFindEmptyDirs)
        {
          PopNextDir:

          while (hLocateStack->hDirsStack.first)
          {
            ReturnDataFromStack(hLocateStack, (FILEITEM *)hLocateStack->hDirsStack.first);
            ItemDelete(&hLocateStack->hDirsStack, (FILEITEM *)hLocateStack->hDirsStack.first);
            hLocateStack->nGoto=LO_GOTO_NEXTDIR;
            return;
          }
        }
        if (hLocateStack->bOutputReverse) goto PopEnd;

        PopFile:
        if (hLocateStack->bFindFiles)
        {
          PopNextFile:

          while (hLocateStack->hFilesStack.first)
          {
            ReturnDataFromStack(hLocateStack, (FILEITEM *)hLocateStack->hFilesStack.first);
            ItemDelete(&hLocateStack->hFilesStack, (FILEITEM *)hLocateStack->hFilesStack.first);
            hLocateStack->nGoto=LO_GOTO_NEXTFILE;
            return;
          }
        }
        if (hLocateStack->bOutputReverse) goto PopDir;

        PopEnd:;
      }

      if (hLocateStack->bGotoSubDir)
      {
        //Scan for folders
        hCurPathStack.first=0;
        hCurPathStack.last=0;
        hCurPathStack.nElements=0;
        xprintfW(wszBuf, L"%s\\*.*", hLocateStack->wszCurPath);

        if ((hLocateStack->hSearch=FindFirstFileWide(wszBuf, &wfd)) != INVALID_HANDLE_VALUE)
        {
          do
          {
            if (wfd.cFileName[0] == L'.' && (wfd.cFileName[1] == L'\0' || (wfd.cFileName[1] == L'.' && wfd.cFileName[2] == L'\0'))) continue;

            if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
              xprintfW(wszBuf, L"%s\\%s", hLocateStack->wszCurPath, wfd.cFileName);

              if ((!hLocateStack->bFullPathsExclude || !IsWord(hLocateStack->wszExcludeFullPaths, wszBuf)) &&
                  (!hLocateStack->bPathNamesExclude || !IsWord(hLocateStack->wszExcludePathNames, wfd.cFileName)))
              {
                if (hLocateStack->nSortDirs == LO_SORT_DIRS_DATE)
                {
                  if (hLocateStack->nTime == LO_TIME_WRITE)
                    FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &lpLocalFileTime);
                  else if (hLocateStack->nTime == LO_TIME_CREATION)
                    FileTimeToLocalFileTime(&wfd.ftCreationTime, &lpLocalFileTime);
                  else if (hLocateStack->nTime == LO_TIME_ACCESS)
                    FileTimeToLocalFileTime(&wfd.ftLastAccessTime, &lpLocalFileTime);
                }

                nUpDown=hLocateStack->bSortDirsReverse?-1:1;

                if (hLocateStack->nSortDirs == LO_SORT_DIRS_NAME)
                  lpDirItem=(DIRITEM *)ItemPushSortName(&hCurPathStack, wszBuf, nUpDown, sizeof(DIRITEM));
                else if (hLocateStack->nSortDirs == LO_SORT_DIRS_DATE)
                  lpDirItem=(DIRITEM *)ItemPushSortDate(&hCurPathStack, wszBuf, &lpLocalFileTime, nUpDown, sizeof(DIRITEM));
                else
                  lpDirItem=(DIRITEM *)ItemPushNoSort(&hCurPathStack, sizeof(DIRITEM));

                if (lpDirItem)
                {
                  xstrcpynW(lpDirItem->wszName, wszBuf, MAX_PATH);
                  lpDirItem->dwAttr=wfd.dwFileAttributes;
                  lpDirItem->ft.dwHighDateTime=lpLocalFileTime.dwHighDateTime;
                  lpDirItem->ft.dwLowDateTime=lpLocalFileTime.dwLowDateTime;
                }
              }
            }
          }
          while (FindNextFileWide(hLocateStack->hSearch, &wfd));

          FindClose(hLocateStack->hSearch);
        }

        //Merge founded folders
        StackJoin((stack **)&hLocateStack->hPathsStack.first, (stack **)&hLocateStack->hPathsStack.last, (stack *)hLocateStack->hPathsStack.first, (stack *)hCurPathStack.first, (stack *)hCurPathStack.last);
        hLocateStack->hPathsStack.nElements+=hCurPathStack.nElements;
      }
    }
    if (hLocateStack->wpNextSearchPath)
    {
      hLocateStack->wpCurSearchPath=hLocateStack->wpNextSearchPath;

      if (xstrstrW(hLocateStack->wpCurSearchPath, -1, L"|", 1, TRUE, &wpStrBegin, &wpStrEnd))
      {
        *wpStrBegin=L'\0';
        hLocateStack->wpNextSearchPath=wpStrEnd;
      }
      else hLocateStack->wpNextSearchPath=NULL;

      xstrcpynW(hLocateStack->wszCurPath, hLocateStack->wpCurSearchPath, MAX_PATH);
      TrimPathBackslash(hLocateStack->wszCurPath);
      hLocateStack->nGoto=LO_GOTO_BEGIN;
      goto Begin;
    }
    hLocateStack->nGoto=LO_GOTO_END;

    End:
    for (i=0; i < 6; ++i) pushstringWide(L"");
  }
}

void __declspec(dllexport) _Close(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    HLOCATESTACK *hLocateStack;

    hLocateStack=(HLOCATESTACK *)popintegerWide();

    if (hLocateStack)
    {
      if (hLocateStack->hSearch)
        FindClose(hLocateStack->hSearch);
      if (hLocateStack->nBanner == LO_BANNER_DETAILS)
        SetWindowTextWide(hSearchDetails, L"");
      ItemFree(&hLocateStack->hPathsStack);
      hLocateStack->nGoto=LO_GOTO_END;
      GlobalFree((HGLOBAL)hLocateStack);
    }
  }
}
#endif //LOCATE

#ifdef GETSIZE
void __declspec(dllexport) _GetSize(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nDivisorGS=1;
    int i;
    BOOL bGotoSubdirGS=TRUE;

    nDirSizeGS=0;
    nFileSumGS=0;
    nDirSumGS=0;
    nBannerGS=0;
    nFunction=GETSIZE_FUNC;

    popstringWide(wszBuf, NSIS_MAX_STRLEN);
    popstringWide(wszOptions, NSIS_MAX_STRLEN);
    TrimPathBackslash(wszBuf);

    if (GetOptionsW(wszOptions, L"/S=", FALSE, wszBuf2, NSIS_MAX_STRLEN))
    {
      if (*wszBuf2 == L'K')
        nDivisorGS=1024;
      else if (*wszBuf2 == L'M')
        nDivisorGS=1048576;
      else if (*wszBuf2 == L'G')
        nDivisorGS=1073741824;
    }
    if (GetOptionsW(wszOptions, L"/G=", FALSE, wszBuf2, NSIS_MAX_STRLEN) && *wszBuf2 == L'0')
      bGotoSubdirGS=FALSE;
    if (GetOptionsW(wszOptions, L"/B=", FALSE, wszBuf2, NSIS_MAX_STRLEN) && *wszBuf2 == L'1')
    {
      nBannerGS=1;

      if (!hSearchDetails)
      {
        if (hSearchDetails=FindWindowExA(hwndParent, NULL, "#32770", NULL))
          hSearchDetails=GetDlgItem(hSearchDetails, IDC_STATIC_DETAILS);
      }
    }
    if (!GetOptionsW(wszOptions, L"/M=", FALSE, wszBuf2, NSIS_MAX_STRLEN))
      xstrcpynW(wszBuf2, L"*.*", MAX_PATH);

    if (SearchFiles(wszBuf, wszBuf2, bGotoSubdirGS))
    {
      nDirSizeGS=nDirSizeGS / nDivisorGS;

      pushintegerWide(--nDirSumGS);
      pushintegerWide(nFileSumGS);
      xi64toaW(nDirSizeGS, wszBuf2);
      pushstringWide(wszBuf2);
    }
    else
    {
      for (i=0; i < 3; ++i) pushstringWide(L"-1");
    }
    if (nBannerGS == 1) SetWindowTextWide(hSearchDetails, L"");
  }
}
#endif //GETSIZE

#ifdef RMDIR_EMPTY
void __declspec(dllexport) _RMDirEmpty(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    BOOL bGotoSubdirRM=TRUE;

    nRemoveDirRM=0;
    nBannerRM=0;
    nFunction=RMDIR_EMPTY_FUNC;

    popstringWide(wszBuf, NSIS_MAX_STRLEN);
    popstringWide(wszOptions, NSIS_MAX_STRLEN);
    TrimPathBackslash(wszBuf);

    if (GetOptionsW(wszOptions, L"/G=", FALSE, wszBuf2, NSIS_MAX_STRLEN) && *wszBuf2 == L'0')
      bGotoSubdirRM=FALSE;
    if (GetOptionsW(wszOptions, L"/B=", FALSE, wszBuf2, NSIS_MAX_STRLEN) && *wszBuf2 == L'1')
    {
      nBannerRM=1;

      if (!hSearchDetails)
      {
        if (hSearchDetails=FindWindowExA(hwndParent, NULL, "#32770", NULL))
          hSearchDetails=GetDlgItem(hSearchDetails, IDC_STATIC_DETAILS);
      }
    }
    if (!GetOptionsW(wszOptions, L"/M=", FALSE, wszBuf2, NSIS_MAX_STRLEN))
      xstrcpynW(wszBuf2, L"*.*", MAX_PATH);

    if (SearchFiles(wszBuf, wszBuf2, bGotoSubdirRM))
      pushintegerWide(nRemoveDirRM);
    else
      pushstringWide(L"-1");
    if (nBannerRM == 1) SetWindowTextWide(hSearchDetails, L"");
  }
}
#endif //RMDIR_EMPTY

void __declspec(dllexport) _Unload(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  return TRUE;
}

FILEITEM* ItemPushNoSort(STACKFILE *hStack, int nBytes)
{
  FILEITEM *tmp;

  if (!StackInsertBefore((stack **)&hStack->first, (stack **)&hStack->last, (stack *)NULL, (stack **)&tmp, nBytes))
    ++hStack->nElements;
  return tmp;
}

FILEITEM* ItemPushSortName(STACKFILE *hStack, const wchar_t *wpName, int nUpDown, int nBytes)
{
  FILEITEM *tmp;
  int i;

  if (nUpDown != 1 && nUpDown != -1) return NULL;

  for (tmp=(FILEITEM *)hStack->first; tmp; tmp=tmp->next)
  {
    i=xstrcmpiW(tmp->wszName, wpName);
    if (i == 0 || i == nUpDown) break;
  }
  if (!StackInsertBefore((stack **)&hStack->first, (stack **)&hStack->last, (stack *)tmp, (stack **)&tmp, nBytes))
    ++hStack->nElements;
  return tmp;
}

FILEITEM* ItemPushSortType(STACKFILE *hStack, const wchar_t *wpName, const wchar_t *wpExt, int nUpDown, int nBytes)
{
  FILEITEM *tmp;
  int i;

  if (nUpDown != 1 && nUpDown != -1) return NULL;

  for (tmp=(FILEITEM *)hStack->first; tmp; tmp=tmp->next)
  {
    i=xstrcmpiW(tmp->wszExt, wpExt);
    if (i == nUpDown) break;
    else if (i == 0)
    {
      i=xstrcmpiW(tmp->wszName, wpName);
      if (i == 0 || i == nUpDown) break;
    }
  }
  if (!StackInsertBefore((stack **)&hStack->first, (stack **)&hStack->last, (stack *)tmp, (stack **)&tmp, nBytes))
    ++hStack->nElements;
  return tmp;
}

FILEITEM* ItemPushSortSize(STACKFILE *hStack, const wchar_t *wpName, __int64 nSize, int nUpDown, int nBytes)
{
  FILEITEM *tmp;
  int i;

  if (nUpDown != 1 && nUpDown != -1) return NULL;

  for (tmp=(FILEITEM *)hStack->first; tmp; tmp=tmp->next)
  {
    if ((nUpDown == 1 && nSize > tmp->nFileSize) ||
        (nUpDown == -1 && nSize < tmp->nFileSize))
    {
      break;
    }
    else if (nSize == tmp->nFileSize)
    {
      i=xstrcmpiW(tmp->wszName, wpName);
      if (i == 0 || i == nUpDown) break;
    }
  }
  if (!StackInsertBefore((stack **)&hStack->first, (stack **)&hStack->last, (stack *)tmp, (stack **)&tmp, nBytes))
    ++hStack->nElements;
  return tmp;
}

FILEITEM* ItemPushSortDate(STACKFILE *hStack, const wchar_t *wpName, const FILETIME *ft, int nUpDown, int nBytes)
{
  FILEITEM *tmp;
  int i;

  if (nUpDown != 1 && nUpDown != -1) return NULL;

  for (tmp=(FILEITEM *)hStack->first; tmp; tmp=tmp->next)
  {
    i=CompareFileTime(&tmp->ft, ft);
    if (i == nUpDown) break;
    else if (i == 0)
    {
      i=xstrcmpiW(tmp->wszName, wpName);
      if (i == 0 || i == nUpDown) break;
    }
  }
  if (!StackInsertBefore((stack **)&hStack->first, (stack **)&hStack->last, (stack *)tmp, (stack **)&tmp, nBytes))
    ++hStack->nElements;
  return tmp;
}

void ItemDelete(STACKFILE *hStack, void *lpFileItem)
{
  if (!StackDelete((stack **)&hStack->first, (stack **)&hStack->last, (stack *)lpFileItem))
    --hStack->nElements;
}

void ItemFree(STACKFILE *hStack)
{
  StackClear((stack **)&hStack->first, (stack **)&hStack->last);
  hStack->nElements=0;
}

void ReturnDataFromStack(HLOCATESTACK *hLocateStack, FILEITEM *lpFileItem)
{
  //attributes
  if (lpFileItem->dwAttr & FILE_ATTRIBUTE_READONLY)
    wszBuf[0]=L'r';
  else
    wszBuf[0]=L'-';
  if (lpFileItem->dwAttr & FILE_ATTRIBUTE_ARCHIVE)
    wszBuf[1]=L'a';
  else
    wszBuf[1]=L'-';
  if (lpFileItem->dwAttr & FILE_ATTRIBUTE_HIDDEN)
    wszBuf[2]=L'h';
  else
    wszBuf[2]=L'-';
  if (lpFileItem->dwAttr & FILE_ATTRIBUTE_SYSTEM)
    wszBuf[3]=L's';
  else
    wszBuf[3]=L'-';
  wszBuf[4]=L'\0';
  pushstringWide(wszBuf);

  //date
  FileTimeToSystemTime(&lpFileItem->ft, &lpSystemTime);
  xprintfW(wszBuf, L"%02d.%02d.%d %02d:%02d:%02d", lpSystemTime.wDay, lpSystemTime.wMonth, lpSystemTime.wYear, lpSystemTime.wHour, lpSystemTime.wMinute, lpSystemTime.wSecond);
  pushstringWide(wszBuf);

  //size
  if (lpFileItem->dwAttr & FILE_ATTRIBUTE_DIRECTORY)
    wszBuf[0]=L'\0';
  else
    xi64toaW(lpFileItem->nFileSize / hLocateStack->nDivisor, wszBuf);
  pushstringWide(wszBuf);

  //name
  pushstringWide(lpFileItem->wszName);

  //path
  pushstringWide(hLocateStack->wszCurPath);

  //path\name
  xprintfW(wszBuf, L"%s\\%s", hLocateStack->wszCurPath, lpFileItem->wszName);
  pushstringWide(wszBuf);
}

//Function: Recursively finds files and directories
BOOL SearchFiles(const wchar_t *wpPath, const wchar_t *wpWildcard, BOOL bSubdir)
{
  wchar_t wszNameTmp[MAX_PATH];
  WIN32_FIND_DATAW wfd;
  HANDLE hSearch;

  if (bSubdir)
  {
    xprintfW(wszNameTmp, L"%s\\*.*", wpPath);

    if ((hSearch=FindFirstFileWide(wszNameTmp, &wfd)) != INVALID_HANDLE_VALUE)
    {
      do
      {
        if (wfd.cFileName[0] == L'.' && (wfd.cFileName[1] == L'\0' || (wfd.cFileName[1] == L'.' && wfd.cFileName[2] == L'\0'))) continue;

        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          xprintfW(wszNameTmp, L"%s\\%s", wpPath, wfd.cFileName);

          SearchFiles(wszNameTmp, wpWildcard, TRUE);
        }
      }
      while (FindNextFileWide(hSearch, &wfd));
    }
    else
      return FALSE;

    FindClose(hSearch);
  }
  xprintfW(wszNameTmp, L"%s\\%s", wpPath, wpWildcard);

  /* Plugin */
  if (nBannerGS == 1)
    SetWindowTextWide(hSearchDetails, wszNameTmp);
  if (nFunction == GETSIZE_FUNC)
    ++nDirSumGS;
  /**********/

  if ((hSearch=FindFirstFileWide(wszNameTmp, &wfd)) == INVALID_HANDLE_VALUE)
    return TRUE;
  do
  {
    if (wfd.cFileName[0] == L'.' && (wfd.cFileName[1] == L'\0' || (wfd.cFileName[1] == L'.' && wfd.cFileName[2] == L'\0'))) continue;

    xprintfW(wszNameTmp, L"%s\\%s", wpPath, wfd.cFileName);

    /* Plugin */
    if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      if (nFunction == GETSIZE_FUNC)
      {
        ++nFileSumGS;
        nDirSizeGS+=((wfd.nFileSizeHigh * ((__int64)MAXDWORD+1)) + wfd.nFileSizeLow);
      }
    }
    else if (nFunction == GETSIZE_FUNC && !bSubdir)
      ++nDirSumGS;
    else if (nFunction == RMDIR_EMPTY_FUNC && RemoveDirectoryWide(wszNameTmp))
      ++nRemoveDirRM;
    /**********/
  }
  while (FindNextFileWide(hSearch, &wfd));

  FindClose(hSearch);

  return TRUE;
}

BOOL IsDirEmpty(const wchar_t *wpDir)
{
  wchar_t wszWildcard[MAX_PATH];
  WIN32_FIND_DATAW wfd;
  HANDLE hFind;
  BOOL bDirEmpty=TRUE;

  xprintfW(wszWildcard, L"%s\\*.*", wpDir);

  if ((hFind=FindFirstFileWide(wszWildcard, &wfd)) != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (wfd.cFileName[0] == L'.' && (wfd.cFileName[1] == L'\0' || (wfd.cFileName[1] == L'.' && wfd.cFileName[2] == L'\0'))) continue;
      bDirEmpty=FALSE;
      break;
    }
    while (FindNextFileWide(hFind, &wfd));

    FindClose(hFind);
  }
  else bDirEmpty=FALSE;

  return bDirEmpty;
}

const wchar_t* GetFileExt(const wchar_t *wpFile)
{
  int i;

  for (i=xstrlenW(wpFile) - 1; i >= 0; --i)
  {
    if (wpFile[i] == L'.') return (wchar_t *)(wpFile + i + 1);
    else if (wpFile[i] == L'\\') break;
  }
  return NULL;
}

void TrimPathBackslash(wchar_t *wszCurPath)
{
  int i;

  for (i=xstrlenW(wszCurPath) - 1; i >= 0; --i)
  {
    if (wszCurPath[i] == L'\\')
      wszCurPath[i]=L'\0';
    else
      break;
  }
}

BOOL IsWord(const wchar_t *wpText, const wchar_t *wpWord)
{
  wchar_t *wpStrBegin;
  wchar_t *wpStrEnd;

  if (xstrstrW(wpText, -1, wpWord, -1, FALSE, &wpStrBegin, &wpStrEnd) &&
      (wpStrBegin == wpText || *(wpStrBegin - 1) == L'|') &&
      (!*wpStrEnd || *wpStrEnd == L'|'))
  {
    return TRUE;
  }
  return FALSE;
}

int GetOptionsW(wchar_t *wpLine, wchar_t *wpOption, BOOL bSensitive, wchar_t *wszResult, int nMaxResult)
{
  wchar_t *wpLineStart;
  wchar_t *wpLineEnd;
  wchar_t *wpOptionCount;
  wchar_t *wpOptionString=NULL;
  wchar_t wchQuote=L'\0';
  wchar_t wchDelimiter=*wpOption++;
  int nResult=0;

  for (wpLineStart=wpLineEnd=wpLine; *wpLineStart && *wpLineEnd; ++wpLineStart)
  {
    if (wchQuote == L'\0' && *wpLineStart != L'\"' && *wpLineStart != L'\'' && *wpLineStart != L'`')
    {
      if (*wpLineStart == wchDelimiter)
      {
        if (wpOptionString) break;

        for (wpLineEnd=wpLineStart + 1, wpOptionCount=wpOption;
             *wpLineEnd && (*wpLineEnd == *wpOptionCount ||
                            #if defined WideCharLower_INCLUDED
                              (!bSensitive && WideCharLower(*wpLineEnd) == WideCharLower(*wpOptionCount)));
                            #elif defined WideCharUpper_INCLUDED
                              (!bSensitive && WideCharUpper(*wpLineEnd) == WideCharUpper(*wpOptionCount)));
                            #else
                              #pragma message ("NOTE: WideCharLower and WideCharUpper undefined - xstrrepW will not work on Win95/98/Me.")
                              (!bSensitive && (UINT_PTR)CharUpperW((wchar_t *)(UINT_PTR)(WORD)*wpLineEnd) == (UINT_PTR)CharUpperW((wchar_t *)(UINT_PTR)(WORD)*wpOptionCount)));
                            #endif
             ++wpLineEnd)
        {
          if (!*++wpOptionCount)
          {
            wpLineStart=wpLineEnd;
            wpOptionString=wpLineEnd + 1;
            break;
          }
        }
      }
    }
    else if (wchQuote == L'\0')
      wchQuote=*wpLineStart;
    else if (*wpLineStart == wchQuote)
      wchQuote=L'\0';
  }
  if (wpOptionString)
  {
    while (*--wpLineStart == L' ' || *wpLineStart == L'\t');

    if (*wpOptionString == *wpLineStart && (*wpOptionString == L'\"' || *wpOptionString == L'\'' || *wpOptionString == L'`'))
    {
      ++wpOptionString, --wpLineStart;
    }
    nResult=wpLineStart - wpOptionString + 2;
    if (wszResult)
    {
      if (nMaxResult < nResult) nResult=nMaxResult;
      nResult=xstrcpynW(wszResult, wpOptionString, nResult);
    }
  }
  return nResult;
}

int popintegerWide()
{
  wchar_t wszInt[32];

  wszInt[0]=L'\0';
  popstringWide(wszInt, 31);
  return xatoiW(wszInt, NULL);
}

void pushintegerWide(int integer)
{
  wchar_t wszInt[32];

  xitoaW(integer, wszInt);
  pushstringWide(wszInt);
}

int popstringAnsi(char *str, int len)
{
  stack_t *th;

  if (g_stacktop && *g_stacktop)
  {
    th=(*g_stacktop);
    if (g_unicode)
    {
      if (len=WideCharToMultiByte(CP_ACP, 0, (const wchar_t *)th->text, -1, str, len, NULL, NULL))
        str[--len]='\0';
    }
    else
    {
      len=xstrcpynA(str, (const char *)th->text, len);
    }
    *g_stacktop=th->next;
    GlobalFree((HGLOBAL)th);
    return len;
  }
  return -1;
}

int popstringWide(wchar_t *str, int len)
{
  stack_t *th;

  if (g_stacktop && *g_stacktop)
  {
    th=(*g_stacktop);
    if (g_unicode)
    {
      len=xstrcpynW(str, th->text, len);
    }
    else
    {
      if (len=MultiByteToWideChar(CP_ACP, 0, (const char *)th->text, -1, str, len))
        str[--len]=L'\0';
    }
    *g_stacktop=th->next;
    GlobalFree((HGLOBAL)th);
    return len;
  }
  return -1;
}

void pushstringAnsi(const char *str)
{
  stack_t *th;

  if (g_stacktop)
  {
    if (g_unicode)
    {
      th=(stack_t *)GlobalAlloc(GPTR, sizeof(stack_t) + NSIS_MAX_STRLEN * sizeof(wchar_t));
      MultiByteToWideChar(CP_ACP, 0, str, -1, (wchar_t *)th->text, NSIS_MAX_STRLEN);
    }
    else
    {
      th=(stack_t *)GlobalAlloc(GPTR, sizeof(stack_t) + NSIS_MAX_STRLEN);
      xstrcpynA((char *)th->text, str, NSIS_MAX_STRLEN);
    }
    th->next=*g_stacktop;
    *g_stacktop=th;
  }
}

void pushstringWide(const wchar_t *str)
{
  stack_t *th;

  if (g_stacktop)
  {
    if (g_unicode)
    {
      th=(stack_t *)GlobalAlloc(GPTR, sizeof(stack_t) + NSIS_MAX_STRLEN * sizeof(wchar_t));
      xstrcpynW(th->text, str, NSIS_MAX_STRLEN);
    }
    else
    {
      th=(stack_t *)GlobalAlloc(GPTR, sizeof(stack_t) + NSIS_MAX_STRLEN);
      WideCharToMultiByte(CP_ACP, 0, str, -1, (char *)th->text, NSIS_MAX_STRLEN, NULL, NULL);
    }
    th->next=*g_stacktop;
    *g_stacktop=th;
  }
}

void Initialize(int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  g_stacktop=stacktop;
  g_variables=variables;
  g_stringsize=string_size;
  g_pluginParms=extra;

  if (g_unicode == -1)
  {
    wchar_t wszCurPath[]=L"C:\\";

    g_pluginParms->validate_filename(wszCurPath);
    g_unicode=(wszCurPath[2] == L'\\')?FALSE:TRUE;

    //Initialize WideFunc.h header
    WideInitialize();
    WideGlobal_bOldWindows=!g_unicode;
  }
}
