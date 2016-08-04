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

#ifndef __DBG_H__
#define __DBG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "typedefs.h"

#ifdef _WIN32
#include <tchar.h>
#else
#include <unistd.h>
#include <sys/syscall.h>
#endif

//#define LOG						JHI_Log(fmt, ); 

UINT32
JHI_Log(
     const char*  Format,
	 ... );


//Codes for LOGGING
#define JHIDLL_INIT_ENTER									0x1000
#define JHIDLL_INIT_EXIT									0x1001
#define	JHISVC_INIT_ENTER									0x1010
#define	JHISVC_INIT_EXIT									0x1011

#define JHIDLL_DEINIT_ENTER									0x2000
#define JHIDLL_DEINIT_EXIT									0x2001
#define JHISVC_DEINIT_ENTER									0x2010
#define JHISVC_DEINIT_EXIT									0x2011

#define JHIDLL_INSTALL_ENTER								0x3000
#define JHIDLL_INSTALL_EXIT									0x3001
#define JHISVC_INSTALL_ENTER								0x3010
#define JHISVC_INSTALL_EXIT									0x3011

#define JHIDLL_UNINSTALL_ENTER								0x4000
#define JHIDLL_UNINSTALL_EXIT								0x4001
#define JHISVC_UNINSTALL_ENTER								0x4010
#define JHISVC_UNINSTALL_EXIT								0x4011

#define JHIDLL_SENDANDRECV_ENTER							0x5000
#define JHIDLL_SENDANDRECV_EXIT								0x5001
#define JHISVC_SENDANDRECV_ENTER							0x5010
#define JHISVC_SENDANDRECV_EXIT								0x5011

#define JHIDLL_GETAPPLETPROPERTY_ENTER						0x6000
#define JHIDLL_GETAPPLETPROPERTY_EXIT						0x6001
#define JHISVC_GETAPPLETPROPERTY_ENTER						0x6010
#define JHISVC_GETAPPLETPROPERTY_EXIT						0x6011

#define JHIDLL_CREATESESSION_ENTER							0x7000
#define JHIDLL_CREATESESSION_EXIT							0x7001
#define JHISVC_CREATESESSION_ENTER							0x7010
#define JHISVC_CREATESESSION_EXIT							0x7011

#define JHIDLL_GETSESSIONCOUNT_ENTER						0x8000
#define JHIDLL_GETSESSIONCOUNT_EXIT							0x8001
#define JHISVC_GETSESSIONCOUNT_ENTER						0x8010
#define JHISVC_GETSESSIONCOUNT_EXIT							0x8011

#define JHIDLL_CLOSESESSION_ENTER							0x9000
#define JHIDLL_CLOSESESSION_EXIT							0x9001
#define JHISVC_CLOSESESSION_ENTER							0x9010
#define JHISVC_CLOSESESSION_EXIT							0x9011

#define JHIDLL_GETSESSIONINFO_ENTER							0xA000  
#define JHIDLL_GETSESSIONINFO_EXIT							0xA001
#define JHISVC_GETSESSIONINFO_ENTER							0xA010
#define JHISVC_GETSESSIONINFO_EXIT							0xA011 

#define JHIDLL_REGISTEREVENT_ENTER							0xB000  
#define JHIDLL_REGISTEREVENT_EXIT							0xB001
#define JHISVC_REGISTEREVENT_ENTER							0xB010
#define JHISVC_REGISTEREVENT_EXIT							0xB011 

#define JHIDLL_UNREGISTEREVENT_ENTER						0xC000  
#define JHIDLL_UNREGISTEREVENT_EXIT							0xC001
#define JHISVC_UNREGISTEREVENT_ENTER						0xC010
#define JHISVC_UNREGISTEREVENT_EXIT							0xC011 

#define JHIDLL_GETVERSIONINFO_ENTER							0xD000  
#define JHIDLL_GETVERSIONINFO_EXIT							0xD001
#define JHISVC_GETVERSIONINFO_ENTER							0xD010
#define JHISVC_GETVERSIONINFO_EXIT							0xD011 

#define JHISVC_IPTAPPLOADDLL_ENTER							0xE010
#define JHISVC_IPTAPPLOADDLL_PRESENT_EXIT					0xE021
#define JHISVC_IPTAPPLOADDLL_SIG_EXIT						0xE031
#define JHISVC_IPTAPPLOADDLL_PUB_EXIT						0xE041
#define JHISVC_IPTAPPLOADDLL_EXIT							0xE011
#define JHISVC_IPTAPPVERIFY_ENTER							0xE050
#define JHISVC_IPTAPPVERIFY_EXIT							0xE051

#define JHIDLL_GETSESSIONTABLE_ENTER						0xF000  
#define JHIDLL_GETSESSIONTABLE_EXIT							0xF001
#define JHISVC_GETSESSIONTABLE_ENTER						0xF010
#define JHISVC_GETSESSIONTABLE_EXIT							0xF011 

#define JHIDLL_GETLOADEDAPPLETS_ENTER						0xF100  
#define JHIDLL_GETLOADEDAPPLETS_EXIT						0xF101
#define JHISVC_GETLOADEDAPPLETS_ENTER						0xF110
#define JHISVC_GETLOADEDAPPLETS_EXIT						0xF111 

#ifdef __ANDROID__
inline int GetCurrentThreadId(){return gettid();}
#elif defined(__linux__)
inline int GetCurrentThreadId(){return syscall(SYS_gettid);}
#endif

// Release prints
// Should be implemented also for Linux
#ifdef _WIN32

#define JHI_LOGGER_EXIT_MACRO(type,fname,ulRetCode)  \
{ \
	JHI_Log("[%d] [%s] FN: %08x RET: 0x%08x\n",GetCurrentThreadId(),type,fname,ulRetCode);\
}
#define JHI_LOGGER_ENTRY_MACRO(type,fname)  \
{ \
	JHI_Log("[%d] [%s] FN: %08x\n",GetCurrentThreadId(),type,fname);\
}

#else
#define JHI_LOGGER_EXIT_MACRO(type,fname,ulRetCode)
#define JHI_LOGGER_ENTRY_MACRO(type,fname)
#endif // _WIN32, release prints

const char *JHIErrorToString(UINT32 retVal);
const char *TEEErrorToString(UINT32 retVal);

/* Maximum length of the string that can be printed*/

#ifdef DEBUG

#define TRACE0                         JHI_Trace
#define TRACE1(fmt,p1)                 JHI_Trace(fmt,p1)
#define TRACE2(fmt,p1,p2)              JHI_Trace(fmt,p1,p2)
#define TRACE3(fmt,p1,p2,p3)           JHI_Trace(fmt,p1,p2,p3)
#define TRACE4(fmt,p1,p2,p3,p4)        JHI_Trace(fmt,p1,p2,p3,p4)
#define TRACE5(fmt,p1,p2,p3,p4,p5)     JHI_Trace(fmt,p1,p2,p3,p4,p5)
#define TRACE6(fmt,p1,p2,p3,p4,p5,p6)  JHI_Trace(fmt,p1,p2,p3,p4,p5,p6)

#define TRACE11(fmt,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11)  JHI_Trace(fmt,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11)

#define T_TRACE1(fmt, p1)              JHI_T_Trace(fmt,p1)


UINT32 JHI_Trace(const char*  Format, ... );

UINT32 JHI_T_Trace(const TCHAR* fmt, ... );

#define __DBG_PREAMBLE__ 
#define __DBG_POSTAMBLE__

//#define __DBG_PREAMBLE__   TRACE1("===> %s: ", __FUNCTION__ ) ;
//#define __DBG_POSTAMBLE__  TRACE1("<=== %s: ", __FUNCTION__ ) ;

#else

#ifdef _MSC_VER
#define TRACE                          
#else
#define TRACE(args...)
#endif

// Evaluate to a non empty but actually no operation to avoid "empty controlled statement" warnings
#define TRACE0(fmt)						do {} while (0)
#define TRACE1(fmt,p1)					do {} while (0)
#define TRACE2(fmt,p1,p2)				do {} while (0)
#define TRACE3(fmt,p1,p2,p3)			do {} while (0)
#define TRACE4(fmt,p1,p2,p3,p4)			do {} while (0)
#define TRACE5(fmt,p1,p2,p3,p4,p5)		do {} while (0)
#define TRACE6(fmt,p1,p2,p3,p4,p5,p6)	do {} while (0)
#define TRACE11(fmt,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11)  do {} while (0)

#define T_TRACE1(fmt, p1)

#define DBG0(fmt)
#define DBG1(fmt,p1)


#define __DBG_PREAMBLE__ 
#define __DBG_POSTAMBLE__

#endif

#ifdef DEBUG


#define DISPLAY_PARA(x, l) 



#ifndef ASSERT
#define ASSERT(x) \
            if (!(x)) \
            { \
                JHI_Trace ( "\n *** ASSERTION FAILED: %s line %d: " #x " ***\n",  \
                            __FILE__, __LINE__ );                         \
            } 
#endif /*  ASSERT*/

#ifndef DEBUGASSERT

#define DEBUGASSERT(x) \
            if (!(x)) \
            { \
                JHI_Trace ( "\n *** ASSERTION FAILED: %s line %d: " #x " ***\n",  \
                            __FILE__, __LINE__ );                         \
            } 
#endif // DEBUGASSERT

#else



#ifndef ASSERT
#define ASSERT(x)
#endif // ASSERT

#ifndef DEBUGASSERT
#define DEBUGASSERT(x)
#endif // DEBUGASSERT

#endif //  DEBUG 








#ifdef DEBUG

#ifndef ASSERTMSG

#define ASSERTMSG(x,msg) \
      if (!(x))  \
      { \
         JHI_Trace ( "\n ** ASSERTION FAILED: %s line %d: " #x "\n", \
                     __FILE__, __LINE__ ) ; \
         JHI_Trace ( " *** %s\n", msg ) ; \
      }

#endif // ASSERTMSG

#else

#ifndef ASSERTMSG
#define ASSERTMSG(x,msg)
#endif // ASSERTMSG

#endif   // DEBUG

#ifdef __cplusplus
};
#endif

#endif
