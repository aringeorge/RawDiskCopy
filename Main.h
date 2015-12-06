#pragma once

#include <windows.h>

//#define WIN_XP
#define WIN_7
//#define WIN_VISTA

bool CheckCommandLineArguments(int argc, char *argv[]);
void FormatSourceDestinationWinXP7(char szName[51], char *argv);
bool FormatSourceDestinationVista(char szName[51], char *argv);
bool CreateSourceFile(char szName[51], HANDLE &hHandle);
bool CreateDestinationFile(char szName[51], HANDLE &hHandle, HANDLE &hSrcHandle);
bool Win7SpecificIOCtl(HANDLE hSrcHandle, HANDLE hDstHandle);
bool CloseFiles(HANDLE hSrcHandle, HANDLE hDstHandle);
bool ReadData(HANDLE hFile, char *szBuff, int nSize, unsigned long &lBytesRead);
bool WriteData(HANDLE hDestinationDrive, char *szBuffer, int lBytesToWrite, unsigned long &lBytesWritten);
void PrintTextCopyUpdate(char *szBuffer, int nSize, int lLoop, int lTotalBytesRead, int lTotalBytesWritten, int lBytesRead, int lBytesWritten);
void PrintTextCompareUpdate(char *szBuffer, int nSize, int lLoop, int lTotalSrcBytesRead, int lTotalDstBytesRead, int lSrcBytesRead, int lDstBytesRead, bool bVerified);
