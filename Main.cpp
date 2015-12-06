#include <cstdio>
#include <string>
#include <windows.h>

#include "Main.h"

using std::string;

int main(int argc, char *argv[]) 
{
   printf("\n%s Version 0.99b For Windows.\n", argv[0]);
   if (CheckCommandLineArguments(argc, argv) == false)
   {
      return 1;
   }

   char szSrcVolName[51];

#if defined(WIN_XP) || defined(WIN_7)
   FormatSourceDestinationWinXP7(szSrcVolName, argv[1]);
   printf("Source Volume- %s\n", szSrcVolName);
#endif

#if defined(WIN_VISTA)
   if (FormatSourceDestinationVista(szSrcVolName, argv[1]) == true)
   {
      printf("Opened Source Mount Point %s As Volume %s\n", argv[1], szSrcVolName);
   }
   else
   {
      return 1;
   }
#endif

   char szDstVolName[51];

#if defined(WIN_XP) || defined(WIN_7)
   FormatSourceDestinationWinXP7(szDstVolName, argv[2]);
   printf("Destination Volume- %s\n", szDstVolName);
#endif

#if defined(WIN_VISTA)
   if (FormatSourceDestinationVista(szDstVolName, argv[2]) == true)
   {
      printf("Opened Destination Mount Point %s As Volume %s\n", argv[2], szDstVolName);
   }
   else
   {
      return 1;
   }
#endif
   
   HANDLE hSourceDrive = INVALID_HANDLE_VALUE;
   if (CreateSourceFile(szSrcVolName, hSourceDrive) == false)
   {
      return 1;
   }
   HANDLE hDestinationDrive = INVALID_HANDLE_VALUE;
   if (CreateDestinationFile(szDstVolName, hDestinationDrive, hSourceDrive) == false)
   {
      return 1;
   }

#if defined(WIN_7)
   if (Win7SpecificIOCtl(hSourceDrive, hDestinationDrive) == false)
   {
      return 1;
   }
#endif

   long lTotalBytesRead = 0;
   long lTotalBytesWritten = 0;
   long lLoopCount = 0;
   char szOutString[512];
   memset(szOutString, 0, sizeof(char) * 512);
   static const int nBufferSize = 262144;
   char szBuffer[nBufferSize];
   memset(szBuffer, 0, sizeof(char) * nBufferSize);

   do 
   {
      unsigned long lNumBytesRead = 0;
      if (ReadData(hSourceDrive, szBuffer, sizeof(char) * nBufferSize, lNumBytesRead) == false)
      {
         break;
      }
      lTotalBytesRead += lNumBytesRead;
      
      unsigned long lNumBytesWritten = 0;
      if (WriteData(hDestinationDrive, szBuffer, lNumBytesRead, lNumBytesWritten) == false)
      {
         break;
      }
      lTotalBytesWritten += lNumBytesWritten;

      lLoopCount++;
      
      PrintTextCopyUpdate(szOutString, sizeof(char) * 512, lLoopCount, lTotalBytesRead, lTotalBytesWritten, lNumBytesRead, lNumBytesWritten);

      if (lNumBytesRead != lNumBytesWritten) 
      {
         printf("\nMismatch In Read Bytes And Write Bytes, Operation Failed\n");
         break;
      }
   }
   while (true);

   printf("\nDone Reading And Writing Files.\n");

   if (CloseFiles(hSourceDrive, hDestinationDrive) == false)
   {
      return 1;
   }

   printf("Verifying Written data.\n");

   HANDLE hSourceDisk = INVALID_HANDLE_VALUE;
   if (CreateSourceFile(szSrcVolName, hSourceDisk) == false)
   {
      return 1;
   }

   HANDLE hDestinationDisk = INVALID_HANDLE_VALUE;
   if (CreateSourceFile(szDstVolName, hDestinationDisk) == false)
   {
      return 1;
   }

   char szSrcBuff[nBufferSize];
   char szDstBuff[nBufferSize];
   unsigned long ulTotalSrcBytesRead = 0;
   unsigned long ulTotalDstBytesRead = 0;
   lLoopCount = 0;
   do
   {
      unsigned long lSrcBytesRead = 0;
      if (ReadData(hSourceDisk, szSrcBuff, nBufferSize, lSrcBytesRead) == false)
      {
         break;
      }
      ulTotalSrcBytesRead += lSrcBytesRead;

      unsigned long lDstBytesRead = 0;
      if (ReadData(hDestinationDisk, szDstBuff, nBufferSize, lDstBytesRead) == false)
      {
         break;
      }
      ulTotalDstBytesRead += lDstBytesRead;

      bool bVerified = (memcmp(szSrcBuff, szDstBuff, nBufferSize) == 0);

      lLoopCount++;
      PrintTextCompareUpdate(szOutString, sizeof(char) * 512, lLoopCount, ulTotalSrcBytesRead, ulTotalDstBytesRead, lSrcBytesRead, lDstBytesRead, bVerified);

      if (bVerified == false)
      {
         printf("\nVerification Failed Between Source And Destination.\n");
         break;
      }
   }
   while (true);

   printf("\nDone Verifying Files.\n");

   if (CloseFiles(hSourceDrive, hDestinationDrive) == false)
   {
      return 1;
   }
   
   return 0;
}

void PrintTextCompareUpdate(char *szBuffer, int nSize, int lLoop, int lTotalSrcBytesRead, int lTotalDstBytesRead, int lSrcBytesRead, int lDstBytesRead, bool bVerified)
{
   for (unsigned int i=0; i<strlen(szBuffer); i++) 
   {
      printf("\b");
   }
   sprintf_s(szBuffer, nSize, "BlkCnt:%d TotSrcRd:%d, TotDstRd:%d, RdSrc:%d, RdDst:%d, Verified:%d", 
      lLoop, lTotalSrcBytesRead, lTotalDstBytesRead, lSrcBytesRead, lDstBytesRead, (bVerified == true) ? 1 : 0);
   printf("%s", szBuffer);
}

void PrintTextCopyUpdate(char *szBuffer, int nSize, int lLoop, int lTotalBytesRead, int lTotalBytesWritten, int lBytesRead, int lBytesWritten)
{
   for (unsigned int i=0; i<strlen(szBuffer); i++) 
   {
      printf("\b");
   }
   sprintf_s(szBuffer, nSize, "BlkCnt:%d TotRd:%d, TotWr:%d, Rd:%d, Wr:%d", lLoop, lTotalBytesRead, lTotalBytesWritten, lBytesRead, lBytesWritten);
   printf("%s", szBuffer);
}

bool WriteData(HANDLE hDestinationDrive, char *szBuffer, int lBytesToWrite, unsigned long &lBytesWritten)
{
   bool bRet = true;
   BOOL bWriteRes = WriteFile(hDestinationDrive, szBuffer, lBytesToWrite, &lBytesWritten, NULL);
   if ((bWriteRes != 0) && (lBytesWritten != lBytesToWrite))   
   {
      printf("WriteFile() Destination Failed With %d\n", GetLastError());
      bRet = false;
   }
   if (bWriteRes == 0)
   {
      printf("WriteFile() Destination Write Error: %d\n", GetLastError());
      bRet = false;
   }
   return bRet;
}

bool ReadData(HANDLE hFile, char *szBuff, int nSize, unsigned long &lBytesRead)
{
   bool bRet = true;
   memset(szBuff, 0, sizeof(char) * nSize);
   BOOL bReadRes = ReadFile(hFile, szBuff, nSize, &lBytesRead, NULL);
   if ((bReadRes != 0) && (lBytesRead == 0)) 
   {
      bRet = false;
   }
   if (bReadRes == 0)
   {
      printf("ReadFile() Source Read Error: %d\n", GetLastError());
      bRet = false;
   }
   return bRet;
}

bool CloseFiles(HANDLE hSrcHandle, HANDLE hDstHandle)
{
   bool bRet = true;
   BOOL bCloseRet = CloseHandle(hSrcHandle);
   if (bCloseRet == 0)  
   {
      printf("CloseHandle() Source Failed With %d\n", GetLastError());
      bCloseRet = CloseHandle(hDstHandle);
      if (bCloseRet == 0)  
      {
         printf("CloseHandle() Destination Failed With %d\n", GetLastError());
         bRet = false;
      }
      bRet = false;
   }
   return bRet;
}

bool Win7SpecificIOCtl(HANDLE hSrcHandle, HANDLE hDstHandle)
{
   bool bRet = true;
   DWORD dwBytesReturned = 0;
   BOOL bRetIOCtl = DeviceIoControl(hDstHandle, (DWORD)FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, (LPDWORD)&dwBytesReturned, NULL);
   if (bRetIOCtl == 0)
   {
      printf("DeviceIoControl(FSCTL_LOCK_VOLUME) Destination Failed With %d\n", GetLastError());
      BOOL bCloseRet = CloseHandle(hSrcHandle);
      if (bCloseRet == 0)
      {
         printf("CloseHandle() Source Failed With %d\n", GetLastError());
      }
      bCloseRet = CloseHandle(hDstHandle);
      if (bCloseRet == 0)
      {
         printf("CloseHandle() Destination Failed With %d\n", GetLastError());
      }
      bRet = false;
   }
   return bRet;
}

bool CreateSourceFile(char szName[51], HANDLE &hHandle)
{
   bool bRet = true;
   hHandle = CreateFile(szName, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL);
   if (hHandle == INVALID_HANDLE_VALUE) 
   {
      printf("CreateFile() Source Failed With %d\n", GetLastError());
      bRet = false;
   }
   return bRet;
}

bool CreateDestinationFile(char szName[51], HANDLE &hHandle, HANDLE &hSrcHandle)
{
   bool bRet = true;
   hHandle = CreateFile(szName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL);
   if (hHandle == INVALID_HANDLE_VALUE)  
   {
      printf("CreateFile() Destination Failed With %d\n", GetLastError());
      BOOL bCloseRet = CloseHandle(hSrcHandle);
      if (bCloseRet != 0)  
      {
         printf("CloseHandle() Source Failed With %d\n", GetLastError());
         bRet = false;
      }
      bRet = false;
   }
   return bRet;
}

bool FormatSourceDestinationVista(char szName[51], char *argv)
{
   bool bRet = true;
   memset(szName, 0, sizeof(char) * 51);

   BOOL bVolumeRet = GetVolumeNameForVolumeMountPoint(argv, szName, sizeof(char) * 51);
   if (bVolumeRet == FALSE)   
   {
      printf("GetVolumeNameForVolumeMountPoint() failed with %d\n", GetLastError());
      bRet = false;
   }
   return bRet;
}

void FormatSourceDestinationWinXP7(char szName[51], char *argv)
{
   sprintf_s(szName, sizeof(szName), "\\\\.\\%s:", argv);
}

bool CheckCommandLineArguments(int argc, char *argv[])
{
   bool bRet = true;
   if (argc != 3) 
   {
      printf("Invalid Command Line Parameters: %s SourceDriveLetter TargetDriveLetter\n", argv[0]);
      printf("Example: %s C D \n", argv[0]);
      bRet = false;
   }
   if (strlen(argv[1]) > 1)   
   {
      printf("Invalid Command Line Parameters: %s SOURCE_DRIVE_LETTER TargetDriveLetter\n", argv[0]);
      printf("Example: %s C D \n", argv[0]);
      bRet = false;
   }
   if (strlen(argv[2]) > 1)   
   {
      printf("Invalid Command Line Parameters: %s SourceDriveLetter TARGET_DRIVE_LETTER\n", argv[0]);
      printf("Example: %s C D \n", argv[0]);
      bRet = false;
   }
   return bRet;
}