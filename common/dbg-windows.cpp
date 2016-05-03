/*
   Copyright 2010-2016 Intel Corporation

   This software is licensed to you in accordance
   with the agreement between you and Intel Corporation.

   Alternatively, you can use this file in compliance
   with the Apache license, Version 2.


   Apache License, Version 2.0

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

/**                                                                            
********************************************************************************
**
**    @file dbg.c
**
**    @brief  Debug functions
**
**    @author Niveditha Sundaram
**	  @author Venky Gokulrangan		  
**
********************************************************************************
*/
#include <stdio.h>
#include <windows.h>
#include "dbg.h"

void PrintTime(FILE* fp, char *s)
{
#ifdef DEBUG
	SYSTEMTIME st;
	GetSystemTime(&st);

	if( !fp ) fp=stdout;
	fprintf(fp, " %20s --> %d:%d:%d:%03d\n" ,s, st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
#endif
}



// Debug Related

void PRINT_PARA( UINT8* pBuf, int bufLen )
{
#ifdef DEBUG
	//   char     Buffer [8192] ;
	//   UINT8    *p = Buffer;

	//   int i, k=0;
	//int buflen = 8192;

	//memset(Buffer, 0, buflen);

	//   for(i=0; i<len; i++ ) {
	//       k = sprintf_s( p, buflen, "%02x ", b[i] ) ;
	//       p += k ;
	//       if( ((i+1) % 16) == 0 ){
	//           TRACE0(Buffer) ;
	//           p=Buffer; // go back to the original place
	//       }
	//   }
	//   if( (len%16) != 0 ) 
	//   {
	//       TRACE0(Buffer) ;
	//   }

	//	if (bufLen <= 0)
	//	return;

	UINT8 outBuf[8193];
	int i,outBufLen = 8192, cnt = 0;

	memset(outBuf, 0, outBufLen);

	for (i=0; i<bufLen; i++)
	{
		if (cnt > outBufLen-10)
			break;
		sprintf_s((char *)outBuf+cnt, 
			outBufLen-cnt, 
			"%02x ", 
			pBuf[i]);
		cnt+=3;
	}
	outBuf[8192] = '\0';
	TRACE0((char *)outBuf); // This is unsafe! if the buffer doesnt end with NULL we will get execption.
	TRACE0("\n");
	return;

	//    TRACE0("\n") ;
#endif
}


//UINT32
//JHI_DbgPrint(
//    char*  Format, 
//    ...
//)
//{
//   UINT32    dwChars = 0;
//#ifdef DEBUG
//   char     Buffer [8192] ;
//   va_list  args ;
//
//   va_start ( args, Format ) ;
//   dwChars = vsprintf ( Buffer, Format, args ) ;
//   va_end (args) ;
//
//   //fprintf (stderr, "%s", Buffer ) ;
//   OutputDebugStringA ( Buffer) ;
//
//#endif
//   return dwChars ;
//}

UINT32 JHI_Log(
	const char*  Format,
	... )
{
	UINT32       dwChars=0;
	char     Buffer [8192] ;
	int buflen = 8192;
	va_list  args ;

	va_start ( args, Format ) ;
	dwChars = vsprintf_s ( Buffer, buflen, Format, args ) ;
	va_end (args) ;

	OutputDebugStringA ( Buffer) ;

	return dwChars ;
}

UINT32 JHI_Trace(
	const char*  Format,
	... )
{
	UINT32       dwChars=0;
#ifdef DEBUG

	char     Buffer [1024] ;
	int buflen = sizeof(Buffer);
	va_list  args ;
	va_start ( args, Format ) ;

#ifdef TRACER_NAME
	size_t tracerNameLen = 0;
	tracerNameLen = strlen(TRACER_NAME);
	strcpy_s (Buffer, tracerNameLen + 1, TRACER_NAME) ;
	dwChars = vsprintf_s ( Buffer + tracerNameLen, buflen - tracerNameLen, Format, args ) ;
#else
	dwChars = vsprintf_s ( Buffer, buflen, Format, args ) ;
#endif
	va_end (args) ;

	//fprintf (stderr, "%s", Buffer ) ;
	OutputDebugStringA ( Buffer) ;
#endif

	return dwChars ;
}

UINT32 JHI_T_Trace(const TCHAR* pFormat, ...)
{
	UINT32       dwChars=0;
#ifdef DEBUG
	TCHAR       chMsg[1024];
	int msglen = sizeof(chMsg) / sizeof(TCHAR);
	va_list     pArg;

	va_start(pArg, pFormat);
	_vstprintf_s(chMsg, msglen, pFormat, pArg);
	va_end(pArg);

	OutputDebugString(chMsg) ;
#endif
	return dwChars ;
}
