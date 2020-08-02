//
// PIL - portable imaging library
// Copyright (c) 2000-2017 BitBank Software, Inc.
// Written by Larry Bank
// Project started 12/9/2000
// A highly optimized imaging library designed for resource-constrained
// environments such as mobile/embedded devices
//
// pil_io.c - functions for memory and file access
//
/****************************************************************************
 *                                                                          *
 * MODULE:  PIL_IO.C                                                        *
 *                                                                          *
 * DESCRIPTION: Generic IO module for Portable Imaging Library              *
 *                                                                          *
 * FUNCTIONS:                                                               *
 *            PILIOOpen - Open a file for reading or writing                *
 *            PILIOCreate - Create a file for writing                       *
 *            PILIOClose - Close a file                                     *
 *            PILIORead - Read a block of data from a file                  *
 *            PILIOWrite - write a block of data to a file                  *
 *            PILIOSeek - Seek to a specific section in a file              *
 *            PILIODate - Provide date and time in TIFF 6.0 format          *
 *            PILIOAlloc - Allocate a block of memory                       *
 *            PILIOFree - Free a block of memory                            *
 *            PILIOSignalThread - Send command to sub-thread                *
 *            PILIOMsgBox - Display a message box                           *
 * COMMENTS:                                                                *
 *            Created the module  12/9/2000  - Larry Bank                   *
 *            3/27/2008 added multithread support 3/27/2008                 *
 *            5/26/2012 added 16-byte alignment to alloc/free functions     *
 ****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <SD.h>

#include "pil.h"
#include "pil_io.h"
#define MAX_SIZE 0x400000 /* 4MB is good */
#define MAX_LIST 100
static int iTotalMem = 0;
static int iMemPtrs[MAX_LIST];
static int iMemSizes[MAX_LIST];
static int iMemCount = 0;

PILBOOL bTraceMem = FALSE;

// wrapper functions that need to exist in Objective-C

//extern void MyNSLog(const char *message);
//extern void * MyNSOpen(void * thename);
//extern void MyNSClose(void *handle);
//extern int MyNSRead(void *handle, unsigned char *pBuffer, int iLen);
//extern unsigned long MyNSSeek(void * handle, unsigned long lOffset, int iMethod);

//#define LOG_MEM

void TraceAdd(void * p, int size)
{
   iMemPtrs[iMemCount] = (int)(intptr_t)p;
   iMemSizes[iMemCount] = size;
   iTotalMem += size;
   iMemCount++;
   
} /* TraceAdd() */

void TraceRemove(void * p)
{
int i;
	for (i=0; i<iMemCount; i++)
	   {
	   if ((int)(intptr_t)p == iMemPtrs[i])
	      break;
	   }
	if (i < iMemCount)
	   {
	   iTotalMem -= iMemSizes[i]; // subtract from total
	   memcpy(&iMemSizes[i], &iMemSizes[i+1], (iMemCount-i)*sizeof(int));
	   memcpy(&iMemPtrs[i], &iMemPtrs[i+1], (iMemCount-i)*sizeof(int));
	   iMemCount--;
	   }
} /* TraceRemove() */


int PILIOCheckSum(char *pString)
{
int c = 0;

   while(*pString)
   {
      c += (char)*pString;
	  pString++;
   }
   return c;

} /* Checksum() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIODelete(char *)                                        *
 *                                                                          *
 *  PURPOSE    : Delete a file.                                             *
 *                                                                          *
 *  PARAMETERS : filename                                                   *
 *                                                                          *
 *  RETURNS    : 0 if successful, -1 if failure.                            *
 *                                                                          *
 ****************************************************************************/
int PILIODelete(TCHAR *szFile)
{
   return remove((char *)szFile);
} /* PILIODelete() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIORename(char *, char *)                                *
 *                                                                          *
 *  PURPOSE    : Rename a file.                                             *
 *                                                                          *
 *  PARAMETERS : src filename, dest filename                                *
 *                                                                          *
 *  RETURNS    : 0 if successful, -1 if failure.                            *
 *                                                                          *
 ****************************************************************************/
int PILIORename(TCHAR *szSrc, TCHAR *szDest)
{
#ifdef _WIN32
//   if (MoveFile(szSrc, szDest))
//      return 0;
//   else
//      return -1;
#endif
   return -1;
} /* PILIORename() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIOExists(char *)                                        *
 *                                                                          *
 *  PURPOSE    : Verify if a file exists or not.                            *
 *                                                                          *
 *  PARAMETERS : filename                                                   *
 *                                                                          *
 *  RETURNS    : PILBOOL - TRUE if exists, FALSE if not.                       *
 *                                                                          *
 ****************************************************************************/
PILBOOL PILIOExists(char *szName)
{
void *ihandle;

	ihandle = (void *)fopen((char *)szName, "rb");
//    ihandle = MyNSOpen(szName);

	if (ihandle == NULL)
	{
		return FALSE;
	}
    else
    {
        PILIOClose(ihandle);
//    	fclose((FILE *)ihandle);
    	return TRUE;
    }

} /* PILIOExists() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIOOpenRO(char *)                                        *
 *                                                                          *
 *  PURPOSE    : Opens a file for reading only.                             *
 *                                                                          *
 *  PARAMETERS : filename                                                   *
 *                                                                          *
 *  RETURNS    : Handle to file if successful, -1 if failure                *
 *                                                                          *
 ****************************************************************************/
void * PILIOOpenRO(void * fname)
{
   void * ihandle;

//    ihandle = MyNSOpen(fname);
   ihandle = (void *)fopen((char *)fname, "rb");
   if (ihandle == NULL)
      {
      return (void *)-1;
      }
   else
      return ihandle;

} /* PILIOOpenRO() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIOOpen(char *)                                          *
 *                                                                          *
 *  PURPOSE    : Opens a file for reading or writing                        *
 *                                                                          *
 *  PARAMETERS : filename                                                   *
 *                                                                          *
 *  RETURNS    : Handle to file if successful, -1 if failure                *
 *                                                                          *
 ****************************************************************************/
void * PILIOOpen(void * fname)
{
   void *ihandle;

//    ihandle = MyNSOpen(fname);
   ihandle = (void *)fopen((char *)fname, "r+b");
   if (ihandle == NULL)
      ihandle = PILIOOpenRO(fname); /* Try readonly */
   return ihandle;

} /* PILIOOpen() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIOCreate(char *)                                        *
 *                                                                          *
 *  PURPOSE    : Creates and opens a file for writing                       *
 *                                                                          *
 *  PARAMETERS : filename                                                   *
 *                                                                          *
 *  RETURNS    : Handle to file if successful, -1 if failure                *
 *                                                                          *
 ****************************************************************************/
void * PILIOCreate(TCHAR * fname)
{
void *ohandle;

   ohandle = (void *)fopen((char *)fname, "w+b");
   if (ohandle == 0) // NULL means failure
   {
#ifdef LOG_OUTPUT
	  __android_log_print(ANDROID_LOG_VERBOSE, "PILIOCreate", "Error = %d", errno);
#endif
      ohandle = (void *)-1;
   }
   return ohandle;

} /* PILIOCreate() */

unsigned long PILIOSize(void *iHandle)
{
unsigned long ulSize;
unsigned long ulStart;

    ulStart = ftell((FILE *)iHandle);
	fseek((FILE *)iHandle, 0L, SEEK_END);
	ulSize = ftell((FILE *)iHandle);
	fseek((FILE *)iHandle, ulStart, SEEK_SET);
//    ulSize = PILIOSeek(iHandle, 0, 2); // set to end to see the length
//    PILIOSeek(iHandle, 0, 0); // reset to start
    return ulSize;
   
} /* PILIOSize() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIOSeek(int, signed long, int)                           *
 *                                                                          *
 *  PURPOSE    : Seeks within an open file                                  *
 *                                                                          *
 *  PARAMETERS : File Handle                                                *
 *               Offset                                                     *
 *               Method - 0=from beginning, 1=from current spot, 2=from end *
 *                                                                          *
 *  RETURNS    : New offset within file.                                    *
 *                                                                          *
 ****************************************************************************/
unsigned long PILIOSeek(void * iHandle, unsigned long lOffset, int iMethod)
{
	   int iType;
#ifdef _WIN32
	   fpos_t ulNewPos;
#else
	   long ulNewPos;
#endif

	   if (iMethod == 0) iType = SEEK_SET;
	   else if (iMethod == 1) iType = SEEK_CUR;
	   else iType = SEEK_END;

	   fseek((FILE *)iHandle, lOffset, iType);
	   ulNewPos = fgetpos((FILE *)iHandle, &ulNewPos);
//       ulNewPos = MyNSSeek(iHandle, lOffset, iMethod);
	   return (unsigned long)ulNewPos;

} /* PILIOSeek() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIORead(int, void *, int)                                *
 *                                                                          *
 *  PURPOSE    : Read a block from an open file                             *
 *                                                                          *
 *  PARAMETERS : File Handle                                                *
 *               Buffer pointer                                             *
 *               Number of bytes to read                                    *
 *                                                                          *
 *  RETURNS    : Number of bytes read                                       *
 *                                                                          *
 ****************************************************************************/
signed int PILIORead(void * iHandle, void * lpBuff, unsigned int iNumBytes)
{
	unsigned int iBytes;

   iBytes = (int)fread(lpBuff, 1, iNumBytes, (FILE *)iHandle);
//    iBytes = MyNSRead(iHandle, lpBuff, iNumBytes);
	return iBytes;

} /* PILIORead() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIOWrite(int, void *, int)                               *
 *                                                                          *
 *  PURPOSE    : Write a block from an open file                            *
 *                                                                          *
 *  PARAMETERS : File Handle                                                *
 *               Buffer pointer                                             *
 *               Number of bytes to write                                   *
 *                                                                          *
 *  RETURNS    : Number of bytes written                                    *
 *                                                                          *
 ****************************************************************************/
unsigned int PILIOWrite(void * iHandle, void * lpBuff, unsigned int iNumBytes)
{
	   unsigned int iBytes;

	   iBytes = (int)fwrite(lpBuff, 1, iNumBytes, (FILE *)iHandle);
	   return iBytes;

} /* PILIOWrite() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIOClose(int)                                            *
 *                                                                          *
 *  PURPOSE    : Close a file                                               *
 *                                                                          *
 *  PARAMETERS : File Handle                                                *
 *                                                                          *
 *  RETURNS    : NOTHING                                                    *
 *                                                                          *
 ****************************************************************************/
void PILIOClose(void * iHandle)
{
//    MyNSClose(iHandle);
	   fflush((FILE *)iHandle);
	   fclose((FILE *)iHandle);

} /* PILIOClose() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIOReAlloc(void *, unsigned long)                        *
 *                                                                          *
 *  PURPOSE    : Re-Allocate a block of writable memory to a new size.      *
 *                                                                          *
 ****************************************************************************/
void * PILIOReAlloc(void *p, unsigned long size)
{
    return realloc(p, size);
} /* PILIOReAlloc() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIOAlloc(long)                                           *
 *                                                                          *
 *  PURPOSE    : Allocate a block of writable memory.                       *
 *                                                                          *
 ****************************************************************************/
void * PILIOAlloc(unsigned long size)
{
	void *p = NULL;
//    void* i;
//    int j;

	   if (size == 0)
          {
          return NULL; // Linux seems to return a non-NULL pointer for 0 size
          }

//	   i = (void *)malloc(size+16);
	   p = (void *)malloc(size+16);
	   if (p)//(i)
	      {
//		  j = 16-((int)(intptr_t)i & 0xf); // make it 16-byte aligned
//		  memset((void *)i, j, j);
//		  p = (void *)(i+j);
	      memset(p, 0, size);
	      }
//	   if (bTraceMem)
//	   {
//	        TraceAdd((void *)p, size);
//	   }
#ifdef LOG_MEM
    {
        char szTemp[256];
        sprintf(szTemp, "PILIOAlloc: size = 0x%x, ptr=0x%016llx", (unsigned int)size, (unsigned long long)(intptr_t)p);
        MyNSLog(szTemp);
    }
#endif // LOG_MEM
	   return p;
   
} /* PILIOAlloc() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIOAllocNoClear(long)                                    *
 *                                                                          *
 *  PURPOSE    : Allocate a block of writable memory, no zero-fill.         *
 *                                                                          *
 ****************************************************************************/
void * PILIOAllocNoClear(unsigned long size)
{
	void *p = NULL;
//    void* i;
//    int j;

	if (size == 0)
       {
       return NULL; // Linux seems to return a non-NULL pointer for 0 size
       }
    p = (void *)malloc(size+16);
//	i = (void *)malloc(size+16);
//    if (i)
//    {
//       j = 16-((int)(intptr_t)i & 0xf); // make it 16-byte aligned
//       memset((void *)i, j, j);
//       p = (void *)(i+j);
//    }
#ifdef LOG_MEM
    {
        char szTemp[256];
        sprintf(szTemp, "PILIOAllocNoClear: size = 0x%x, ptr=%08x", (unsigned int)size, (int)p);
        MyNSLog(szTemp);
    }
#endif // LOG_MEM
	return p;
   
} /* PILIOAllocNoClear() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIOAllocOutbuf(long)                                     *
 *                                                                          *
 *  PURPOSE    : Allocate a block of writable memory.                       *
 *                                                                          *
 ****************************************************************************/
void * PILIOAllocOutbuf(void)
{
void *p = NULL;
//void * i;
//int j;

//   i = (void *)malloc(MAX_SIZE+16);
   p = (void *)malloc(MAX_SIZE+16);
   if (p)//(i)
   {
//      j = 16-((int)(intptr_t)i & 0xf); // make it 16-byte aligned
//      memset((void *)i, j, j);
//      p = (void *)(i+j);
	  memset(p, 0, MAX_SIZE);
	  }
#ifdef LOG_MEM
    {
        char szTemp[256];
        sprintf(szTemp, "PILIOAllocOutBuf: size = 0x%x, ptr=%08x", (unsigned int)(MAX_SIZE+16), (int)p);
        MyNSLog(szTemp);
    }
#endif // LOG_MEM
   return p;
} /* PILIOAllocOutbuf() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIOFree(void *)                                          *
 *                                                                          *
 *  PURPOSE    : Free a block of writable memory.                           *
 *                                                                          *
 ****************************************************************************/
void PILIOFree(void *p)
{
//	unsigned char *puc = (unsigned char *)p;
//	unsigned char ucOff;

	   if (bTraceMem)
	   {
	        TraceRemove((void *)p);
#ifdef LOG_MEM
           {
               char szTemp[256];
               sprintf(szTemp, "PILIOFree: ptr = %08x, total size = 0x%x", (int)(intptr_t)p, iTotalMem);
               MyNSLog(szTemp);
           }
#endif
	   }
    if (p == NULL || p == (void *)-1)
       return; /* Don't try to free bogus pointer */
//	ucOff = puc[-1]; // how many bytes do we have to adjust to point to start of real buffer?
//	if (ucOff > 0 && ucOff <= 16)
//	   puc = &puc[-ucOff];
#ifdef LOG_MEM
    {
        char szTemp[256];
        sprintf(szTemp, "PILIOFree: ptr = 0x%016llx", (unsigned long long)(intptr_t)p);
        MyNSLog(szTemp);
    }
#endif // LOG_MEM
//	free((void *)puc);
    free(p);
} /* PILIOFree() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILIOFreeOutbuf(void *)                                    *
 *                                                                          *
 *  PURPOSE    : Free a block of writable memory.                           *
 *                                                                          *
 ****************************************************************************/
void PILIOFreeOutbuf(void *p)
{
//	unsigned char *puc = (unsigned char *)p;
//	unsigned char ucOff;

	if (p == NULL || p == (void *)-1)
      return; /* Don't try to free bogus pointer */
//	ucOff = puc[-1]; // how many bytes do we have to adjust to point to start of real buffer?
//	if (ucOff > 0 && ucOff <= 16)
//	   puc = &puc[-ucOff];
#ifdef LOG_MEM
    {
        char szTemp[256];
        sprintf(szTemp, "PILIOFreeOutbuf: ptr = %016llx", (unsigned long long)(intptr_t)p);
        MyNSLog(szTemp);
    }
#endif // LOG_MEM
//    free((void *)puc);
   free((void *)p);
} /* PILIOFree() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILAssertHandlerProc(void *)                               *
 *                                                                          *
 *  PURPOSE    : Handle an assertion in a platform specific fashion.        *
 *                                                                          *
 ****************************************************************************/

void PILAssertHandlerProc(char *pExpression, char *pFile, unsigned long int ulLineNumber)
{
//	AssertHandlerProc((UINT8 *) pExpression, (UINT8 *) pFile, (UINT32) ulLineNumber);
}
