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

char* JHIErrorToString(UINT32 retVal)
{
	char* str = "";
	switch (retVal)
	{
	//case 0x101: 		str = "JHI_FILE_MISSING_SRC";					break;		// Source File not found in install/uninstall or unable to load
		// file in SendAndRecv
	case 0x102: 		str = "JHI_FILE_ERROR_AUTH";					break;		// Attempted to load the file, but FW returned back 
		// a manifest failure check and rejected it
	//case 0x104: 		str = "JHI_FILE_ERROR_DELETE";					break;		// Unable to remove file corresponding to the UUID in uninstall
		// Maybe permission issues
	case 0x105: 		str = "JHI_FILE_INVALID";						break;		// Invalid file - bad characters or larger than 64K
	case 0x106: 		str = "JHI_FILE_ERROR_OPEN";					break;		// Unable to open file. Maybe permission issues
	case 0x107: 		str = "JHI_FILE_UUID_MISMATCH";					break;		// UUIDs dont match between applet file and function input
	case 0x108: 		str = "JHI_FILE_IDENTICAL";						break;		// downloaded applet matches existing one in Jom

	case 0x202: 		str = "JHI_INVALID_COMMAND";					break;		// invalid JHI interface command
	//case 0x204: 		str = "JHI_ILLEGAL_VALUE";						break;		// validation failed on input parameters

	//case 0x300: 		str = "JHI_COMMS_ERROR";						break;		// Communications error due to HECI timeouts
		// or ME auto resets or any other COMMS error
	case 0x302: 		str = "JHI_SERVICE_INVALID_GUID";				break;		// Invalid COM guid (from DLL)

	case 0x401: 		str = "JHI_APPLET_TIMEOUT";						break;		// This may be a result of a Java code in VM in an infinite loop. 
		// TL will kill applet in JOM and return error code

	//case 0x402: 		str = "JHI_APPID_NOT_EXIST";					break;		// If appid is not present in app table
	case 0x403: 		str = "JHI_JOM_FATAL";							break;		//JOM fatal error
	//case 0x404: 		str = "JHI_JOM_OVERFLOW";						break;		//exceeds max installed applets or active sessions in JOM
	case 0x405: 		str = "JHI_JOM_ERROR_DOWNLOAD";					break;		//JOM download error
	case 0x406: 		str = "JHI_JOM_ERROR_UNLOAD";					break;		//JOM unload error

	case 0x500: 		str = "JHI_ERROR_LOGGING";						break;		// Error in logging

	case 0x600: 		str = "JHI_UNKNOWN_ERROR";						break;		// Any other error

		//----------------------------------------------------------------------------------------------------------------
		// JHI 8.0 return codes
		//----------------------------------------------------------------------------------------------------------------

		// General JHI Return Code
	case 0x00: 			str = "JHI_SUCCESS";							break;		// general success response
	case 0x201: 		str = "JHI_INVALID_HANDLE";						break;		// invalid JHI handle
	case 0x203: 		str = "JHI_INVALID_PARAMS";						break;		// passed a null pointer to a required argument / illegal arguments passed to API function
	case 0x204:			str = "JHI_INVALID_APPLET_GUID";				break;		// the applet UUID is invalid
	case 0x301: 		str = "JHI_SERVICE_UNAVAILABLE";				break;		// there is no connection to JHI service
	case 0x501: 		str = "JHI_ERROR_REGISTRY";						break;		// error for any registry based access or registry corruption
	case 0x1000: 		str = "JHI_ERROR_REPOSITORY_NOT_FOUND";			break;		// when cannot find applets repository directory
	case 0x601: 		str = "JHI_INTERNAL_ERROR";						break;		// an unexpected internal error happened.
	case 0x1001: 		str = "JHI_INVALID_BUFFER_SIZE";				break;		// used a buffer that is larger than JHI_BUFFER_MAX
	case 0x1002: 		str = "JHI_INVALID_COMM_BUFFER";				break;		// JVM_COMM_BUFFER passed to function is invalid

		// Install errors
	case 0x1003:		str = "JHI_INVALID_INSTALL_FILE";				break;		// the dalp file path is invalid
	case 0x1004:		str = "JHI_READ_FROM_FILE_FAILED";				break;		// failed to read DALP file
	case 0x1005:		str = "JHI_INVALID_PACKAGE_FORMAT";				break;		// dalp file format is not a valid
	case 0x103:			str = "JHI_FILE_ERROR_COPY";					break;		// applet file could not be copied to repository
	case 0x1006:		str = "JHI_INVALID_INIT_BUFFER";				break;		// passed an invalid init buffer to the function
	case 0x101:			str = "JHI_FILE_NOT_FOUND";						break;		// could not find the specified dalp file
	case 0x1007:		str = "JHI_INVALID_FILE_EXTENSION";				break;		// applets package file must end with .dalp extension.
	case 0x404:			str = "JHI_MAX_INSTALLED_APPLETS_REACHED";		break;		// exceeds max applets allowed, need to uninstall an applet.
	case 0x1008:		str = "JHI_INSTALL_FAILURE_SESSIONS_EXISTS";	break;		// could not install because there are open sessions.
	case 0x1009:		str = "JHI_INSTALL_FAILED";						break;		// no compatible applet was found in the DALP file 

		// Uninstall errors
	case 0x104:			str = "JHI_DELETE_FROM_REPOSITORY_FAILURE";		break;		// unable to delete applet DALP file from repository
	case 0x100A:		str = "JHI_UNINSTALL_FAILURE_SESSIONS_EXISTS";	break;		// for app uninstallation errors

		// Create Session errors
	case 0x402:			str = "JHI_APPLET_NOT_INSTALLED";				break;		// trying to create a session of uninstalled applet
	case 0x100C: 		str = "JHI_MAX_SESSIONS_REACHED";				break;		// exceeds max sessions allowed, need to close a session.
	case 0x100D: 		str = "JHI_SHARED_SESSION_NOT_SUPPORTED";		break;		// the applet does not support shared sessions.
	case 0x100E: 		str = "JHI_MAX_SHARED_SESSION_REACHED";			break;		// failed to get session handle due to maximun handles limit.
	case 0x1018:        str = "JHI_FIRMWARE_OUT_OF_RESOURCES";			break;		// request causes the VM to exceed its memory quota
	case 0x1019:        str = "JHI_ONLY_SINGLE_INSTANCE_ALLOWED";		break;		// trying to create more than a single instance of an applet

		// Close Session errors
	case 0x100F: 		str = "JHI_INVALID_SESSION_HANDLE";				break;		// the session handle is not of an active session.

		// Send And Recieve errors
	case 0x200: 		str = "JHI_INSUFFICIENT_BUFFER";				break;		// buffer overflow - response greater than supplied Rx buffer
	case 0x400: 		str = "JHI_APPLET_FATAL";						break;		// This may be a result of uncaught exception or unusual applet 
		// error that results in applet being terminated by TL VM. 
		// Register/Unregister session events
	case 0x1010: 		str = "JHI_SESSION_NOT_REGISTERED";				break;		// trying to unregister a session that is not registered for events.
	case 0x1011: 		str = "JHI_SESSION_ALREADY_REGSITERED";			break;		// Registration to an event is done only once. 
	case 0x1012: 		str = "JHI_EVENTS_NOT_SUPPORTED";				break;		// events are not supported for this type of session

		// Get Applet Property errors
	case 0x1013: 		str = "JHI_APPLET_PROPERTY_NOT_SUPPORTED";		break;		// Rerturned when calling GetAppletProperty with invalid property 

		// Init errors
	case 0x1014: 		str = "JHI_SPOOLER_NOT_FOUND";					break;		// cannot find the spooler file
	case 0x1015: 		str = "JHI_INVALID_SPOOLER";					break;		// cannot download spooler / create an instance of the spooler 
	case 0x300:			str = "JHI_NO_CONNECTION_TO_FIRMWARE";			break;		// JHI has no connection to the VM

		// DLL errors
	case 0x1016:		str = "JHI_VM_DLL_FILE_NOT_FOUND";				break;		// VM DLL is missing from the exe path
	case 0x1017:		str = "JHI_VM_DLL_VERIFY_FAILED";				break;		// DLL Signature or Publisher name are not valid.

	default: str = "JHI_UNKNOWN_ERROR";
	}
	return str;
}

char* TEEErrorToString(UINT32 retVal)
{
	char* str = "";

	switch (retVal)
	{
	// General errors
	case 0x0000: str = "TEE_STATUS_SUCCESS";						break;
	case 0x2001: str = "TEE_STATUS_INTERNAL_ERROR";					break;
	case 0x2002: str = "TEE_STATUS_INVALID_PARAMS";					break;
	case 0x2003: str = "TEE_STATUS_INVALID_HANDLE";					break;
	case 0x2004: str = "TEE_STATUS_INVALID_UUID";					break;
	case 0x2005: str = "TEE_STATUS_NO_FW_CONNECTION";				break;
	case 0x2006: str = "TEE_STATUS_UNSUPPORTED_PLATFORM";			break;
	
	// Service errors
	case 0x2100: str = "TEE_STATUS_SERVICE_UNAVAILABLE";			break;
	case 0x2101: str = "TEE_STATUS_REGISTRY_ERROR";					break;
	case 0x2102: str = "TEE_STATUS_REPOSITORY_ERROR";				break;
	case 0x2103: str = "TEE_STATUS_SPOOLER_MISSING";				break;
	case 0x2104: str = "TEE_STATUS_SPOOLER_INVALID";				break;
	case 0x2105: str = "TEE_STATUS_PLUGIN_MISSING";					break;
	case 0x2106: str = "TEE_STATUS_PLUGIN_VERIFY_FAILED";			break;
	
	// Package errors
	case 0x2200: str = "TEE_STATUS_INVALID_PACKAGE";				break;
	case 0x2201: str = "TEE_STATUS_INVALID_SIGNATURE";				break;
	case 0x2202: str = "TEE_STATUS_MAX_SVL_RECORDS";				break;
	
	// Install / uninstall TA errors:
	case 0x2300: str = "TEE_STATUS_CMD_FAILURE_SESSIONS_EXISTS";	break;
	case 0x2301: str = "TEE_STATUS_CMD_FAILURE";					break;
	case 0x2302: str = "TEE_STATUS_MAX_TAS_REACHED";				break;
	case 0x2303: str = "TEE_STATUS_MISSING_ACCESS_CONTROL";			break;
	case 0x2304: str = "TEE_STATUS_TA_DOES_NOT_EXIST";				break;
	case 0x2305: str = "TEE_STATUS_INVALID_TA_SVN";					break;
	case 0x2306: str = "TEE_STATUS_IDENTICAL_PACKAGE";				break;
	
	default: str = "TEE_UNKNOWN_ERROR";
	}

	return str;
}