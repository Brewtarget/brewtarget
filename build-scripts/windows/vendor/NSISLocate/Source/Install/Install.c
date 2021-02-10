/*****************************************************************
 *                       NSIS plugin setup                       *
 *                                                               *
 * 2011 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)  *
 *****************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include "..\StrFunc.h"
#include "..\WideFunc.h"
#include "Resources\Resource.h"

#define PROGRAM_NAME        L"Locate"
#define PROGRAM_CAPTION     PROGRAM_NAME L" Plugin Setup"
#define PROGRAM_REGROOT     HKEY_LOCAL_MACHINE
#define PROGRAM_REGKEYA     L"SOFTWARE\\NSIS"
#define PROGRAM_REGKEYW     L"SOFTWARE\\NSIS\\Unicode"
#define PROGRAM_FILE        L"makensis.exe"
#define PROGRAM_BROWSETEXT  L"Choose NSIS directory:"


//Include string functions
#define xmemcpy
#define xstrcpynW
#define xstrlenW
#define xatoiW
#define xitoaW
#define xuitoaW
#define dec2hexW
#define xprintfW
#include "..\StrFunc.h"

//Include wide functions
#define DispatchMessageWide
#define FileExistsWide
#define CopyFileWide
#define CreateDirectoryWide
#define GetFileAttributesWide
#define GetMessageWide
#define GetModuleFileNameWide
#define GetWindowTextWide
#define IsDialogMessageWide
#define RegOpenKeyExWide
#define RegQueryValueExWide
#define SetWindowTextWide
#define SHBrowseForFolderWide
#define SHGetPathFromIDListWide
#include "..\WideFunc.h"

//Defines
#ifndef BIF_NONEWFOLDERBUTTON
  #define BIF_NONEWFOLDERBUTTON 0x200
#endif
#ifndef BIF_NEWDIALOGSTYLE
  #define BIF_NEWDIALOGSTYLE 0x0040
#endif

//Global variables
HINSTANCE hInstance;
HICON hIcon;
wchar_t wszExeDir[MAX_PATH];
wchar_t wszBuffer[MAX_PATH];

//Functions prototypes
BOOL CALLBACK SelectPath(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
BOOL CopyFileWithMessages(HWND hWnd, wchar_t *wpSourcePath, wchar_t *wpSourceFile, wchar_t *wpTargetPath, wchar_t *wpTargetFile);
int GetExeDir(HINSTANCE hInstance, wchar_t *wszExeDir, int nLen);
void TrimBackslash(wchar_t *wszPath);

void _WinMain()
{
  HWND hDlg;
  MSG msg;

  //Initialize WideFunc.h header
  WideInitialize();

  hInstance=GetModuleHandle(NULL);
  hIcon=LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_ICON));
  GetExeDir(hInstance, wszExeDir, MAX_PATH);

  if (WideGlobal_bOldWindows)
    hDlg=CreateDialogA(hInstance, MAKEINTRESOURCEA(IDD_PATH), 0, (DLGPROC)SelectPath);
  else
    hDlg=CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_PATH), 0, (DLGPROC)SelectPath);
  if (hDlg) ShowWindow(hDlg, SW_SHOW);

  while (GetMessageWide(&msg, 0, 0, 0))
  {
    if (!IsDialogMessageWide(hDlg, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessageWide(&msg);
    }
  }
  ExitProcess(0);
}

BOOL CALLBACK SelectPath(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static HWND hWndPathAnsi;
  static HWND hWndPathUni;
  static HWND hWndOK;
  HKEY hKey;
  DWORD dwType;
  DWORD dwSize;

  if (uMsg == WM_INITDIALOG)
  {
    SendMessage(hDlg, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
    hWndPathAnsi=GetDlgItem(hDlg, IDC_PATHANSI);
    hWndPathUni=GetDlgItem(hDlg, IDC_PATHUNI);
    hWndOK=GetDlgItem(hDlg, IDOK);

    SetWindowTextWide(hDlg, PROGRAM_CAPTION);
    SendMessage(hWndPathAnsi, EM_LIMITTEXT, MAX_PATH, 0);

    wszBuffer[0]=L'\0';
    if (RegOpenKeyExWide(PROGRAM_REGROOT, PROGRAM_REGKEYA, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
      dwSize=MAX_PATH;
      RegQueryValueExWide(hKey, L"", NULL, &dwType, (unsigned char *)wszBuffer, &dwSize);
      RegCloseKey(hKey);
    }
    SetWindowTextWide(hWndPathAnsi, wszBuffer);

    wszBuffer[0]=L'\0';
    if (RegOpenKeyExWide(PROGRAM_REGROOT, PROGRAM_REGKEYW, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
      dwSize=MAX_PATH;
      RegQueryValueExWide(hKey, L"", NULL, &dwType, (unsigned char *)wszBuffer, &dwSize);
      RegCloseKey(hKey);
    }
    SetWindowTextWide(hWndPathUni, wszBuffer);
  }
  else if (uMsg == WM_COMMAND)
  {
    if (LOWORD(wParam) == IDC_PATHANSI ||
        LOWORD(wParam) == IDC_PATHUNI)
    {
      GetWindowTextWide((HWND)lParam, wszBuffer, MAX_PATH);
      TrimBackslash(wszBuffer);
      xprintfW(wszBuffer, L"%s\\%s", wszBuffer, PROGRAM_FILE);
      EnableWindow((HWND)lParam, FileExistsWide(wszBuffer));

      if (!IsWindowEnabled(hWndPathAnsi) && !IsWindowEnabled(hWndPathUni))
        EnableWindow(hWndOK, FALSE);
      else
        EnableWindow(hWndOK, TRUE);
      return TRUE;
    }
    else if (LOWORD(wParam) == IDC_BROWSEANSI ||
             LOWORD(wParam) == IDC_BROWSEUNI)
    {
      BROWSEINFOW bi;
      LPITEMIDLIST pIdList;
      LPMALLOC pMalloc;
      HWND hWndPath;

      if (LOWORD(wParam) == IDC_BROWSEANSI)
        hWndPath=hWndPathAnsi;
      else
        hWndPath=hWndPathUni;
      GetWindowTextWide(hWndPath, wszBuffer, MAX_PATH);
      bi.hwndOwner=hDlg;
      bi.pidlRoot=NULL;
      bi.pszDisplayName=wszBuffer;
      bi.lpszTitle=PROGRAM_BROWSETEXT;
      bi.ulFlags=BIF_RETURNONLYFSDIRS|BIF_NONEWFOLDERBUTTON|BIF_NEWDIALOGSTYLE;
      bi.lpfn=BrowseCallbackProc;
      bi.lParam=(LPARAM)wszBuffer;
      bi.iImage=0;

      if (pIdList=SHBrowseForFolderWide(&bi))
      {
        SHGetPathFromIDListWide(pIdList, wszBuffer);

        if (SHGetMalloc(&pMalloc))
        {
          pMalloc->lpVtbl->Free(pMalloc, pIdList);
          pMalloc->lpVtbl->Release(pMalloc);
        }
        SetWindowTextWide(hWndPath, wszBuffer);
      }
      return TRUE;
    }
    else if (LOWORD(wParam) == IDOK)
    {
      HWND lpWnd[]={hWndPathAnsi, hWndPathUni, 0};
      wchar_t szSourcePath[MAX_PATH];
      wchar_t szTargetPath[MAX_PATH];
      int i;
      BOOL bResult=FALSE;

      for (i=0; lpWnd[i]; ++i)
      {
        if (IsWindowEnabled(lpWnd[i]))
        {
          GetWindowTextWide(lpWnd[i], wszBuffer, MAX_PATH);

          //Plugin installation
          xprintfW(szTargetPath, L"%s\\Docs\\%s", wszBuffer, PROGRAM_NAME);
          if (!(bResult=CopyFileWithMessages(hDlg, wszExeDir, L"Readme.txt", szTargetPath, NULL)))
            break;
          xprintfW(szSourcePath, L"%s\\Example", wszExeDir);
          xprintfW(szTargetPath, L"%s\\Examples\\%s", wszBuffer, PROGRAM_NAME);
          if (!(bResult=CopyFileWithMessages(hDlg, szSourcePath, PROGRAM_NAME L"Test.nsi", szTargetPath, NULL)))
            break;
          xprintfW(szSourcePath, L"%s\\Include", wszExeDir);
          xprintfW(szTargetPath, L"%s\\Include", wszBuffer);
          if (!(bResult=CopyFileWithMessages(hDlg, szSourcePath, PROGRAM_NAME L".nsh", szTargetPath, NULL)))
            break;
          xprintfW(szSourcePath, L"%s\\Plugin", wszExeDir);
          xprintfW(szTargetPath, L"%s\\Plugins", wszBuffer);
          if (!(bResult=CopyFileWithMessages(hDlg, szSourcePath, PROGRAM_NAME L".dll", szTargetPath, NULL)))
            break;
        }
      }

      ShowWindow(hDlg, SW_HIDE);
      if (bResult) MessageBoxW(NULL, L"Installation complite", PROGRAM_CAPTION, MB_OK|MB_ICONINFORMATION);
      DestroyWindow(hDlg);
      return TRUE;
    }
    else if (LOWORD(wParam) == IDCANCEL)
    {
      DestroyWindow(hDlg);
      return TRUE;
    }
  }
  else if (uMsg == WM_DESTROY)
  {
    PostQuitMessage(0);
  }
  return FALSE;
}

int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
  char szPath[MAX_PATH];
  wchar_t wszPath[MAX_PATH];
  BOOL bEnable=FALSE;

  if (uMsg == BFFM_INITIALIZED || uMsg == BFFM_SELCHANGED)
  {
    if (uMsg == BFFM_INITIALIZED)
    {
      if (WideGlobal_bOldWindows)
      {
        WideCharToMultiByte(CP_ACP, 0, (wchar_t *)lpData, -1, szPath, MAX_PATH, NULL, NULL);
        SendMessage(hWnd, BFFM_SETSELECTIONA, TRUE, (LPARAM)szPath);
      }
      else
      {
        xstrcpynW(wszPath, (wchar_t *)lpData, MAX_PATH);
        SendMessage(hWnd, BFFM_SETSELECTIONW, TRUE, (LPARAM)wszPath);
      }
    }
    else if (uMsg == BFFM_SELCHANGED)
    {
      SHGetPathFromIDListWide((LPITEMIDLIST)lParam, wszPath);
    }

    if (*wszPath)
    {
      TrimBackslash(wszPath);
      xprintfW(wszPath, L"%s\\%s", wszPath, PROGRAM_FILE);
      bEnable=FileExistsWide(wszPath);
    }
    SendMessage(hWnd, BFFM_ENABLEOK, 0, bEnable);
  }
  return 0;
}

BOOL CopyFileWithMessages(HWND hWnd, wchar_t *wpSourcePath, wchar_t *wpSourceFile, wchar_t *wpTargetPath, wchar_t *wpTargetFile)
{
  wchar_t wszSource[MAX_PATH];
  wchar_t wszTarget[MAX_PATH];
  wchar_t wszTmp[MAX_PATH+32];
  int nChoice;

  xprintfW(wszSource, L"%s\\%s", wpSourcePath, wpSourceFile);
  if (!FileExistsWide(wszSource))
  {
    xprintfW(wszTmp, L"%s\n\nFile does not exists. Continue?", wszSource);
    if (MessageBoxW(hWnd, wszTmp, PROGRAM_CAPTION, MB_YESNO|MB_ICONEXCLAMATION|MB_DEFBUTTON2) == IDNO)
      return FALSE;
    return TRUE;
  }

  xprintfW(wszTmp, L"%s\\*.*", wpTargetPath);
  if (!FileExistsWide(wszTmp))
  {
    CreateDirectoryWide(wpTargetPath, NULL);
  }

  xprintfW(wszTarget, L"%s\\%s", wpTargetPath, wpTargetFile?wpTargetFile:wpSourceFile);
  if (FileExistsWide(wszTarget))
  {
    xprintfW(wszTmp, L"%s\n\nFile already exists. Replace it?", wszTarget);

    nChoice=MessageBoxW(hWnd, wszTmp, PROGRAM_CAPTION, MB_YESNOCANCEL|MB_ICONQUESTION);

    if (nChoice == IDNO)
    {
      return TRUE;
    }
    else if (nChoice == IDCANCEL)
    {
      MessageBoxW(hWnd, L"Installation aborted", PROGRAM_CAPTION, MB_OK|MB_ICONEXCLAMATION);
      return FALSE;
    }
  }
  CopyFileWide(wszSource, wszTarget, FALSE);

  return TRUE;
}

int GetExeDir(HINSTANCE hInstance, wchar_t *wszExeDir, int nLen)
{
  if (nLen=GetModuleFileNameWide(hInstance, wszExeDir, nLen))
  {
    while (nLen > 0 && wszExeDir[nLen] != '\\') --nLen;
    wszExeDir[nLen]='\0';
  }
  return nLen;
}

void TrimBackslash(wchar_t *wszPath)
{
  wchar_t *wpPath=wszPath + xstrlenW(wszPath) - 1;

  while (wpPath >= wszPath && *wpPath == '\\') *wpPath--='\0';
}
