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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <mbstring.h>
#include <tchar.h>
#else //!_WIN32
#include <sys/stat.h>
#include <ctype.h>
#include <sys/types.h>
#include <pthread.h>
#include <limits.h>
#include "jhi_i.h"
#include "reg.h"
#endif //_WIN32

#include <wchar.h>
#include "typedefs.h"
#include "jhi.h"
#include "dbg.h"
#include "misc.h"
#include "teemanagement.h"
#include "smoketest.h"
#include <cstdint>
#include <ostream>
#include <fstream>
#include <iterator>
#include <string_s.h>
#include <jhi.h>
#include <teemanagement.h>
#include <iostream>

using namespace std;

int console_mode=1;
static JHI_HANDLE hJOM=0;

int main (int ac, char **av)
{
	int cmd;
	JHI_RET status;

	if( 1 == ac )
	{
		print_menu();
		console_mode = 0;
		fprintf( stderr, "Please enter a valid command.\n");
		scanf("%d", &cmd);
		if( (cmd < 0) || (cmd > TESTS_NUM)   )
		{
			fprintf( stderr, "Invalid command. run SmokeTest.exe without parameters for usage.\n") ;
			exit_test(EXIT_FAILURE);
		}
	}
	else if( 2 == ac )
	{
		cmd = atoi( av[1] ) ;
		if( (cmd < 0) || (cmd > TESTS_NUM)   )
		{
			fprintf( stderr, "Invalid test number.\n") ;
			print_menu();
			exit_test(EXIT_FAILURE);
		}
	}
	else
	{
		cmd = 0;
		fprintf(stderr, "Too many arguments.\n");
		print_menu();
		exit_test(EXIT_FAILURE);
	}

	status = JHI_Initialize( &hJOM, NULL, 0 ) ; // Check for Success
	if (status != JHI_SUCCESS)
	{
		fprintf( stdout, "JHI init failed. error code: 0x%x (%s)",status, JHIErrorToString(status)) ;
		exit_test(EXIT_FAILURE);
	}

	// Check for valid handle
	fprintf( stderr, "\n Initializing JHI handle :  %p\n", hJOM);
	if( !hJOM ) {
		fprintf( stdout, "Not a valid handle during JHI init") ;
		exit_test(EXIT_FAILURE);
	}

	if ( cmd == 0 )//run all tests
	{
		fprintf(stdout, "Running all tests except for 6 (will take all applet slots in the FW and require a reflash) and 22 (only applicable a limited subset of FW types).");
		
		int i;
		for(i = 1; i <= TESTS_NUM; i++)
		{
#ifdef _WIN32
			FILECHAR title[32];
			memset((void*)title, 0, 32 * sizeof(wchar_t));
			swprintf_s(title, 32, L"Running test #%i of %i", i, TESTS_NUM);
			SetConsoleTitle(title);
#endif
			if(i != 22 && i != 6) // don't run the SD tests.
				run_cmd(i, &hJOM);
		}

	}
	else
	{
		run_cmd(cmd, &hJOM);
	}


	// now
	status = JHI_Deinit(hJOM);
	if (status != JHI_SUCCESS)
	{
		fprintf( stdout, "JHI deinit failed, error code: 0x%x (%s)", status, JHIErrorToString(status)) ;
		exit_test(EXIT_FAILURE);
	}

	exit_test(EXIT_SUCCESS);
	/*Not really reached, but make gcc happy*/
	return 1;
}

void print_menu()
{
	fprintf( stderr, "\n======================  JHI SMOKE TEST  ======================\n");
	fprintf( stderr, "Usage: SmokeTest.exe <Command Number>\n\n") ;
	fprintf( stderr, "Available Commands:\n") ;
	fprintf( stderr, "*************************************************\n");
	fprintf( stderr, "0) Run all tests.                               *\n");
	fprintf( stderr, "1) Send and Recieve test.                       *\n");
	fprintf( stderr, "2) Sessions API test.                           *\n");
	fprintf( stderr, "3) Events test.                                 *\n");
	fprintf( stderr, "4) Test max Sessions                            *\n");
	fprintf( stderr, "5) Get applet property test.                    *\n");
	fprintf( stderr, "6) JHI max applets test.                        *\n");
	fprintf( stderr, "7) JHI install from package test.               *\n");
	fprintf( stderr, "8) JHI get version info test.                   *\n");
	fprintf( stderr, "9) JHI shared session test.                     *\n");
	fprintf( stderr, "10) Send and Recieve timeout test.              *\n");
	fprintf( stderr, "11) Init Deinit reference count test.           *\n");
	fprintf( stderr, "12) Negative events test.                       *\n");
	fprintf( stderr, "13) Negative send and Recieve test.             *\n");
	fprintf( stderr, "14) Negative get applet property test.          *\n");
	fprintf( stderr, "15) Negative JHI get version info test.         *\n");
	fprintf( stderr, "16) Negative install applet test.               *\n");
	fprintf( stderr, "17) JHI list installed applets.                 *\n");
	fprintf( stderr, "18) JHI test send admin install / uninstall.    *\n");
	fprintf( stderr, "19) JHI test send admin install with session.   *\n");
	fprintf( stderr, "20) JHI test send admin UpdateSVL acp.          *\n");
	fprintf( stderr, "21) JHI test send admin QueryTeeMetadata.       *\n");
	fprintf( stderr, "22) OEM signing test.                           *\n");
	fprintf( stderr, "23) Applet Encryption                           *\n");
	fprintf( stderr, "*************************************************\n");
}

void run_cmd(int cmd, JHI_HANDLE *phJOM)
{
    switch (cmd)
    {
        case 1 :
            test_01_send_and_recieve(*phJOM);
            break;
        case 2 :
            test_02_sessions_api(*phJOM);
            break;
        case 3 :
            test_03_events(*phJOM);
            break;
        case 4 :
            test_04_max_sessions(*phJOM);
            break;
        case 5 :
            test_05_get_applet_property(*phJOM);
            break;
        case 6:
            test_06_max_installed_applets(*phJOM);
            break;
        case 7:
            test_07_install_dalp(*phJOM);
            break;
        case 8:
            test_08_get_version_info(*phJOM);
            break;
        case 9:
            test_09_shared_session(*phJOM);
            break;
        case 10:
            test_10_sar_timeout(*phJOM);
            break;
        case 11:
            test_11_init_deinit(phJOM);
            break;
        case 12:
            test_12_negative_test_events(hJOM);
            break;
        case 13:
            test_13_negative_test_send_and_recieve(hJOM);
            break;
        case 14:
            test_14_negative_test_get_applet_property(hJOM);
            break;
        case 15:
            test_15_negative_test_get_version_info(hJOM);
            break;
        case 16:
            test_16_negative_test_install_applet(hJOM);
            break;
// teemanagement tests
        case 17:
            test_17_list_installed_applets();
            break;
        case 18:
            test_18_admin_install_uninstall();
            break;
        case 19:
            test_19_admin_install_with_session(hJOM);
            break;
        case 20:
            test_20_admin_updatesvl();
            break;
        case 21:
            test_21_admin_query_tee_metadata();
            break;
		case 22:
            test_22_oem_signing();
			break;
		case 23:
			test_23_applet_encryption(*phJOM);
			break;
    }
}

void exit_test(int result)
{
	int tmp;
	if (!console_mode)
	{
		if (result == EXIT_SUCCESS)
		{
#ifdef _WIN32
			SetConsoleTitle(L"SmokeTest Passed!");
#endif
			printf("\nSmokeTest Passed!");
		}
		else
		{
#ifdef _WIN32
			SetConsoleTitle(L"SmokeTest Failed!");
#endif
			printf("\nSmokeTest Failed!");
		}

		//pause
		fflush(stdout);
		tmp = getchar();
		putchar(tmp);
		tmp += 1; // just so the compiler won't remove this
		printf("Press Enter to continue: ");
		fflush(stdout);
		while ( getchar() != '\n' )	;
	}
	exit(result);
}

#ifdef _WIN32
void GetFullFilename(wchar_t* szCurDir, const wchar_t* fileName)
{
	GetCurrentDirectory( (LEN_DIR - sizeof(TCHAR)), szCurDir);
	_tcscat(szCurDir, fileName);
}
#else
void GetFullFilename(char* szCurDir, const char* filename)
{

	//	getcwd(szCurDir, PATH_MAX);
	JhiQuerySpoolerLocationFromRegistry (szCurDir, LEN_DIR - 2);
	strcat(szCurDir, filename);
}
#endif //WIN32

void print_buffer(unsigned char* buffer, int len)
{
	int i;

	if (len == 0)
		printf("EMPTY BUFFER\n");
	else
	{
		for( i=0; i<len; i++ )
		{
			if( !(i%16) )
				printf("\n") ;
			printf("%02X ", buffer[i]) ;
		}
	}
}

void fill_buffer(unsigned char* buffer, int len)
{
	int i;

	for( i=0; i<len; i++ ) {
		buffer[i] = i % 127 ;
	}
}

int check_buffer(unsigned char* rxBuffer, int len)
{
	int i;

	if (rxBuffer == NULL && len!=0)
		return 1;

	for( i=0; i<len; i++ ) {
		//fprintf(stderr, "%d \n", rxBuffer[i]);
		if(rxBuffer[i] != (i % 127) )
			return 1;
	}
	return 0;
}

FILESTRING getEchoFileName(int num)
{
#ifdef _WIN32
    return FILEPREFIX("/echos/echo") + FILETOSTRING((long)num) + FILEPREFIX(".dalp");
#else
    char number [9];
    sprintf (number, "%d", num);
    return FILEPREFIX("/echos/echo") + FILESTRING (number) + FILEPREFIX(".dalp");
#endif
}

string getEchoUuid(int num)
{
    char hexStr[3];
    string tmp(ECHO_1_APP_ID);
    string postfix = tmp.substr(LEN_APP_ID -2,2);
    int newVal = strtol(postfix.c_str(), NULL, 16);
    newVal += num -1;
    newVal %= 0x100; //in case we get passed 32 (overflow), we keep only the 2 LSBs
#ifdef _WIN32
    _itoa_s(newVal, hexStr, 16);
#else
    snprintf(hexStr, 3, "%x", newVal);
#endif

    if (newVal < 10) // missing the leading '0'
    {
        hexStr[2]='\0';
        hexStr[1]=hexStr[0];
        hexStr[0]='0';
    }

    tmp.replace(LEN_APP_ID -2, LEN_APP_ID, hexStr, hexStr[1]);
    return tmp;
}

int AppPropertyCall(
        JHI_HANDLE hJOM,
        const FILECHAR *AppProperty,
        UINT8 rxBuffer[APP_PROPERTY_BUFFER_SIZE],
        JVM_COMM_BUFFER* txrx
)
{
    int status;
    memset(rxBuffer,0,APP_PROPERTY_BUFFER_SIZE);
    txrx->TxBuf->length = (uint32_t)FILECHARLEN(AppProperty);
    txrx->TxBuf->buffer = const_cast<FILECHAR *>(AppProperty);
    txrx->RxBuf->length = APP_PROPERTY_BUFFER_SIZE - 1;
    txrx->RxBuf->buffer = rxBuffer ;

    status = JHI_GetAppletProperty(hJOM, ECHO_APP_ID, txrx) ;

    if (status == JHI_SUCCESS)
#ifdef _WIN32
        printf("%S: %s\n", AppProperty, rxBuffer);
#else
        printf("%s: %s\n", AppProperty, rxBuffer);
#endif
    return status;
}

bool getFWVersion(VERSION* fw_version)
{
    JHI_VERSION_INFO info;
    JHI_RET status = JHI_GetVersionInfo(hJOM, &info);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "\nJHI get version info failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    if (sscanf_s(info.fw_version,"%hd.%hd.%hd.%hd",&fw_version->Major,&fw_version->Minor,&fw_version->Hotfix,&fw_version->Build) != 4)
    {
        TRACE0("recieved invalid fw version format from devplatform\n");
        return false;
    }
    return true;
}

void printUUIDs(UUID_LIST& uuidList)
{
    fprintf( stdout, "UUIDs found - %d\n", uuidList.uuidCount);

    for (uint32_t i = 0; i < uuidList.uuidCount; ++i)
    {
        fprintf( stdout, "UUID #%d - %s\n", i, uuidList.uuids[i]);
    }
}

TEE_STATUS readFileAsBlob(const FILECHAR* filepath, vector<uint8_t>& blob)
{
    TEE_STATUS ret = TEE_STATUS_INVALID_PARAMS;

    std::ifstream is(filepath, std::ios::binary);

    if (!is)
    {
        return TEE_STATUS_INTERNAL_ERROR;
    }

    try
    {

        is >> std::noskipws;
        is.seekg (0, is.end);
        std::streamoff len = is.tellg();
        is.seekg (0, is.beg);

        if (len >= MAX_APPLET_BLOB_SIZE)
        {
            ret = TEE_STATUS_INVALID_PACKAGE;
        }
        std::istream_iterator<uint8_t> start(is), end;
        vector<uint8_t> tmp(start, end);
        blob.swap(tmp);
        is.close();
        ret = TEE_STATUS_SUCCESS;
    }
    catch(...)
    {
        if (is.is_open())
        {
            is.close();
        }
        ret =  TEE_STATUS_INVALID_PARAMS;
    }

    return ret;
}

void test_01_send_and_recieve(JHI_HANDLE hJOM)
{
    UINT8   txBuffer[BUFFER_SIZE] = {0x0},
            rxBuffer[BUFFER_SIZE] = {0x0};

    int i;
    int count = 50;

    FILECHAR szCurDir [LEN_DIR];

    int32_t ResponseCode = 99999;

    JVM_COMM_BUFFER txrx ;
    JHI_SESSION_HANDLE hSession;
    JHI_RET status;

    GetFullFilename(szCurDir, ECHO_FILENAME);

    // now install the echo applet in JOM
    fprintf( stderr, "\ninstalling the echo applet \n") ;
    status = JHI_Install2( hJOM, ECHO_APP_ID, szCurDir) ;
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // create a session
    fprintf( stderr, "creating session of the echo applet \n") ;
    status = JHI_CreateSession(hJOM,ECHO_APP_ID,0,NULL,&hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // how to iterate thru
    fill_buffer(txBuffer, BUFFER_SIZE ) ;

    fprintf( stderr, "starting send and recieve sequence..\n") ;
    for( i=1; i<count+1; i++ )
    {
        txrx.TxBuf->length = i ; // length
        txrx.TxBuf->buffer = txBuffer;

        txrx.RxBuf->length = i ;
        txrx.RxBuf->buffer = rxBuffer ;

        fprintf( stderr, "Sending and receiving buffer to JOM Size: %04d... ", i );

        status = JHI_SendAndRecv2(hJOM, hSession,0, &txrx,&ResponseCode);

        if( JHI_SUCCESS != status )
        {
            fprintf( stderr, "Error in performing JHI_SendAndRecv, error code: 0x%x (%s)\n", status , JHIErrorToString(status));
            exit_test(EXIT_FAILURE);
        }

        if ((uint32_t)ResponseCode != txrx.TxBuf->length)
        {
            fprintf( stderr, "Error: SendAndRecv resposne code should have match the input buffer size.\n");
            exit_test(EXIT_FAILURE);
        }

        if( !check_buffer((unsigned char*) txrx.RxBuf->buffer, i) )
        {
            fprintf( stderr, "Verification PASS \n") ;
        }
        else
        {
            fprintf( stderr, "Verification FAIL \n") ;
        }
    }

    // send max buffer size
    txrx.TxBuf->length = BUFFER_SIZE ; // length
    txrx.TxBuf->buffer = txBuffer;

    txrx.RxBuf->length = BUFFER_SIZE ;
    txrx.RxBuf->buffer = rxBuffer ;

    fprintf( stderr, "Sending and receiving buffer to JOM Size: %04d... ", BUFFER_SIZE );
    status = JHI_SendAndRecv2(hJOM, hSession,0, &txrx,NULL);

    if( JHI_SUCCESS != status ) {
        fprintf( stderr, "\nError sending buffer with size %d, error code: 0x%x (%s)\n",BUFFER_SIZE, status , JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    if( !check_buffer((unsigned char*) txrx.RxBuf->buffer, BUFFER_SIZE) )
    {
        fprintf( stderr, "Verification PASS \n") ;
    }
    else
    {
        fprintf( stderr, "Verification FAIL \n") ;
    }

    // send short buffer size
    txrx.TxBuf->length = BUFFER_SIZE ; // length
    txrx.TxBuf->buffer = txBuffer;

    txrx.RxBuf->length = 0;
    txrx.RxBuf->buffer = NULL;

    fprintf( stderr, "Sending short response buffer to JOM, expecting JHI_INSUFFICIENT_BUFFER\n");

    status = JHI_SendAndRecv2(hJOM, hSession,0, &txrx,NULL);

    if( JHI_INSUFFICIENT_BUFFER != status)
    {
        fprintf( stderr, "Error sending short buffer to JOM failed, error code: 0x%x (%s)\n", status , JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    if( txrx.RxBuf->length != BUFFER_SIZE)
    {
        fprintf( stderr, "Error sending short buffer to JOM failed expected RxBuf size %d, received %d\n", BUFFER_SIZE, txrx.RxBuf->length);
        exit_test(EXIT_FAILURE);
    }


    //send null buffer
    txrx.TxBuf->length = 0 ; // length
    txrx.TxBuf->buffer = NULL;
    //	txrx.TxBuf->buffer = txBuffer;

    txrx.RxBuf->length = 0 ;
    txrx.RxBuf->buffer = NULL ;

    status = JHI_SendAndRecv2(hJOM, hSession,0, &txrx,NULL);

    if( JHI_SUCCESS != status )
    {
        fprintf( stderr, "Error sending buffer with size %d, error code: 0x%x (%s)\n",txrx.TxBuf->length, status, JHIErrorToString(status) );
        exit_test(EXIT_FAILURE);
    }

    fprintf( stderr, "Sending and receiving buffer to JOM Size: %04d - ", txrx.TxBuf->length);

    if( txrx.RxBuf->length == 0 )
    {
        fprintf( stderr, "Verification PASS \n") ;
    }
    else
    {
        fprintf( stderr, "Verification FAIL \n") ;
    }

    // close the session
    status = JHI_CloseSession(hJOM,&hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    status = JHI_Uninstall (hJOM, ECHO_APP_ID);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall applet failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "Send and Recieve test passed\n") ;
}

void test_02_sessions_api(JHI_HANDLE hJOM)
{
    JHI_SESSION_HANDLE hSession;
    JHI_RET status;
    UINT32 session_count;
    JHI_SESSION_INFO info;
    FILECHAR szCurDir [LEN_DIR];
    UINT8 buffer[5] = { 0x01,0x02,0x03,0x04,0x05 };
    DATA_BUFFER initData;

    initData.buffer = buffer;
    initData.length = 5;

    GetFullFilename(szCurDir, ECHO_FILENAME);

    // try to  install the spooler applet in JOM
    status = JHI_Install2( hJOM, SPOOLER_APP_ID, szCurDir) ;
    if (status == JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install spooler applet should have failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // now install the echo applet in JOM
    status = JHI_Install2( hJOM, ECHO_APP_ID, szCurDir) ;
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // create a session
    status = JHI_CreateSession(hJOM,ECHO_APP_ID,0,&initData,&hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    status = JHI_GetSessionsCount(hJOM,ECHO_APP_ID,&session_count);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI GetSessionsCount failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    if (session_count != 1)
    {
        fprintf( stdout, "error: session count should be 1\n") ;
        exit_test(EXIT_FAILURE);
    }

    // get session status

    status = JHI_GetSessionInfo(hJOM,hSession,&info);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI Get Session Status failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    if (info.state != JHI_SESSION_STATE_ACTIVE)
    {
        fprintf( stdout, "error: session status should be SESSION_ACTIVE(1)\n") ;
        exit_test(EXIT_FAILURE);
    }

    // close the session
    status = JHI_CloseSession(hJOM,&hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    status = JHI_GetSessionsCount(hJOM,ECHO_APP_ID,&session_count);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI GetSessionsCount failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    if (session_count != 0)
    {
        fprintf( stdout, "error: session count should be 0\n") ;
        exit_test(EXIT_FAILURE);
    }

    status = JHI_Uninstall (hJOM, ECHO_APP_ID);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall applet failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "\nSessions test passed\n") ;
}

static int test_event_raised = 0;
static int test_event_buffer_match;
static int test_event_max_number;

#ifdef _WIN32
static HANDLE callback_event;
#else //!WIN32
static pthread_mutex_t callback_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t callback_cond = PTHREAD_COND_INITIALIZER;
#endif //WIN32

void test_03_events(JHI_HANDLE hJOM)
{
	JHI_SESSION_HANDLE hSession;
	JHI_SESSION_HANDLE openCloseSession;
	JHI_RET status;
	FILECHAR szCurDir [LEN_DIR];

	UINT8   txBuffer[EVENTS_BUFFER_SIZE] = {0x0},
		rxBuffer[EVENTS_BUFFER_SIZE] = {0x0};

	JVM_COMM_BUFFER txrx ;
	int i;
	txrx.TxBuf->length = EVENTS_BUFFER_SIZE ;
	txrx.TxBuf->buffer = txBuffer;
	txrx.RxBuf->length = EVENTS_BUFFER_SIZE ;
	txrx.RxBuf->buffer = rxBuffer;

	/*printf("10 seconds...");
	Sleep(10000);*/

	fill_buffer(txBuffer,EVENTS_BUFFER_SIZE);

	GetFullFilename(szCurDir, EVENT_SERVICE_FILENAME);

	test_event_raised = 0;
	test_event_buffer_match = 0;
	test_event_max_number = 5;

#ifdef _WIN32
	callback_event = CreateEvent(NULL,FALSE,FALSE,NULL);
	if (callback_event == NULL)
	{
		fprintf( stderr, "Error: failed to create win32 event handle.\n");
		exit_test(EXIT_FAILURE);
	}
#else //!WIN32
	pthread_mutex_init(&callback_mutex, NULL);
	pthread_cond_init(&callback_cond, NULL);
#endif //WIN32

	fprintf( stderr, "\nInstalling the Event Service applet \n");

	// install the Event Service applet in JOM
	status = JHI_Install2( hJOM, EVENT_SERVICE_APP_ID, szCurDir);
	if (status != JHI_SUCCESS)
	{
		fprintf( stdout, "JHI install failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
		exit_test(EXIT_FAILURE);
	}

	// create a session, register it for events and close the session without unregistering
	fprintf( stderr, "\ncreate a session of Event Service\n");
	status = JHI_CreateSession(hJOM,EVENT_SERVICE_APP_ID,0,NULL,&openCloseSession);
	if (status != JHI_SUCCESS)
	{
		fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
		exit_test(EXIT_FAILURE);
	}

	// register for event from the Event Service session
	fprintf( stderr, "register for event from the Event Service session\n");
	status = JHI_RegisterEvents(hJOM,openCloseSession,(JHI_EventFunc)onEvent);
	if (status != JHI_SUCCESS)
	{
		fprintf( stdout, "JHI register event failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
		exit_test(EXIT_FAILURE);
	}

	// close the session
	fprintf( stderr, "close the session without calling unregister\n");
	status = JHI_CloseSession(hJOM,&openCloseSession);
	if (status != JHI_SUCCESS)
	{
		fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
		exit_test(EXIT_FAILURE);
	}

	// create a session of Event Service
	fprintf( stderr, "\ncreate a session of Event Service\n");
	status = JHI_CreateSession(hJOM,EVENT_SERVICE_APP_ID,0,NULL,&hSession);
	if (status != JHI_SUCCESS)
	{
		fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
		exit_test(EXIT_FAILURE);
	}

	// register for event from the Event Service session
	fprintf( stderr, "register for event from the Event Service session\n");
	status = JHI_RegisterEvents(hJOM,hSession,(JHI_EventFunc)onEvent);
	if (status != JHI_SUCCESS)
	{
		fprintf( stdout, "JHI register event failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
		exit_test(EXIT_FAILURE);
	}

	for (i = 0; i < test_event_max_number; i++) {
		// call send and recieve with command = 10 in order to invoke event by the Event Service
		fprintf( stderr, "call SAR2 with command = 10 in order to invoke event by the Event Service\n");
		status = JHI_SendAndRecv2(hJOM, hSession,10, &txrx,NULL);
		if (status != JHI_SUCCESS)
		{
			fprintf( stdout, "JHI send and recieve 2 failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
			exit_test(EXIT_FAILURE);
		}
	}

	// do some waiting here...
	fprintf( stdout, "Entering infinite sleep until callback invoked...\n") ;
#ifdef _WIN32
	WaitForSingleObject(callback_event, INFINITE);
#else //!WIN32
	pthread_mutex_lock(&callback_mutex);
	pthread_cond_wait(&callback_cond, &callback_mutex);
	pthread_mutex_unlock(&callback_mutex);
#endif //WIN32

	// unregister the event
	status = JHI_UnRegisterEvents(hJOM,hSession);
	if (status != JHI_SUCCESS)
	{
		fprintf( stdout, "JHI untegister event failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
		exit_test(EXIT_FAILURE);
	}

	// close the session
	status = JHI_CloseSession(hJOM,&hSession);
	if (status != JHI_SUCCESS)
	{
		fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
		exit_test(EXIT_FAILURE);
	}

	status = JHI_Uninstall (hJOM, EVENT_SERVICE_APP_ID);
	if (status != JHI_SUCCESS)
	{
		fprintf( stdout, "JHI uninstall applet failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
		exit_test(EXIT_FAILURE);
	}

	if (test_event_raised < test_event_max_number)
	{
		fprintf( stdout, "not all events were raised - test number %d, raised events %d.\n",
			test_event_max_number, test_event_raised ) ;
		exit_test(EXIT_FAILURE);
	}

	if (test_event_buffer_match < test_event_max_number)
	{
		fprintf( stdout, "not all event buffers are valid - test number %d, valid event buffers %d.\n",
			test_event_max_number, test_event_buffer_match) ;
		exit_test(EXIT_FAILURE);
	}

	fprintf( stdout, "\nevents test passed\n") ;

}

void onEvent(JHI_SESSION_HANDLE SessionHandle,JHI_EVENT_DATA eventData)
{
	JHI_RET status;
	JHI_SESSION_INFO info;
	info.state = JHI_SESSION_STATE_NOT_EXISTS;
	info.flags = 0xFFFFFFFF;

	fprintf( stdout, "*****************   EVENT RAISED   **********************\n") ;
	test_event_raised++;

	if (check_buffer(eventData.data,EVENTS_BUFFER_SIZE) == 0)
		test_event_buffer_match++;

	status = JHI_GetSessionInfo(hJOM,SessionHandle,&info);
	if (status != JHI_SUCCESS)
	{
		fprintf( stdout, "JHI Get Session Status failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
		exit_test(EXIT_FAILURE);
	}
	if (info.state != JHI_SESSION_STATE_ACTIVE)
	{
		fprintf( stdout, "error: session status should be SESSION_ACTIVE(1)\n") ;
		exit_test(EXIT_FAILURE);
	}

#ifdef _WIN32
	if (test_event_raised == test_event_max_number)
	{
		SetEvent(callback_event);
	}
#else //!WIN32
	pthread_mutex_lock(&callback_mutex);
	if (test_event_raised == test_event_max_number)
	{
		pthread_cond_signal(&callback_cond);
	}
	pthread_mutex_unlock(&callback_mutex);
#endif //WIN32
}

void test_04_max_sessions(JHI_HANDLE hJOM)
{
    JHI_SESSION_HANDLE* hSession = NULL;
    JHI_RET status;
    UINT32 session_count;
    JHI_SESSION_INFO info;
    UINT32 i, maxSessionNum = 0;
    FILECHAR szCurDir [LEN_DIR];

    //check FW version
    VERSION version;
    if (!getFWVersion(&version))
    {
        fprintf( stdout, "Get version failed, aborting test.\n");
        exit_test(EXIT_FAILURE);
    }

    if ((version.Major >= 7 && version.Major <= 10) || version.Major == 1 || version.Major == 2)
		maxSessionNum = (UINT32)MAX_SESSIONS_BH1;
	else if (version.Major == 11 || version.Major == 12)
		maxSessionNum = (UINT32)MAX_SESSIONS_BH2_GEN1;
    else
		maxSessionNum = (UINT32)MAX_SESSIONS_BH2_GEN2;
    
    hSession = JHI_ALLOC_T_ARRAY<JHI_SESSION_HANDLE>(maxSessionNum);
    if (hSession == NULL) return;

    fprintf(stdout, "\n Starting MAX Sessions test. (Max sessions allowed:%d)\n", maxSessionNum) ;

    GetFullFilename(szCurDir, ECHO_FILENAME);

    // now install the echo applet in JOM
    status = JHI_Install2( hJOM, ECHO_APP_ID, szCurDir) ;
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // 1. create maximum sessions
    for (i=0; i < maxSessionNum; i++)
    {
        fprintf( stdout, "Creating Session No. %d...\n", i+1) ;
        // create a session
        status = JHI_CreateSession(hJOM, ECHO_APP_ID, 0,NULL, &(hSession[i]));
        if (status != JHI_SUCCESS)
        {
            fprintf( stdout, "JHI create session %d failed, error code: 0x%x (%s)\n", i+1, status, JHIErrorToString(status));
            exit_test(EXIT_FAILURE);
        }

        status = JHI_GetSessionsCount(hJOM, ECHO_APP_ID, &session_count);
        if (status != JHI_SUCCESS)
        {
            fprintf( stdout, "JHI GetSessionsCount failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
            exit_test(EXIT_FAILURE);
        }
        if (session_count != (i+1))
        {
            fprintf( stdout, "error: session count should be %d\n", i+1) ;
            exit_test(EXIT_FAILURE);
        }

        // get session status
        status = JHI_GetSessionInfo(hJOM,hSession[i], &info);
        if (status != JHI_SUCCESS)
        {
            fprintf( stdout, "JHI Get Session Status failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
            exit_test(EXIT_FAILURE);
        }
        if (info.state != JHI_SESSION_STATE_ACTIVE)
        {
            fprintf( stdout, "error: session status should be SESSION_ACTIVE(1)\n") ;
            exit_test(EXIT_FAILURE);
        }
    }

    // 2. try to create one more session - should fail.
    status = JHI_CreateSession(hJOM, ECHO_APP_ID, 0, NULL, &(hSession[i]));
    if (status == JHI_SUCCESS)
    {
        fprintf( stdout, "Error: JHI create a session beyond max sessions succeded when should have failed\n") ;
        exit_test(EXIT_FAILURE);
    }

    if (status != JHI_MAX_SESSIONS_REACHED)
    {
        fprintf( stdout, "Error: wrong error code received - 0x%x (%s)\n, should be JHI_MAX_SESSIONS_REACHED.\n", status, JHIErrorToString(status)) ;
        exit_test(EXIT_FAILURE);
    }

    // 3. close all sessions
    for (i=0; i < maxSessionNum; i++)
    {
        // close the session
        status = JHI_CloseSession(hJOM, &(hSession[i]));
        if (status != JHI_SUCCESS)
        {
            fprintf( stdout, "JHI close session %d failed, error code: 0x%x (%s)\n", i+1, status, JHIErrorToString(status));
            exit_test(EXIT_FAILURE);
        }

        status = JHI_GetSessionsCount(hJOM,ECHO_APP_ID,&session_count);
        if (status != JHI_SUCCESS)
        {
            fprintf( stdout, "JHI GetSessionsCount failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
            exit_test(EXIT_FAILURE);
        }
        if (session_count != maxSessionNum-(i+1))
        {
            fprintf( stdout, "error: session count should be %d\n",maxSessionNum-(i+1)) ;
            exit_test(EXIT_FAILURE);
        }
    }

    JHI_DEALLOC_T_ARRAY<JHI_SESSION_HANDLE>(hSession);

    status = JHI_Uninstall (hJOM, ECHO_APP_ID);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall applet failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "\nMAX Sessions test passed\n") ;
}

void test_05_get_applet_property(JHI_HANDLE hJOM)
{
    UINT8 rxBuffer[APP_PROPERTY_BUFFER_SIZE] = {0x0};

    int ispass = 1;
    FILECHAR szCurDir [LEN_DIR];

    // supported proprties
    const FILECHAR * AppProperty_Name = FILEPREFIX("applet.name");
    const FILECHAR * AppProperty_Version = FILEPREFIX("applet.version");
    const FILECHAR * AppProperty_Vendor = FILEPREFIX("applet.vendor");
    const FILECHAR * AppProperty_SecurityVersion = FILEPREFIX("security.version");
    const FILECHAR * AppProperty_Description = FILEPREFIX("applet.description");
    const FILECHAR * AppProperty_FlashQuota = FILEPREFIX("applet.flash.quota");
    const FILECHAR * AppProperty_DebugEnable = FILEPREFIX("applet.debug.enable");
    const FILECHAR * AppProperty_SharedSessionSupport = FILEPREFIX("applet.shared.session.support");
    const FILECHAR * AppProperty_Platform = FILEPREFIX("applet.platform");

    // not supported properties
    const FILECHAR * AppProperty_ServiceID = FILEPREFIX("config.s.serviceID");
    const FILECHAR * AppProperty_HeapSize = FILEPREFIX("config.s.heap_size");
    const FILECHAR * AppProperty_MinFWVersion = FILEPREFIX("firmware.min_version");
    const FILECHAR * AppProperty_WatchDogTimeOut = FILEPREFIX("config.s.watchdog.timeout");
    const FILECHAR * AppProperty_SuspendTimeout = FILEPREFIX("config.s.debug.suspend.timeout");
    const FILECHAR * AppProperty_WrittenByIntel = FILEPREFIX("applet.written.by.intel");
    const FILECHAR * AppProperty_EventRegister = FILEPREFIX("config.s.permission.event.register");
    const FILECHAR * AppProperty_EventPost = FILEPREFIX("config.s.permission.event.post");

    JVM_COMM_BUFFER txrx;
    JHI_RET status;

    GetFullFilename(szCurDir, ECHO_FILENAME);

    // now install the echo applet in JOM
    fprintf( stderr, "\ninstalling the echo applet \n") ;
    status = JHI_Install2( hJOM, ECHO_APP_ID, szCurDir) ;
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stderr, "starting get applet property calls\n\n");

    do
    {
        // sending invalid properties
        status = AppPropertyCall(hJOM,AppProperty_ServiceID,rxBuffer,&txrx);
        if (status!= JHI_APPLET_PROPERTY_NOT_SUPPORTED) break;

        status = AppPropertyCall(hJOM,AppProperty_HeapSize,rxBuffer,&txrx);
        if (status!= JHI_APPLET_PROPERTY_NOT_SUPPORTED) break;

        status = AppPropertyCall(hJOM,AppProperty_WatchDogTimeOut,rxBuffer,&txrx);
        if (status!= JHI_APPLET_PROPERTY_NOT_SUPPORTED) break;

        status = AppPropertyCall(hJOM,AppProperty_SuspendTimeout,rxBuffer,&txrx);
        if (status!= JHI_APPLET_PROPERTY_NOT_SUPPORTED) break;

        status = AppPropertyCall(hJOM,AppProperty_WrittenByIntel,rxBuffer,&txrx);
        if (status!= JHI_APPLET_PROPERTY_NOT_SUPPORTED) break;

        status = AppPropertyCall(hJOM,AppProperty_EventRegister,rxBuffer,&txrx);
        if (status!= JHI_APPLET_PROPERTY_NOT_SUPPORTED) break;

        status = AppPropertyCall(hJOM,AppProperty_EventPost,rxBuffer,&txrx);
        if (status!= JHI_APPLET_PROPERTY_NOT_SUPPORTED) break;

        status = AppPropertyCall(hJOM,AppProperty_MinFWVersion,rxBuffer,&txrx);
        if (status!= JHI_APPLET_PROPERTY_NOT_SUPPORTED) break;


        // try to send valid applet property with a short buffer
        memset(rxBuffer, 0, APP_PROPERTY_BUFFER_SIZE);
        txrx.TxBuf->length = (uint32_t)FILECHARLEN(AppProperty_Name);
        txrx.TxBuf->buffer = const_cast<FILECHAR *>(AppProperty_Name);
        txrx.RxBuf->length = 0;
        txrx.RxBuf->buffer = rxBuffer;

        status = JHI_GetAppletProperty(hJOM, ECHO_APP_ID, &txrx) ;

        if (status != JHI_INSUFFICIENT_BUFFER || txrx.RxBuf->length != 11) // "echo applet" = 11 chars
        {
            status = -1;
            break;
        }

        // sending all valid properties
        status = AppPropertyCall(hJOM,AppProperty_Name,rxBuffer,&txrx);
        if (status!= JHI_SUCCESS) break;

        status = AppPropertyCall(hJOM,AppProperty_Version,rxBuffer,&txrx);
        if (status!= JHI_SUCCESS) break;

        status = AppPropertyCall(hJOM,AppProperty_Vendor,rxBuffer,&txrx);
        if (status!= JHI_SUCCESS) break;

        status = AppPropertyCall(hJOM,AppProperty_SecurityVersion,rxBuffer,&txrx);
        if (status!= JHI_SUCCESS) break;

        status = AppPropertyCall(hJOM,AppProperty_Description,rxBuffer,&txrx);
        if (status!= JHI_SUCCESS) break;

        status = AppPropertyCall(hJOM,AppProperty_FlashQuota,rxBuffer,&txrx);
        if (status!= JHI_SUCCESS) break;

        status = AppPropertyCall(hJOM,AppProperty_DebugEnable,rxBuffer,&txrx);
        if (status!= JHI_SUCCESS) break;

        status = AppPropertyCall(hJOM,AppProperty_SharedSessionSupport,rxBuffer,&txrx);
        if (status!= JHI_SUCCESS) break;

        status = AppPropertyCall(hJOM,AppProperty_Platform,rxBuffer,&txrx);
        if (status!= JHI_SUCCESS) break;

    }
    while (0);

    if( JHI_SUCCESS != status ) {
        fprintf( stderr, "\nError: Get Applet property failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        ispass = 0;
    }

    status = JHI_Uninstall (hJOM, ECHO_APP_ID);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall echo applet failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    if (!ispass)
        exit_test(EXIT_FAILURE);

    fprintf( stdout, "\nGet Applet Property test passed\n") ;
}

void test_06_max_installed_applets(JHI_HANDLE hJOM)
{
    JHI_RET status;
    JHI_SESSION_HANDLE hSession[16];	// 16 is max of all versions
	uint8_t maxAppletsCount;
	uint8_t maxSessionsCount;
    FILECHAR szCurDir [LEN_DIR];

    fprintf( stdout, "\nStarting JHI Max applets test...\n");

    //check FW version
    VERSION version;
    if (!getFWVersion(&version))
    {
        fprintf( stdout, "Get version failed, aborting test.\n");
        exit_test(EXIT_FAILURE);
    }

	if ((version.Major >= 7 && version.Major <= 10) || version.Major == 1 || version.Major == 2)
	{
		maxAppletsCount = MAX_APPLETS_BH1;
		maxSessionsCount = (UINT32)MAX_SESSIONS_BH1;
	}
	else if (version.Major == 11 || version.Major == 12)
	{
		maxAppletsCount = MAX_APPLETS_BH2;
		maxSessionsCount = (UINT32)MAX_SESSIONS_BH2_GEN1;
	}
	else
	{
		maxAppletsCount = MAX_APPLETS_BH2;
		maxSessionsCount = (UINT32)MAX_SESSIONS_BH2_GEN2;
	}

	fprintf( stdout, "FW major version is %d, max applets limit is %d, max sessions limit is %d.\n", version.Major, maxAppletsCount, maxSessionsCount);
    
    //installing the event service TA
    fprintf( stdout, "JHI installing the event service TA...");
    GetFullFilename(szCurDir, EVENT_SERVICE_FILENAME);

    status = JHI_Install2( hJOM, EVENT_SERVICE_APP_ID, szCurDir) ;
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "\nJHI installing the event service, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    fprintf( stdout, " succeeded\n");

    // Install max TAs & create max sessions
    for (int i = 1; i <= maxAppletsCount; ++i)
    {
        GetFullFilename(szCurDir, getEchoFileName(i).c_str());

        fprintf( stdout, "JHI installing applet #%d from %s\n", i, ConvertWStringToString(szCurDir).c_str());
        status = JHI_Install2( hJOM, getEchoUuid(i).c_str(), szCurDir) ;
        if (status != JHI_SUCCESS)
        {
            if (status == JHI_MAX_INSTALLED_APPLETS_REACHED)
            {
                fprintf( stdout, "\nERROR: JHI install echo received JHI_MAX_INSTALLED_APPLETS_REACHED prematurely,\nperhaps another TA was installed prior to this test.\nTry again with a clean FW.\n");
            }
            else
            {
                fprintf( stdout, "JHI install echo %d failed, error code: 0x%x (%s)\n", i, status, JHIErrorToString(status));
            }
            exit_test(EXIT_FAILURE);
        }
        fprintf( stdout, "Succeeded\n");

        if (i <= maxSessionsCount) //open only until max sessions reached.
        {
            fprintf( stdout, "JHI creating session %d...", i);
            status = JHI_CreateSession(hJOM, getEchoUuid(i).c_str(), 0, NULL, &(hSession[i-1]));
            if (status != JHI_SUCCESS)
            {
                fprintf( stdout, "\nJHI create session %d failed, error code: 0x%x (%s)\n", i, status, JHIErrorToString(status));
                exit_test(EXIT_FAILURE);
            }
            fprintf( stdout, " succeeded\n");
        }
    }

    // now install the last echo applet in JOM after reachind the maximum. should fail.
    GetFullFilename(szCurDir, getEchoFileName(maxAppletsCount + 1).c_str());
    fprintf( stdout, "\nNow install the last echo applet in JOM after reachind the maximum. should fail.\n");

    status = JHI_Install2( hJOM, getEchoUuid(maxAppletsCount + 1).c_str(), szCurDir) ;
    if (status != JHI_MAX_INSTALLED_APPLETS_REACHED)
    {
        fprintf( stdout, "JHI install echo%d did not return the correct return code\nReceived 0x%x (%s), expected JHI_MAX_INSTALLED_APPLETS_REACHED\n", maxAppletsCount + 1, status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    fprintf( stdout, "Install failed as expected.\n\n");

    status = JHI_SUCCESS;

    // Close sessions & uninstall the echo applets
    for (int i = 1; i <= maxAppletsCount; ++i)
    {
        if (i <= maxSessionsCount) //close only until max sessions reached.
        {
            fprintf( stdout, "JHI closing session %d...", i);
            status = JHI_CloseSession(hJOM, &(hSession[i-1]));
            if (status != JHI_SUCCESS)
            {
                fprintf( stdout, "\nJHI close session %d failed, error code: 0x%x (%s)\n", i, status, JHIErrorToString(status));
                exit_test(EXIT_FAILURE);
            }
            fprintf( stdout, " succeeded\n");
        }

        // now install the echo_1 applet in JOM
        status = JHI_Uninstall( hJOM, (char*)getEchoUuid(i).c_str());
        if (status != JHI_SUCCESS)
        {
            fprintf( stdout, "JHI uninstall echo %d failed, error code: 0x%x (%s)\n", i, status, JHIErrorToString(status));
            exit_test(EXIT_FAILURE);
        }
    }

    // uninstalling the event service TA
    fprintf( stdout, "\nJHI uninstalling the event service TA...");
    status = JHI_Uninstall( hJOM, EVENT_SERVICE_APP_ID) ;
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "\nJHI uninstalling the event service, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    fprintf( stdout, " succeeded\n");

    //try to uninstall the last applet, should fail.
    fprintf( stdout, "\nTry to uninstall the last applet, should fail.\n");
    status = JHI_Uninstall (hJOM, (char*)getEchoUuid(maxAppletsCount + 1).c_str());
    if (status == JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall echo applet6 succeded when should have failed\n") ;
        exit_test(EXIT_FAILURE);
    }
    fprintf( stdout, "Uninstall failed as expected.\n\n");

    fprintf( stdout, "\nMax Applets test passed\n") ;
}

void test_07_install_dalp(JHI_HANDLE hJOM)
{
    JHI_RET status;
    FILECHAR szCurDir [LEN_DIR];

    GetFullFilename(szCurDir, ECHO_FILENAME);

    // now install the echo applet in JOM
    status = JHI_Install2( hJOM, ECHO_APP_ID, szCurDir) ;
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install echo failed\n") ;
        exit_test(EXIT_FAILURE);
    }

    // Uninstall the echo applets
    status = JHI_Uninstall (hJOM, ECHO_APP_ID);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall echo applet failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "install from package test passed.\n") ;
}

void test_08_get_version_info(JHI_HANDLE hJOM)
{
    JHI_VERSION_INFO info;
    JHI_RET status = JHI_GetVersionInfo(hJOM, &info);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "\nJHI get version info failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // print here the info
    printf("\nJHI VERSION INFO:\n");
    printf("jhi version: %s\n",info.jhi_version);
    printf("FW version: %s\n",info.fw_version);

    if (info.comm_type == JHI_SOCKETS)
        printf("Communication type: SOCKETS\n");
    else if (info.comm_type == JHI_HECI)
        printf("Communication type: HECI\n");
    else
    {
        fprintf( stdout, "\ninvalid communication type! test failed.\n") ;
        exit_test(EXIT_FAILURE);
    }

    if (info.platform_id == ME)
        printf("Platform type: ME\n");
    else if (info.platform_id == SEC)
        printf("Platform type: SEC\n");
    else if (info.platform_id == CSE)
        printf("Platform type: CSE\n");
    else
    {
        fprintf( stdout, "\ninvalid platform type! test failed.\n") ;
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "\nJHI get version info passed\n") ;
}

void test_09_shared_session(JHI_HANDLE hJOM)
{
    JHI_SESSION_HANDLE hSession1;
    JHI_SESSION_HANDLE hSession2;
    JHI_RET status;
    UINT32 session_count;
    JHI_SESSION_INFO info;

    FILECHAR szCurDir [LEN_DIR];

    fprintf( stdout, "\nStarting Shared Session test...\n") ;

    GetFullFilename(szCurDir, ECHO_FILENAME);

    // now install the echo applet in JOM
    status = JHI_Install2( hJOM, ECHO_APP_ID, szCurDir) ;
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // create the first shared session
    status = JHI_CreateSession(hJOM, ECHO_APP_ID, JHI_SHARED_SESSION, NULL, &hSession1);

    if (status == JHI_SHARED_SESSION_NOT_SUPPORTED)
    {
        fprintf( stdout, "error: shared sessions are not supported in this applet.\n") ;
        exit_test(EXIT_FAILURE);
    }

    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    status = JHI_GetSessionsCount(hJOM,ECHO_APP_ID,&session_count);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI GetSessionsCount failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    if (session_count != 1)
    {
        fprintf( stdout, "error: session count should be 1\n") ;
        exit_test(EXIT_FAILURE);
    }

    // get session status

    status = JHI_GetSessionInfo(hJOM,hSession1,&info);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI Get Session Status failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    if (info.state != JHI_SESSION_STATE_ACTIVE)
    {
        fprintf( stdout, "error: session status should be SESSION_ACTIVE(1)\n") ;
        exit_test(EXIT_FAILURE);
    }

    if ((info.flags & JHI_SHARED_SESSION) != JHI_SHARED_SESSION)
    {
        fprintf( stdout, "error: shared session flag should be set\n") ;
        exit_test(EXIT_FAILURE);
    }

    // create the second shared session
    status = JHI_CreateSession(hJOM, ECHO_APP_ID, JHI_SHARED_SESSION, NULL, &hSession2);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI create second session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    status = JHI_GetSessionsCount(hJOM,ECHO_APP_ID,&session_count);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI GetSessionsCount failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    if (session_count != 1)
    {
        fprintf( stdout, "error: session count should be 1\n") ;
        exit_test(EXIT_FAILURE);
    }

    // close the session
    status = JHI_CloseSession(hJOM,&hSession1);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    status = JHI_GetSessionsCount(hJOM,ECHO_APP_ID,&session_count);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI GetSessionsCount failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    if (session_count != 1)
    {
        fprintf( stdout, "error: session count should be 1\n") ;
        exit_test(EXIT_FAILURE);
    }

    // close the session
    status = JHI_CloseSession(hJOM,&hSession2);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    status = JHI_GetSessionsCount(hJOM,ECHO_APP_ID,&session_count);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI GetSessionsCount failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    if (session_count != 1)
    {
        fprintf( stdout, "error: session count should be 1\n") ;
        exit_test(EXIT_FAILURE);
    }

    status = JHI_Uninstall (hJOM, ECHO_APP_ID);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall applet failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "\nShared Session test passed\n") ;
}

void test_10_sar_timeout(JHI_HANDLE hJOM)
{
    UINT8   txBuffer[BUFFER_SIZE] = {0x0},
            rxBuffer[BUFFER_SIZE] = {0x0};

    FILECHAR szCurDir [LEN_DIR];

    INT32 appletRetCode = 1;

    JVM_COMM_BUFFER txrx ;
    JHI_SESSION_HANDLE hSession;
    JHI_RET status;
    UINT32 session_count = -1;

    printf("\nStarting Send and Recieve timeout test: \n");

    GetFullFilename(szCurDir, ECHO_FILENAME);

    // now install the echo applet in JOM
    fprintf( stderr, "\ninstalling the echo applet \n") ;
    status = JHI_Install2( hJOM, ECHO_APP_ID, szCurDir) ;
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // create a session
    fprintf( stderr, "creating session of the echo applet \n") ;
    status = JHI_CreateSession(hJOM,ECHO_APP_ID,0,NULL,&hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // how to iterate thru
    fill_buffer(txBuffer, BUFFER_SIZE ) ;

    fprintf( stderr, "starting send and recieve sequence..\n") ;

    //send null buffer
    txrx.TxBuf->length = 1; // length
    txrx.TxBuf->buffer = txBuffer;

    txrx.RxBuf->length = 1;
    txrx.RxBuf->buffer = rxBuffer;

    fprintf( stdout, "\nEntering infinite loop in session,\nexpecting to recieve timeout (JHI_APPLET_TIMEOUT)\n") ;
    status = JHI_SendAndRecv2(hJOM, hSession,1000, &txrx, &appletRetCode);

    if(JHI_APPLET_TIMEOUT != status ) {
        fprintf( stderr, "Error - JHI_APPLET_TIMEOUT was not received as expected.\nJHI error code - 0x%x (%s)\nApplet error code - %d", status, JHIErrorToString(status), appletRetCode);
        exit_test(EXIT_FAILURE);
    }

    status = JHI_GetSessionsCount(hJOM,ECHO_APP_ID,&session_count);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI GetSessionsCount failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    if (session_count != 0)
    {
        fprintf( stdout, "error: session count should be 0\n") ;
        exit_test(EXIT_FAILURE);
    }

    status = JHI_Uninstall (hJOM, ECHO_APP_ID);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall applet failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "Send and Recieve timeout test passed\n") ;
}

void test_11_init_deinit(JHI_HANDLE *hJOM)
{
    JHI_SESSION_HANDLE hSession;
    JHI_RET status;
    int i;


    FILECHAR szCurDir [LEN_DIR];

    fprintf( stdout, "\nStarting Init Deinit Ref Count test...\n") ;

    GetFullFilename(szCurDir, ECHO_FILENAME);

    // now install the echo applet in JOM
    status = JHI_Install2(*hJOM, ECHO_APP_ID, szCurDir) ;
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }


    for (i=0; i < 20; i++)
    {
        status = JHI_Initialize( hJOM, NULL, 0 ) ; // Check for Success
        if (status != JHI_SUCCESS)
        {
            fprintf( stdout, "JHI init failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
            exit_test(EXIT_FAILURE);
        }

        // create a session
        status = JHI_CreateSession(*hJOM, ECHO_APP_ID, 0, NULL, &hSession);

        if (status != JHI_SUCCESS)
        {
            fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
            exit_test(EXIT_FAILURE);
        }

        // close the session
        status = JHI_CloseSession(*hJOM,&hSession);
        if (status != JHI_SUCCESS)
        {
            fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
            exit_test(EXIT_FAILURE);
        }
    }

    for (i=0; i < 20; i++)
    {
        status = JHI_Deinit(*hJOM);
        if (status != JHI_SUCCESS)
        {
            fprintf( stdout, "JHI deinit failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
            exit_test(EXIT_FAILURE);
        }

        // create a session
        status = JHI_CreateSession(*hJOM, ECHO_APP_ID, 0, NULL, &hSession);

        if (status != JHI_SUCCESS)
        {
            fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
            exit_test(EXIT_FAILURE);
        }

        // close the session
        status = JHI_CloseSession(*hJOM,&hSession);
        if (status != JHI_SUCCESS)
        {
            fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
            exit_test(EXIT_FAILURE);
        }

    }

    status = JHI_Deinit(*hJOM);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI deinit failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // create session
    status = JHI_CreateSession(*hJOM, ECHO_APP_ID, 0, NULL, &hSession);
    if (status == JHI_SUCCESS)
    {
        fprintf( stdout, "JHI create session succeeded when should have failed!\n") ;
        exit_test(EXIT_FAILURE);
    }

    status = JHI_Initialize( hJOM, NULL, 0 ) ; // Check for Success
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI init failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // create a session
    status = JHI_CreateSession(*hJOM, ECHO_APP_ID, 0, NULL, &hSession);

    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // close the session
    status = JHI_CloseSession(*hJOM,&hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    status = JHI_Uninstall (*hJOM, ECHO_APP_ID);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall applet failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "\nInit Deinit Ref Count test passed\n") ;
}

/*---------------    Negative Tests --------------*/

void test_12_negative_test_events(JHI_HANDLE hJOM)
{
    JHI_SESSION_HANDLE hSession;
    JHI_SESSION_HANDLE openCloseSession;
    JHI_RET status;
    FILECHAR szCurDir [LEN_DIR];
    UINT8   txBuffer[EVENTS_BUFFER_SIZE] = {0x0},
            rxBuffer[EVENTS_BUFFER_SIZE] = {0x0};
    JVM_COMM_BUFFER txrx ;
    txrx.TxBuf->length = EVENTS_BUFFER_SIZE ;
    txrx.TxBuf->buffer = NULL;//txBuffer;
    txrx.RxBuf->length = EVENTS_BUFFER_SIZE ;
    txrx.RxBuf->buffer = rxBuffer;
    fill_buffer(txBuffer,EVENTS_BUFFER_SIZE);

    GetFullFilename(szCurDir, EVENT_SERVICE_FILENAME);
    test_event_raised = 0;
    test_event_buffer_match = 0;

#ifdef _WIN32
    callback_event = CreateEvent(NULL,FALSE,FALSE,NULL);
	if (callback_event == NULL)
	{
		fprintf( stderr, "Error: failed to create win32 event handle.\n");
		exit_test(EXIT_FAILURE);
	}
#else //!WIN32
    pthread_mutex_init(&callback_mutex, NULL);
    pthread_cond_init(&callback_cond, NULL);
#endif //WIN32

    fprintf( stderr, "\nInstalling the Event Service applet \n");

    // install the Event Service applet in JOM
    status = JHI_Install2( hJOM, EVENT_SERVICE_APP_ID, szCurDir);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // create a session, register it for events and close the session without unregistering
    fprintf( stderr, "\ncreate a session of Event Service\n");
    status = JHI_CreateSession(hJOM,EVENT_SERVICE_APP_ID,0,NULL,&openCloseSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // register for event from the Event Service session-->should fail
    fprintf( stderr, "register for event from the Event Service session\n");
    status = JHI_RegisterEvents(hJOM,(UINT32*)openCloseSession + 3,(JHI_EventFunc)onEvent);
    if (status == JHI_SUCCESS)
    {
        fprintf( stdout, "JHI register event succeeded, should fail!\n");
        exit_test(EXIT_FAILURE);
    }
    else if( status != JHI_INVALID_SESSION_HANDLE)
    {
        fprintf( stdout, "JHI create session failed with wrong error code, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    // register for event from the Event Service session-->should succeed
    status = JHI_RegisterEvents(hJOM,openCloseSession,(JHI_EventFunc)onEvent);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI register event failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // close the session
    fprintf( stderr, "close the session without calling unregister\n");
    status = JHI_CloseSession(hJOM,&openCloseSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // create a session of Event Service
    fprintf( stderr, "\ncreate a session of Event Service\n");
    status = JHI_CreateSession(hJOM,EVENT_SERVICE_APP_ID,0,NULL,&hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // register for event from the Event Service session
    fprintf( stderr, "register for event from the Event Service session\n");
    status = JHI_RegisterEvents(hJOM,hSession,(JHI_EventFunc)onEvent);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI register event failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // call send and recieve with command = 10 in order to invoke event by the Event Service
    fprintf( stderr, "call SAR2 with command = 10 in order to invoke event by the Event Service\n");
    status = JHI_SendAndRecv2(hJOM, hSession,10, &txrx,NULL);
    if (status == JHI_SUCCESS)
    {
        fprintf( stdout, "JHI send and recieve 2 succeeded, should fail!\n");
        exit_test(EXIT_FAILURE);
    }
    else if( status != JHI_INVALID_COMM_BUFFER)
    {
        fprintf( stdout, "JHI_SendAndRecv2 failed with wrong error code, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // unregister the event
    status = JHI_UnRegisterEvents(hJOM,hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI untegister event failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // close the session
    status = JHI_CloseSession(hJOM,&hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // uninstall the Event Service applet in JOM
    status = JHI_Uninstall (hJOM, EVENT_SERVICE_APP_ID);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall applet failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "\nevents negative test passed\n") ;
}

void test_13_negative_test_send_and_recieve(JHI_HANDLE hJOM)
{
    UINT8   txBuffer[BUFFER_SIZE] = {0x0},
            rxBuffer[BUFFER_SIZE] = {0x0};
    FILECHAR szCurDir [LEN_DIR];
    JVM_COMM_BUFFER txrx ;
    JHI_SESSION_HANDLE hSession;
    JHI_RET status;

    GetFullFilename(szCurDir, ECHO_FILENAME);

    // now install the echo applet in JOM
    fprintf( stderr, "\ninstalling the echo applet \n") ;
    status = JHI_Install2( hJOM, ECHO_APP_ID, szCurDir) ;
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // create a session
    fprintf( stderr, "creating session of the echo applet \n") ;
    status = JHI_CreateSession(hJOM,ECHO_APP_ID,0,NULL,&hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // send max buffer size
    txrx.TxBuf->length = BUFFER_SIZE ; // length
    txrx.TxBuf->buffer = txBuffer;
    txrx.RxBuf->length = BUFFER_SIZE ;
    txrx.RxBuf->buffer = rxBuffer ;

    fprintf( stderr, "Sending and receiving\n");
    status = JHI_SendAndRecv2(hJOM, (UINT32*)hSession + 5,0, &txrx,NULL);

    if( JHI_SUCCESS == status )
    {
        fprintf( stderr, "Send and receive succeeded, but should fail\n");
        fprintf( stderr, "\nError sending buffer with size %d, error code: 0x%x (%s)\n",BUFFER_SIZE, status , JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    else if( status != 0x100F)
    {
        fprintf( stdout, "JHI Send and receive failed with wrong error code, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // close the session
    status = JHI_CloseSession(hJOM,&hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    status = JHI_Uninstall (hJOM, ECHO_APP_ID);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall applet failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "Send and Recieve negative test passed\n") ;
}



void test_14_negative_test_get_applet_property(JHI_HANDLE hJOM)
{
    UINT8   rxBuffer[APP_PROPERTY_BUFFER_SIZE] = {0x0};
    FILECHAR szCurDir [LEN_DIR];
    JVM_COMM_BUFFER txrx ;
    JHI_RET status;
    const FILECHAR *AppProperty_Name = FILEPREFIX("applet.name");

    GetFullFilename(szCurDir, ECHO_FILENAME);

    // now install the echo applet in JOM
    fprintf( stderr, "\ninstalling the echo applet \n") ;
    status = JHI_Install2( hJOM, ECHO_APP_ID, szCurDir) ;
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stderr, "starting get applet property call\n\n");

    txrx.TxBuf->length = 0;
    txrx.TxBuf->buffer = NULL;
    txrx.RxBuf->length = APP_PROPERTY_BUFFER_SIZE - 1;
    txrx.RxBuf->buffer = rxBuffer ;

    status = JHI_GetAppletProperty(hJOM, ECHO_APP_ID, &txrx) ;
    if (status!= JHI_APPLET_PROPERTY_NOT_SUPPORTED)
    {
        fprintf( stdout, "Test failed! should have accepted JHI_APPLET_PROPERTY_NOT_SUPPORTED instead received error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "Received JHI_APPLET_PROPERTY_NOT_SUPPORTED as expected.\n");

    // try to send valid applet property with a short buffer
    memset(rxBuffer,0,APP_PROPERTY_BUFFER_SIZE);
    txrx.TxBuf->length = (uint32_t)FILECHARLEN(AppProperty_Name);
    txrx.TxBuf->buffer = const_cast<FILECHAR *>(AppProperty_Name);
    txrx.RxBuf->length = 0;
    txrx.RxBuf->buffer = rxBuffer;

    status = JHI_GetAppletProperty(hJOM, ECHO_APP_ID, &txrx) ;

    if (status != JHI_INSUFFICIENT_BUFFER || txrx.RxBuf->length != 11) // "echo applet" = 11 chars
    {
        fprintf( stdout, "Test failed! should have accepted JHI_INSUFFICIENT_BUFFER\n instead received error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "Received JHI_INSUFFICIENT_BUFFER as expected.\n");

    status = JHI_Uninstall (hJOM, ECHO_APP_ID);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall echo applet failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "\nGet Applet Property test passed\n") ;
}

void test_15_negative_test_get_version_info(JHI_HANDLE hJOM)
{
    JHI_RET status = JHI_GetVersionInfo(hJOM, NULL);
    if (status == JHI_SUCCESS)
    {
        fprintf( stdout, "\nJHI get version info succeeded, but it should fail\n");
        exit_test(EXIT_FAILURE);
    }

    if (status != 0x203)
    {
        fprintf( stdout, "JHI GetVersionInfo failed with wrong error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    fprintf( stdout, "\nJHI get version info passed\n") ;
}

void test_16_negative_test_install_applet(JHI_HANDLE hJOM)
{
    FILECHAR szCurDir [LEN_DIR];
    JHI_RET status;

    GetFullFilename(szCurDir, ECHO_FILENAME);

    // now install the echo applet in JOM
    fprintf( stderr, "\ninstalling the echo applet \n") ;
    status = JHI_Install2( (UINT32*)hJOM + 5, ECHO_APP_ID, szCurDir) ;
    if (status == JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install succeeded, but should fail\n");
        exit_test(EXIT_FAILURE);
    }
    if (status != 0x201)
    {
        fprintf( stdout, "JHI install failed, with wrong error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    status = JHI_Install2( NULL, ECHO_APP_ID, szCurDir) ;
    if (status == JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install succeeded, but should fail\n");
        exit_test(EXIT_FAILURE);
    }

    if (status != 0x201)
    {
        fprintf( stdout, "JHI install failed, with wrong error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "Bad handle test passed\n") ;

#if 0 // this test is incorrect on CSE
    // now install the echo applet in JOM
	fprintf( stderr, "\ninstalling the echo applet with echo4 APP_ID \n") ;
	status = JHI_Install2( (UINT32*)hJOM, getEchoUuid(4).c_str(), szCurDir) ;
	if (status == JHI_SUCCESS) {
		fprintf( stdout, "JHI install succeeded, but should fail\n");
		exit_test(EXIT_FAILURE);
	} else if (status == 0x2001) { //incompatible guid
		fprintf( stdout, "Incompatible Applet GUID test passed\n");
	} else {
		fprintf( stdout, "Incompatible Applet GUID test failed with wrong error, return status 0x%x (%s)\n",
			 status, JHIErrorToString(status));
	}
#endif

}

void test_17_list_installed_applets()
{
	JHI_RET jhiStatus;
	TEE_STATUS teeStatus;
	SD_SESSION_HANDLE sdSession;
	UUID_LIST uuidList;

	uint8_t appletsCount = 5;

	FILECHAR szCurDir [LEN_DIR];

	const char *intelSD = INTEL_SD_UUID;

	fprintf( stdout, "\nStarting JHI list installed applets test...\n");

	//check FW version
	VERSION version;
	if (!getFWVersion(&version))
	{
		fprintf( stdout, "Get version failed, aborting test.\n");
		exit_test(EXIT_FAILURE);
	}
	if (version.Major < 11 && version.Major != 3)
	{
		fprintf( stdout, "FW isn't CSE or BXT.\n");
		teeStatus = TEE_OpenSDSession(intelSD, &sdSession);
		if (teeStatus != TEE_STATUS_UNSUPPORTED_PLATFORM)
		{
			fprintf( stdout, "Wrong error code recieved from TEE_OpenSDSession, error code: 0x%x (%s), expected 0x%x (TEE_STATUS_UNSUPPORTED_PLATFORM).\n", teeStatus, TEEErrorToString(teeStatus), TEE_STATUS_UNSUPPORTED_PLATFORM);
			exit_test(EXIT_FAILURE);
		}
		teeStatus = TEE_ListInstalledTAs(sdSession, &uuidList);
		if (teeStatus != TEE_STATUS_UNSUPPORTED_PLATFORM)
		{
			fprintf( stdout, "Wrong error code recieved from TEE_ListInstalledTAs, error code: 0x%x (%s), expected 0x%x (TEE_STATUS_UNSUPPORTED_PLATFORM).\n", teeStatus, TEEErrorToString(teeStatus), TEE_STATUS_UNSUPPORTED_PLATFORM);
			exit_test(EXIT_FAILURE);
		}
		return;
	}

	teeStatus = TEE_OpenSDSession(intelSD, &sdSession);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf( stdout, "TEE_OpenSDSession failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

	// Getting the installes TA list
	teeStatus = TEE_ListInstalledTAs(sdSession, &uuidList);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf( stdout, "TEE_ListInstalledTAs failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

	// validate the uuid list
	if (!validateUuidList(&uuidList))
	{
		fprintf( stdout, "uuidList validation failed.\n");
		exit_test(EXIT_FAILURE);
	}

	printUUIDs(uuidList);

	if (uuidList.uuidCount > 1)
	{
		fprintf( stdout, "TEE_ListInstalledTAs, UUID count is %d, where it should be 1.\nUninstalling extra TAs\n", uuidList.uuidCount);
		for (int i=0; i<uuidList.uuidCount; ++i)
		{
#ifdef _WIN32
			if (_stricmp(SPOOLER_APP_ID, uuidList.uuids[i]) != 0)
#else
			if (strcasecmp(SPOOLER_APP_ID, uuidList.uuids[i]) != 0)
#endif
			{
				fprintf( stdout, "JHI uninstalling ta %s...\n", uuidList.uuids[i]);
				jhiStatus = JHI_Uninstall(hJOM, uuidList.uuids[i]);
				if (jhiStatus != JHI_SUCCESS)
				{
					fprintf( stdout, "JHI uninstall ta %s failed, error code: 0x%x (%s)\n", uuidList.uuids[i], jhiStatus, JHIErrorToString(jhiStatus));
					exit_test(EXIT_FAILURE);
				}
			}
		}

		TEE_DEALLOC(uuidList.uuids);

		// Getting the installes TA list again
		teeStatus = TEE_ListInstalledTAs(sdSession, &uuidList);
		if (teeStatus != TEE_STATUS_SUCCESS)
		{
			fprintf( stdout, "TEE_ListInstalledTAs failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
			exit_test(EXIT_FAILURE);
		}

		// validate the uuid list
		if (!validateUuidList(&uuidList))
		{
			fprintf( stdout, "uuidList validation failed.\n");
			exit_test(EXIT_FAILURE);
		}
	}

#ifdef _WIN32
	if (_stricmp(SPOOLER_APP_ID, uuidList.uuids[0]) != 0)
#else
	if (strcasecmp(SPOOLER_APP_ID, uuidList.uuids[0]) != 0)
#endif
	{
		fprintf( stdout, "uuidList doesn't match the expected results\nExpected - %s, Received - %s.\n", SPOOLER_APP_ID, uuidList.uuids[0]);
		exit_test(EXIT_FAILURE);
	}

	fprintf( stdout, "\nTEE_ListInstalledTAs succeeded.\n");

	// free the buffer
	TEE_DEALLOC(uuidList.uuids);

	fprintf( stdout, "\n");

	// Install 5 TAs
	for (int i = 1; i <= appletsCount; ++i)
	{
		GetFullFilename(szCurDir, getEchoFileName(i).c_str());
		fprintf( stdout, "JHI installing applet %d...", i);
		jhiStatus = JHI_Install2( hJOM, getEchoUuid(i).c_str(), szCurDir) ;
		if (jhiStatus != JHI_SUCCESS)
		{
			fprintf( stdout, "\nJHI install echo %d failed, error code: 0x%x (%s)\n", i, jhiStatus, JHIErrorToString(jhiStatus));
			exit_test(EXIT_FAILURE);
		}
		fprintf( stdout, " succeeded\n");
	}

	// Getting the installes TA list
	teeStatus = TEE_ListInstalledTAs(sdSession, &uuidList);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf( stdout, "TEE_ListInstalledTAs failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}
	// validate the uuid list
	if (!validateUuidList(&uuidList))
	{
		fprintf( stdout, "uuidList validation failed.\n");
		exit_test(EXIT_FAILURE);
	}

	if (uuidList.uuidCount != 6)
	{
		fprintf( stdout, "TEE_ListInstalledTAs failed, UUID count is %d, where it should be 6.\n", uuidList.uuidCount);
		exit_test(EXIT_FAILURE);
	}

	fprintf( stdout, "\nTEE_ListInstalledTAs succeeded.\n");

	printUUIDs(uuidList);

	// free the buffer
	TEE_DEALLOC(uuidList.uuids);

	fprintf( stdout, "\n");

	// Uninstall the echo applets
	for (int i = 1; i <= appletsCount; ++i)
	{
		// now install the echo_1 applet in JOM
		fprintf( stdout, "JHI uninstalling applet %d...", i);
		jhiStatus = JHI_Uninstall( hJOM, (char*)getEchoUuid(i).c_str());
		if (jhiStatus != JHI_SUCCESS)
		{
			fprintf( stdout, "\nJHI uninstall echo %d failed, error code: 0x%x (%s)\n", i, jhiStatus, JHIErrorToString(jhiStatus));
			exit_test(EXIT_FAILURE);
		}
		fprintf( stdout, " succeeded\n");
	}

	fprintf( stdout, "\n");

	// Getting the installes TA list
	teeStatus = TEE_ListInstalledTAs(sdSession, &uuidList);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf( stdout, "TEE_ListInstalledTAs failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

	// validate the uuid list
	if (!validateUuidList(&uuidList))
	{
		fprintf( stdout, "uuidList validation failed.\n");
		exit_test(EXIT_FAILURE);
	}

	if (uuidList.uuidCount != 1)
	{
		fprintf( stdout, "TEE_ListInstalledTAs failed, UUID count is %d, where it should be 1.\n", uuidList.uuidCount);
		exit_test(EXIT_FAILURE);
	}

	fprintf( stdout, "TEE_ListInstalledTAs succeeded.\n");

	printUUIDs(uuidList);

	// free the buffer
	TEE_DEALLOC(uuidList.uuids);

	fprintf( stdout, "\nJHI list installed applets test passed\n") ;

	teeStatus = TEE_CloseSDSession(&sdSession);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf( stdout, "TEE_CloseSDSession failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}
}

void test_18_admin_install_uninstall()
{
	TEE_STATUS teeStatus;
	SD_SESSION_HANDLE sdSession;
	FILECHAR echoInstallAcp [LEN_DIR];
	FILECHAR echoUninstallAcp [LEN_DIR];
	vector<uint8_t> installBlob, uninstallBlob;
	const char *intelSD = INTEL_SD_UUID;

	fprintf( stdout, "\nStarting JHI admin install / uninstall applets test...\n");

	//check FW version
	VERSION version;
	if (!getFWVersion(&version))
	{
		fprintf( stdout, "Get version failed, aborting test.\n");
		exit_test(EXIT_FAILURE);
	}
	if (version.Major < 11 && version.Major != 3)
	{
		fprintf( stdout, "FW isn't CSE or BXT, skipping test.\n");
		return;
	}

	teeStatus = TEE_OpenSDSession(intelSD, &sdSession);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf( stdout, "TEE_OpenSDSession failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

	//set the full paths
	GetFullFilename(echoInstallAcp, ECHO_ACP_INSTALL_FILENAME);
	GetFullFilename(echoUninstallAcp, ECHO_ACP_UNINSTALL_FILENAME);

	//reading the ACPs into the blobs.
	teeStatus = readFileAsBlob(echoInstallAcp, installBlob);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		cout << "readFileAsBlob failed to read install acp at " << echoInstallAcp << ", error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	teeStatus = readFileAsBlob(echoUninstallAcp, uninstallBlob);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		cout << "readFileAsBlob failed to read uninstall acp at " << echoUninstallAcp << ", error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}
	// install the echo applet in acp format.
	teeStatus = TEE_SendAdminCmdPkg(sdSession, &installBlob[0], (uint32_t) installBlob.size()) ;
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf( stdout, "TEE_SendAdminCmdPkg failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

	// uninstall the echo applet in acp format.
	teeStatus = TEE_SendAdminCmdPkg(sdSession, &uninstallBlob[0], (uint32_t) uninstallBlob.size()) ;
	if (teeStatus != JHI_SUCCESS)
	{
		fprintf( stdout, "TEE_SendAdminCmdPkg failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

	fprintf( stdout, "\nTEE_SendAdminCmdPkg test passed\n") ;
}

void test_19_admin_install_with_session(JHI_HANDLE hJOM)
{
    JHI_SESSION_HANDLE hSession;
    JHI_RET jhiStatus;
    TEE_STATUS teeStatus;
    UINT32 session_count;
    JHI_SESSION_INFO info;
    FILECHAR szCurDir [LEN_DIR];
    UINT8 buffer[5] = { 0x01,0x02,0x03,0x04,0x05 };
    DATA_BUFFER initData;

    initData.buffer = buffer;
    initData.length = 5;

    SD_SESSION_HANDLE sdSession;
    vector<uint8_t> blob;
    const char *intelSD = INTEL_SD_UUID;

    fprintf( stdout, "\nStarting JHI admin install with session test...\n");

    //check FW version
    VERSION version;
    if (!getFWVersion(&version))
    {
        fprintf( stdout, "Get version failed, aborting test.\n");
        exit_test(EXIT_FAILURE);
    }
    if (version.Major < 11 && version.Major != 3)
    {
        fprintf( stdout, "FW isn't CSE or BXT, skipping test.\n");
        return;
    }

    vector<string> uuids;

    teeStatus = TEE_OpenSDSession(intelSD, &sdSession);
    if (teeStatus != TEE_STATUS_SUCCESS)
    {
        fprintf( stdout, "TEE_OpenSDSession failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
        exit_test(EXIT_FAILURE);
    }

    // now install the echo_1 applet in JOM
    GetFullFilename(szCurDir, ECHO_ACP_INSTALL_FILENAME);

    jhiStatus = readFileAsBlob(szCurDir, blob);
    if (jhiStatus != JHI_SUCCESS)
    {
		cout << "readFileAsBlob failed to read install acp at " << szCurDir << ", error code: 0x" << hex << jhiStatus << "(" << JHIErrorToString(jhiStatus) << ")" << endl;
        exit_test(EXIT_FAILURE);
    }

    teeStatus = TEE_SendAdminCmdPkg(sdSession, &blob[0], (uint32_t) blob.size()) ;
    if (teeStatus != TEE_STATUS_SUCCESS)
    {
        fprintf( stdout, "TEE_SendAdminCmdPkg failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
        exit_test(EXIT_FAILURE);
    }

    // create a session
    jhiStatus = JHI_CreateSession(hJOM,ECHO_APP_ID,0,&initData,&hSession);
    if (jhiStatus != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", jhiStatus, JHIErrorToString(jhiStatus));
        exit_test(EXIT_FAILURE);
    }

    jhiStatus = JHI_GetSessionsCount(hJOM,ECHO_APP_ID,&session_count);
    if (jhiStatus != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI GetSessionsCount failed, error code: 0x%x (%s)\n", jhiStatus, JHIErrorToString(jhiStatus));
        exit_test(EXIT_FAILURE);
    }
    if (session_count != 1)
    {
        fprintf( stdout, "error: session count should be 1\n") ;
        exit_test(EXIT_FAILURE);
    }

    // get session status

    jhiStatus = JHI_GetSessionInfo(hJOM,hSession,&info);
    if (jhiStatus != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI Get Session Status failed, error code: 0x%x (%s)\n", jhiStatus, JHIErrorToString(jhiStatus));
        exit_test(EXIT_FAILURE);
    }
    if (info.state != JHI_SESSION_STATE_ACTIVE)
    {
        fprintf( stdout, "error: session status should be SESSION_ACTIVE(1)\n") ;
        exit_test(EXIT_FAILURE);
    }

    // close the session
    jhiStatus = JHI_CloseSession(hJOM,&hSession);
    if (jhiStatus != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", jhiStatus, JHIErrorToString(jhiStatus));
        exit_test(EXIT_FAILURE);
    }

    jhiStatus = JHI_GetSessionsCount(hJOM,ECHO_APP_ID,&session_count);
    if (jhiStatus != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI GetSessionsCount failed, error code: 0x%x (%s)\n", jhiStatus, JHIErrorToString(jhiStatus));
        exit_test(EXIT_FAILURE);
    }
    if (session_count != 0)
    {
        fprintf( stdout, "error: session count should be 0\n") ;
        exit_test(EXIT_FAILURE);
    }

    jhiStatus = JHI_Uninstall (hJOM, ECHO_APP_ID);
    if (jhiStatus != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall applet failed, error code: 0x%x (%s)\n", jhiStatus, JHIErrorToString(jhiStatus));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "\nSessions test passed\n") ;
}

void test_20_admin_updatesvl()
{
	TEE_STATUS teeStatus;
	SD_SESSION_HANDLE sdSession;
	FILECHAR echoUpdatesvlAcp[LEN_DIR];
	vector<uint8_t> updatesvlBlob;
	const char *intelSD = INTEL_SD_UUID;

	fprintf(stdout, "\nStarting JHI admin UpdateSVL test...\n");

	//check FW version
	VERSION version;
	if (!getFWVersion(&version))
	{
		fprintf(stdout, "Get version failed, aborting test.\n");
		exit_test(EXIT_FAILURE);
	}
	if (version.Major < 11 && version.Major != 3)
	{
		fprintf(stdout, "FW isn't CSE or BXT, skipping test.\n");
		return;
	}

	teeStatus = TEE_OpenSDSession(intelSD, &sdSession);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf(stdout, "TEE_OpenSDSession failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

	//set the full paths
	GetFullFilename(echoUpdatesvlAcp, ECHO_ACP_UPDATESVL_FILENAME);

	//reading the ACP into the blobs.
	teeStatus = readFileAsBlob(echoUpdatesvlAcp, updatesvlBlob);
	if (teeStatus != TEE_STATUS_SUCCESS) 
	{
		cout << "readFileAsBlob failed to read UpdateSVL acp at " << echoUpdatesvlAcp << ", error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	// Send UpdateSVL
	teeStatus = TEE_SendAdminCmdPkg(sdSession, &updatesvlBlob[0], (uint32_t)updatesvlBlob.size());
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf(stdout, "TEE_SendAdminCmdPkg failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}
	
	fprintf(stdout, "\nTEE_SendAdminCmdPkg test passed\n");
}

void test_21_admin_query_tee_metadata()
{
	TEE_STATUS teeStatus;

	fprintf(stdout, "\nStarting JHI admin QueryTeeMetadata test...\n");

	//check FW version
	VERSION version;
	if (!getFWVersion(&version))
	{
		fprintf(stdout, "Get version failed, aborting test.\n");
		exit_test(EXIT_FAILURE);
	}
	if (version.Major < 11 && version.Major != 3)
	{
		fprintf(stdout, "FW isn't CSE or BXT, skipping test.\n");
		return;
	}

	dal_tee_metadata metadata;
	// Call the API
	teeStatus = TEE_QueryTEEMetadata(NULL, &metadata);

	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf(stdout, "TEE_QueryTEEMetadata failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

	fprintf(stdout, "\nTEE_QueryTEEMetadata test passed\n");
}

void test_22_oem_signing()
{
	TEE_STATUS teeStatus;
	SD_SESSION_HANDLE intelSdSession, oemSdSession;
	FILECHAR installSdAcp[LEN_DIR];
	FILECHAR uninstallSdAcp[LEN_DIR];
    FILECHAR installAppletAcp[LEN_DIR];
    FILECHAR uninstallAppletAcp[LEN_DIR];
    vector<uint8_t> installSdBlob, uninstallSdBlob, installAppletBlob, uninstallAppletBlob;
	const char *intelSD = INTEL_SD_UUID;
    const char *oemSD = OEM_SD_UUID;
    UUID_LIST uuidList;

	fprintf(stdout, "\nStarting OEM signing test...\n");

	// Is OEM signing supported?
	if (get_vm_type() != JHI_VM_TYPE_BEIHAI_V2)
	{
		cout << "VM is not BHv2. OEM signing is not supported" << endl;
		exit_test(EXIT_FAILURE);
	}

	teeStatus = TEE_OpenSDSession(intelSD, &intelSdSession);

	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf(stdout, "TEE_OpenSDSession failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

	//set the full paths
	GetFullFilename(installSdAcp, ACP_INSTALL_SD_FILENAME);
	GetFullFilename(uninstallSdAcp, ACP_UNINSTALL_SD_FILENAME);
    GetFullFilename(installAppletAcp, ACP_INSTALL_SD_APPLET_FILENAME);
    GetFullFilename(uninstallAppletAcp, ACP_UNINSTALL_SD_APPLET_FILENAME);

	//reading the ACPs into the blobs.
	teeStatus = readFileAsBlob(installSdAcp, installSdBlob);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		cout << "readFileAsBlob failed to read install acp at " << installSdAcp << ", error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	teeStatus = readFileAsBlob(uninstallSdAcp, uninstallSdBlob);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		cout << "readFileAsBlob failed to read uninstall acp at " << uninstallSdAcp << ", error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

    teeStatus = readFileAsBlob(installAppletAcp, installAppletBlob);
    if (teeStatus != TEE_STATUS_SUCCESS)
    {
		cout << "readFileAsBlob failed to read install applet acp at " << installAppletAcp << ", error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
        exit_test(EXIT_FAILURE);
    }

    teeStatus = readFileAsBlob(uninstallAppletAcp, uninstallAppletBlob);
    if (teeStatus != TEE_STATUS_SUCCESS)
    {
		cout << "readFileAsBlob failed to read uninstall applet acp at " << uninstallAppletAcp << ", error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
        exit_test(EXIT_FAILURE);
    }

    // First, uninstall the OEM SD if it was previously installed
    fprintf(stdout, "Uninstalling the OEM SD if it was previously installed...\n");
    teeStatus = TEE_SendAdminCmdPkg(intelSdSession, &uninstallSdBlob[0], (uint32_t)uninstallSdBlob.size());
    if (teeStatus != JHI_SUCCESS)
    {
        fprintf(stdout, "TEE_SendAdminCmdPkg failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
    }

	// Install OEM SD
    fprintf(stdout, "Installing the OEM SD...\n");
	teeStatus = TEE_SendAdminCmdPkg(intelSdSession, &installSdBlob[0], (uint32_t)installSdBlob.size());
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf(stdout, "TEE_SendAdminCmdPkg failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

    // Open OEM SD session
    fprintf(stdout, "Openning an OEM SD session...\n");
    teeStatus = TEE_OpenSDSession(oemSD, &oemSdSession);
    if (teeStatus != TEE_STATUS_SUCCESS)
    {
        fprintf(stdout, "TEE_OpenSDSession with OEM SD failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
        exit_test(EXIT_FAILURE);
    }

    // Install OEM signed applet
    fprintf(stdout, "Installing an OEM signed applet...\n");
    teeStatus = TEE_SendAdminCmdPkg(oemSdSession, &installAppletBlob[0], (uint32_t)installAppletBlob.size());
    if (teeStatus != TEE_STATUS_SUCCESS)
    {
        fprintf(stdout, "TEE_SendAdminCmdPkg failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
        exit_test(EXIT_FAILURE);
    }

    // (needed?)
    // Send and receive to and from the OEM signed applet

    // List installed TAs of the OEM SD
    fprintf(stdout, "Checking the number of installed OEM signed applets...\n");
    teeStatus = TEE_ListInstalledTAs(oemSdSession, &uuidList);
    if (teeStatus != TEE_STATUS_SUCCESS)
    {
        fprintf(stdout, "TEE_ListInstalledTAs failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
        exit_test(EXIT_FAILURE);
    }

    if(uuidList.uuidCount != 1)
    {
        fprintf(stdout, "OEM installed TAs number is not 1 as expected but %d. Aborting...\n", uuidList.uuidCount);
		JHI_DEALLOC(uuidList.uuids);
        exit_test(EXIT_FAILURE);
    }

	JHI_DEALLOC(uuidList.uuids);

    // Uninstall OEM signed applet
    fprintf(stdout, "Uninstalling the OEM signed applet...\n");
    teeStatus = TEE_SendAdminCmdPkg(oemSdSession, &uninstallAppletBlob[0], (uint32_t)uninstallAppletBlob.size());
    if (teeStatus != TEE_STATUS_SUCCESS)
    {
        fprintf(stdout, "TEE_SendAdminCmdPkg failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
        exit_test(EXIT_FAILURE);
    }

	// List installed TAs of the OEM SD
	fprintf(stdout, "Checking the number of installed OEM signed applets...\n");
	teeStatus = TEE_ListInstalledTAs(oemSdSession, &uuidList);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf(stdout, "TEE_ListInstalledTAs failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

	if(uuidList.uuidCount != 0)
	{
		fprintf(stdout, "OEM installed TAs number is not 0 as expected but %d. Aborting...\n", uuidList.uuidCount);
		JHI_DEALLOC(uuidList.uuids);
		exit_test(EXIT_FAILURE);
	}

	JHI_DEALLOC(uuidList.uuids);

    // Close OEM SD session
    fprintf(stdout, "Closing the OEM SD session...\n");
    teeStatus = TEE_CloseSDSession(&oemSdSession);
    if (teeStatus != TEE_STATUS_SUCCESS)
    {
        fprintf(stdout, "TEE_CloseSDSession with OEM SD failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
        exit_test(EXIT_FAILURE);
    }

	// Uninstall OEM SD
    fprintf(stdout, "Uninstalling the OEM SD...\n");
	teeStatus = TEE_SendAdminCmdPkg(intelSdSession, &uninstallSdBlob[0], (uint32_t)uninstallSdBlob.size());
	if (teeStatus != JHI_SUCCESS)
	{
		fprintf(stdout, "TEE_SendAdminCmdPkg failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

    teeStatus = TEE_CloseSDSession(&intelSdSession);

	fprintf(stdout, "\nOEM signing test passed\n");
}

void test_23_applet_encryption(JHI_HANDLE hJOM)
{
	TEE_STATUS teeStatus;
	JHI_RET jhiStatus;
	const char *intelSD = INTEL_SD_UUID;
	const char *oemSD   = OEM_SD_UUID;
	const uint8_t oemSdBin[TEE_SDID_LENGTH] = OEM_SD_UUID_BIN;
	SD_SESSION_HANDLE intelSdSession, oemSdSession;
	vector<uint8_t> installSdBlob, uninstallSdBlob, installAppletBlob, uninstallAppletBlob;
	FILECHAR installSdAcp[LEN_DIR];
	FILECHAR uninstallSdAcp[LEN_DIR];
	FILECHAR installAppletAcp[LEN_DIR];
	FILECHAR uninstallAppletAcp[LEN_DIR];
	JHI_SESSION_HANDLE hSession;
	JVM_COMM_BUFFER comm_buf;
	int32_t responseCode = 99999;

	// Is TA encryption supported?
	if (get_vm_type() != JHI_VM_TYPE_BEIHAI_V2)
	{
		cout << "VM is not BHv2. Applet Encryption is not supported" << endl;
		exit_test(EXIT_FAILURE);
	}

	// Set full paths
	GetFullFilename(installSdAcp, ACP_INSTALL_SD_FILENAME);
	GetFullFilename(uninstallSdAcp, ACP_UNINSTALL_SD_FILENAME);
	GetFullFilename(installAppletAcp, ECHO_ENCRYPTED_FILENAME);
	GetFullFilename(uninstallAppletAcp, ECHO_ACP_UNINSTALL_FILENAME);

	// Read ACPs into blobs
	cout << "Reading applet blobs..." << endl;
	teeStatus = readFileAsBlob(installSdAcp, installSdBlob);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		cout << "readFileAsBlob failed. Failed to read install SD acp at " << installSdAcp << ", error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	teeStatus = readFileAsBlob(uninstallSdAcp, uninstallSdBlob);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		cout << "readFileAsBlob failed. Falied to read uninstall SD acp at " << uninstallSdAcp << ", error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	teeStatus = readFileAsBlob(installAppletAcp, installAppletBlob);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		cout << "readFileAsBlob failed. Failed to read install applet acp at " << installAppletAcp << ", error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	teeStatus = readFileAsBlob(uninstallAppletAcp, uninstallAppletBlob);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		cout << "readFileAsBlob failed. Failed to read uninstall applet acp at " << uninstallAppletAcp << ", error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	// Open an Intel SD session
	cout << "Opening an Intel SD session..." << endl;
	teeStatus = TEE_OpenSDSession(intelSD, &intelSdSession);

	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf(stdout, "TEE_OpenSDSession failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

	// First, uninstall the OEM SD if it was previously installed
	cout << "Uninstalling the OEM SD if it was previously installed...\n";
	teeStatus = TEE_SendAdminCmdPkg(intelSdSession, &uninstallSdBlob[0], (uint32_t)uninstallSdBlob.size());
	if (teeStatus != JHI_SUCCESS)
	{
		fprintf(stdout, "SD uninstallation (TEE_SendAdminCmdPkg) failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
	}

	// Install OEM SD
	cout << "Installing the OEM SD...\n";
	teeStatus = TEE_SendAdminCmdPkg(intelSdSession, &installSdBlob[0], (uint32_t)installSdBlob.size());
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf(stdout, "SD installation (TEE_SendAdminCmdPkg) failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

	// Provision OMK
	cout << "Provisioning the OEM master key..." << endl;
	
	const uint32_t asym_key_length = 256;

	const uint8_t n[asym_key_length] = { 0x27,0xFE,0x26,0xB3,0xFF,0x34,0x31,0x54,0xBD,0xAF,0x65,0xF3,0xF5,0xDA,0x59,0xB0,0xC7,0xD9,0x24,0xBE,0x9A,0xC9,0x5C,0xF6,0x0E,0xC2,0x0C,0x53,0x7C,0x88,0xA9,0xAD,0xD3,0xE1,0xDA,0x54,0x47,0x33,0x3C,0x68,0xE7,0xEF,0x1A,0x2B,0x57,0xC0,0x19,0x9B,0x30,0x3D,0x2D,0x0E,0xB0,0x08,0x90,0xAF,0x93,0x06,0x81,0xD9,0xE0,0x8A,0xE9,0xC9,0x5F,0x58,0x7C,0xFE,0x4D,0xA4,0xBA,0x3A,0x9F,0x77,0x71,0xBB,0x30,0x2F,0x08,0x75,0x7C,0x74,0x33,0xC8,0xFE,0xC1,0x2D,0xBA,0x24,0xF7,0x61,0xC4,0x0E,0xFB,0x79,0xB5,0x5C,0xE0,0x08,0xD9,0xA1,0x3A,0x29,0xBB,0x2E,0x0F,0x48,0xD4,0x80,0xE5,0xDB,0x3B,0xE7,0x2B,0x2E,0x89,0x13,0xD2,0x65,0xB3,0x1F,0x29,0x17,0x4A,0xB8,0x05,0x2B,0xF4,0x09,0xD0,0x07,0x2F,0x6D,0xEE,0x3C,0x81,0xFE,0x3A,0x79,0xCF,0x5B,0xAE,0x00,0x7E,0x51,0x76,0x15,0xC5,0x5F,0x68,0x1B,0x36,0x44,0x3A,0x77,0x8B,0xB3,0x2D,0x76,0x87,0xC5,0x2E,0x4E,0xFE,0x0F,0x03,0x9E,0xA0,0x77,0x82,0x9C,0x8A,0xD8,0x8C,0xBF,0x46,0x0A,0x77,0x57,0x36,0x90,0x04,0xF1,0x1C,0xD7,0x32,0x72,0x08,0x49,0x5B,0x91,0x88,0x6E,0xBC,0xEB,0x25,0xD4,0xA2,0x51,0xCB,0x68,0x9D,0xA7,0xDF,0x7C,0x3B,0xC2,0x64,0xF7,0xDE,0xE0,0x9B,0xB6,0x61,0x97,0x3B,0x2F,0x28,0xD4,0x04,0xA3,0x20,0x58,0x0C,0x4C,0x8F,0x9B,0xB3,0xF9,0xC5,0xA2,0x42,0x93,0x4D,0xDF,0x83,0xFF,0x5D,0x12,0xC4,0x98,0xEC,0x42,0xCB,0x50,0xE4,0x4E,0x6C,0x4C,0xF6,0xEC,0x36,0x01,0x3D,0x8A,0xCA };
	const uint8_t d[asym_key_length] = { 0x31,0x89,0x09,0xde,0x8e,0x6c,0x5c,0xbb,0xb8,0x27,0x44,0x49,0xc4,0xb5,0xac,0xbf,0x96,0x82,0x92,0x0f,0x85,0x31,0x4d,0x76,0x14,0xea,0xbb,0x6b,0x25,0x2a,0x85,0xc0,0x18,0x87,0xaa,0xc3,0x0e,0x2d,0x01,0x1f,0x57,0xe6,0xc2,0x36,0x8c,0xed,0x34,0xa9,0x07,0x3f,0x35,0xe8,0x16,0xb8,0x93,0x7e,0xb6,0x28,0xe9,0x47,0x0d,0x8d,0x43,0xe8,0x76,0x87,0x04,0xe2,0xca,0x19,0xa0,0x36,0x3e,0x82,0xea,0x9c,0x90,0xc6,0x24,0xae,0xa1,0x1d,0xbe,0x40,0xa9,0x16,0xfd,0x21,0x0e,0x4b,0xbc,0xd0,0x5c,0xfa,0xf7,0x27,0x10,0x0b,0xb7,0x4f,0x40,0x34,0x85,0x85,0x2d,0x08,0xfa,0xff,0xed,0xbe,0x62,0x39,0x38,0x1a,0xb3,0xb3,0xb5,0x99,0x2c,0x9f,0xf8,0x7d,0xcf,0x4e,0xe5,0x5b,0x7d,0x1b,0x64,0x1f,0x81,0x0d,0x9d,0x68,0x27,0xfc,0x45,0x3c,0x6d,0xdd,0x00,0x97,0x8c,0xe4,0x4e,0x3a,0x2b,0x13,0x47,0xd1,0x7f,0xc1,0x28,0xa2,0xed,0x50,0xb1,0x52,0x9a,0xb7,0x53,0xed,0xdc,0xce,0x31,0xb2,0xe3,0xe5,0xc4,0x26,0x51,0xc2,0x81,0xde,0x77,0xb0,0xc4,0x16,0xd1,0x2c,0xce,0x08,0x64,0xff,0x25,0x69,0x0c,0x8f,0x3a,0x5a,0x23,0xd7,0x9c,0x7f,0x63,0xa3,0x0f,0x17,0x6d,0x22,0x14,0x04,0x36,0x85,0x92,0xf1,0x38,0x67,0xe6,0x21,0xb1,0x52,0xc2,0xea,0xbe,0x27,0xaf,0x80,0x86,0x87,0x0c,0xf5,0x62,0xd6,0xfd,0xa2,0x0b,0x7f,0x6c,0x3f,0x49,0xd8,0xf6,0x71,0x92,0x3a,0xed,0x98,0x92,0x46,0x6c,0x05,0x10,0xc4,0xd5,0x63,0x7e,0x79,0x7d,0xb4,0x85,0xc8,0x33,0x97,0x75,0x9d };
	
	tee_asym_key_material asym_key;
	memset(&asym_key, 0x00, sizeof(asym_key));

	asym_key.initial_counter = 1;
	asym_key.alg_type = tee_encryption_alg_pkcs;
	asym_key.key_length = asym_key_length;
	asym_key.public_exponent = 65537; // E
	memcpy(asym_key.modulus, n, asym_key_length); // N
	memcpy(asym_key.private_exponent, d, asym_key_length); // D

	teeStatus = TEE_ProvisionOemMasterKey(intelSdSession, &asym_key);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		fprintf(stdout, "TEE_ProvisionOemMasterKey failed, error code: 0x%x (%s)\n", teeStatus, TEEErrorToString(teeStatus));
		exit_test(EXIT_FAILURE);
	}

	// Set TA encryption key
	cout << "Setting a TA encryption key..." << endl;
	// The key, unencrypted: 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
	// IV:                   6d792073616d706c65204956
	const uint32_t sym_key_length = 256;
	const uint8_t sym_key_enc[sym_key_length]                = { 0x79,0x31,0xf9,0xb4,0xf4,0x28,0xb7,0x86,0xf4,0x65,0xbb,0x5f,0x95,0x5e,0x92,0x71,0x91,0x0e,0x31,0x72,0x10,0x0b,0x78,0xb3,0xc0,0xfe,0xf4,0x55,0x05,0x03,0x54,0xd6,0xc3,0x5a,0x1d,0x42,0x67,0xd5,0x10,0x7b,0x90,0x96,0x4c,0xb6,0xaa,0x2a,0x67,0x0a,0xf7,0xbe,0x3d,0xcd,0x82,0x78,0x70,0x64,0xd2,0xf1,0xae,0xee,0x41,0x5b,0x9c,0x14,0xf9,0x80,0xb9,0xa7,0xe3,0x13,0xcc,0xf8,0xf8,0x5b,0x8e,0x7c,0x42,0x0b,0xf9,0x4b,0x2a,0xbb,0x1c,0x28,0x5f,0xc6,0xc7,0x5f,0x2f,0xc5,0x06,0x8f,0xa7,0x9a,0xa5,0x26,0x57,0xab,0x62,0x95,0x82,0xd1,0x26,0x43,0x46,0x98,0xb8,0xb5,0x26,0x68,0x23,0x4b,0x58,0x70,0x57,0x25,0x6e,0xb9,0xc3,0x5a,0x99,0x77,0x2c,0x97,0xba,0x29,0x59,0x60,0x2b,0x41,0xf1,0x04,0x16,0xd8,0x0b,0x52,0xc8,0x23,0x64,0x21,0xbd,0x96,0xa7,0x6e,0x28,0x1f,0x22,0x1d,0x1c,0x32,0x9c,0xa2,0x4a,0x35,0x01,0xaa,0xdf,0x1e,0xcc,0x82,0x87,0xbb,0x7f,0x1d,0xe0,0x8c,0x82,0x09,0x8f,0x3f,0xb1,0xa5,0xca,0xb7,0xc2,0xfb,0x06,0x3e,0x62,0xd7,0xe5,0xa8,0xb2,0x09,0x7d,0x04,0x09,0xf2,0x0a,0xb0,0x36,0x0b,0x37,0xb0,0x00,0x80,0x78,0x8a,0x40,0x03,0x05,0xa2,0xd6,0x7a,0x01,0x7d,0x8f,0xdb,0xca,0xe2,0xba,0x65,0xa7,0x29,0xb6,0x47,0x06,0x75,0x15,0x84,0x9f,0x16,0x4d,0xf1,0x06,0x50,0x0e,0x13,0xd2,0xfd,0xa8,0x20,0x5b,0x15,0x32,0x7b,0xe0,0x16,0x9a,0x1e,0xc8,0x99,0x81,0x3f,0x93,0xfb,0xdc,0x5f,0x2d,0x79,0x67,0x06,0xe9,0xb6,0x3d,0x12 };
	const uint8_t tee_key_material_signature[sym_key_length] = { 0xD0,0xD5,0x8E,0x72,0xBE,0x81,0x4D,0x3A,0x4C,0x22,0x1B,0x04,0x0F,0x58,0x16,0x2D,0x45,0x96,0x08,0xE2,0xE8,0x1D,0x09,0xEA,0x53,0xA1,0xF1,0xA4,0x79,0xE6,0x66,0xD8,0x80,0x1B,0xBF,0x2A,0x97,0x08,0xD1,0x01,0xE4,0xF7,0x26,0x0E,0x21,0x68,0x7E,0x9C,0xE7,0xFC,0x5E,0x50,0x5A,0x07,0xA2,0x16,0x3C,0x31,0x69,0x6E,0x94,0xF8,0x0E,0xF7,0x79,0x9E,0x8C,0x90,0xFF,0x8F,0xFE,0x3D,0x62,0x3C,0x4D,0xF4,0x43,0x81,0xFF,0x19,0x61,0x74,0x4D,0x0D,0x3E,0x89,0x89,0x8F,0xAB,0x8D,0x70,0x2E,0x18,0xC7,0x38,0x2A,0x4F,0x45,0xF9,0x11,0x4D,0x9C,0x83,0x2A,0x37,0xFE,0x78,0xBA,0x3C,0x01,0xEC,0x41,0x62,0x3D,0x54,0x5A,0x9C,0x9E,0x3C,0x13,0x2D,0x36,0x37,0x86,0x8C,0x3E,0x33,0xDB,0x54,0x2E,0xA4,0x26,0xA2,0x76,0xAC,0x53,0xC3,0x4B,0x29,0xA1,0xDF,0x5C,0xC4,0x86,0x41,0xF8,0xC3,0x7F,0xC6,0x9A,0x77,0x53,0xA5,0xA6,0xE1,0x9A,0x96,0xAE,0x3F,0xC1,0x82,0x26,0x26,0x0B,0x2C,0x9E,0xCC,0xC8,0xD1,0x21,0xF4,0x9D,0x15,0xD0,0x30,0x67,0xAB,0xD8,0xB6,0xB5,0xF5,0x92,0xA7,0xD0,0x80,0x6C,0x65,0x61,0x79,0x82,0xA9,0xE2,0xCD,0x96,0x79,0xC8,0x18,0x22,0xC8,0xF3,0x78,0x8B,0x46,0x3F,0x13,0xA1,0xD4,0x55,0x15,0x8D,0x34,0xA1,0xDF,0xCE,0x6E,0x6D,0x14,0x16,0xAC,0x97,0xB4,0x9B,0xED,0x6E,0x53,0x96,0xB9,0xB1,0xCA,0xC6,0x6D,0xD4,0x7C,0xB6,0xD4,0x5F,0x90,0x93,0x67,0xEC,0x96,0x42,0x84,0xD0,0x0E,0xA8,0xF5,0x2D,0x1A,0x1A,0x37,0x21,0x49,0x5D,0x4F,0x5D };

	tee_key_material sym_key;
	memset(&sym_key, 0x00, sizeof(sym_key));

	memcpy(sym_key.sig_sd_id, oemSdBin, TEE_SDID_LENGTH);
	sym_key.key_id = 1;
	sym_key.counter = 1;
	sym_key.cipher_type = tee_cipher_aes_gcm_256;
	memset(sym_key.reserved, 0, sizeof(sym_key.reserved));
	memcpy(sym_key.encrypted_key, sym_key_enc, sym_key_length);
	memcpy(sym_key.signature, tee_key_material_signature, sym_key_length);

	teeStatus = TEE_SetTAEncryptionKey(intelSdSession, &sym_key);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		cout << "TEE_SetTAEncryptionKey failed. Error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	// Open OEM SD session
	cout << "Openning an OEM SD session..." << endl;
	teeStatus = TEE_OpenSDSession(oemSD, &oemSdSession);
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		cout << "TEE_OpenSDSession with OEM SD failed. Error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	// Install an encrypted applet
	cout << "Installing an encrypted applet..." << endl;
	teeStatus = TEE_SendAdminCmdPkg(oemSdSession, &installAppletBlob[0], (uint32_t)installAppletBlob.size());
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		cout << "Encrypted applet installation failed. Error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	// Create a session of the encrypted applet
	cout << "Creating a session to the encrypted applet..." << endl;
	jhiStatus = JHI_CreateSession(hJOM, ECHO_APP_ID, 0, 0, &hSession);
	if (jhiStatus != JHI_SUCCESS)
	{
		cout << "JHI_CreateSession to the encrypted applet failed. Error code: 0x" << hex << jhiStatus << "(" << JHIErrorToString(jhiStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	// Send a command to the encrypted applet
	cout << "Sending a command to the encrypted applet..." << endl; 
	
	const size_t bufsize = 5;
	UINT8   txBuffer[bufsize] = { 0x0 },
			rxBuffer[bufsize] = { 0x0 };
	
	fill_buffer(txBuffer, bufsize);

	comm_buf.TxBuf->length = bufsize;
	comm_buf.TxBuf->buffer = txBuffer;

	comm_buf.RxBuf->length = bufsize;
	comm_buf.RxBuf->buffer = rxBuffer;
	
	jhiStatus = JHI_SendAndRecv2(hJOM, hSession, 0, &comm_buf, &responseCode);
	if (jhiStatus != JHI_SUCCESS)
	{
		cout << "JHI_SendAndRecv2 to the encrypted applet failed. Error code: 0x" << hex << jhiStatus << "(" << JHIErrorToString(jhiStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	// Verify result
	if ((uint32_t)responseCode != comm_buf.TxBuf->length)
	{
		cout << "Error: SendAndRecv resposne code should have match the input buffer size." << endl;
		exit_test(EXIT_FAILURE);
	}

	if (!check_buffer((unsigned char*)comm_buf.RxBuf->buffer, bufsize))
	{
		cout << "Verification PASS" << endl;
	}
	else
	{
		cout << "Verification FAIL" << endl;
	}

	// Close the session
	cout << "Closing the session to the encrypted applet..." << endl;
	jhiStatus = JHI_CloseSession(hJOM, &hSession);
	if (jhiStatus != JHI_SUCCESS)
	{
		cout << "JHI_CloseSession to the encrypted applet failed. Error code: 0x" << hex << jhiStatus << "(" << JHIErrorToString(jhiStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	// Uninstall the encrypted applet
	cout << "Uninstalling the encrypted applet..." << endl;
	jhiStatus = JHI_Uninstall(hJOM, ECHO_APP_ID);
	if (jhiStatus != JHI_SUCCESS)
	{
		cout << "Encrypted applet Uninstallation failed. Error code: 0x" << hex << jhiStatus << "(" << JHIErrorToString(jhiStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	// Uninstall OEM SD
	cout << "Uninstalling the OEM SD..." << endl;
	TEE_SendAdminCmdPkg(intelSdSession, &uninstallSdBlob[0], (uint32_t)uninstallSdBlob.size());
	if (teeStatus != TEE_STATUS_SUCCESS)
	{
		cout << "Uninstall OEM SD failed. Error code: 0x" << hex << teeStatus << "(" << TEEErrorToString(teeStatus) << ")" << endl;
		exit_test(EXIT_FAILURE);
	}

	cout << "Applet Encryption test passed" << endl;
}

void negative_test_sessions(JHI_HANDLE hJOM)
{
    JHI_SESSION_HANDLE hSession;
    JHI_RET status;
    FILECHAR szCurDir [LEN_DIR];
    UINT8 buffer[5] = { 0x01,0x02,0x03,0x04,0x05 };
    DATA_BUFFER initData;
    initData.buffer = buffer;
    initData.length = 5;

    GetFullFilename(szCurDir, ECHO_FILENAME);

    // try to  install the spooler applet in JOM
    status = JHI_Install2( hJOM, SPOOLER_APP_ID, szCurDir) ;
    if (status == JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install spooler applet should have failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // now install the echo applet in JOM
    status = JHI_Install2( hJOM, ECHO_APP_ID, szCurDir) ;
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // create a session
    status = JHI_CreateSession(hJOM,ECHO_APP_ID,0,&initData,&hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    status = JHI_GetSessionsCount(hJOM,ECHO_APP_ID, NULL);

    if( status == JHI_SUCCESS)
    {
        fprintf( stdout, "JHI GetSessionsCount succeeded, but should fail\n");
        exit_test(EXIT_FAILURE);
    }
    if (status != 0x203)
    {
        fprintf( stdout, "JHI GetSessionsCount failed, with wrong error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // get session status
    status = JHI_GetSessionInfo(hJOM,hSession, NULL);
    if( status == JHI_SUCCESS)
    {
        fprintf( stdout, "JHI GetSessionsCount succeeded, but should fail\n");
        exit_test(EXIT_FAILURE);
    }
    if (status != 0x203)
    {
        fprintf( stdout, "JHI GetSessionsCount failed, with wrong error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // close the session
    status = JHI_CloseSession(hJOM,&hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    status = JHI_Uninstall (hJOM, ECHO_APP_ID);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall applet failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    fprintf( stdout, "\nSessions negative test passed\n") ;
}

JHI_VM_TYPE get_vm_type()
{
	VERSION version;
	JHI_VM_TYPE vm_type = JHI_VM_TYPE_BEIHAI_V2;

	if (!getFWVersion(&version))
	{
		fprintf(stdout, "Get version failed, aborting test.\n");
		exit_test(EXIT_FAILURE);
	}

	if (version.Major < 3)
	{
		vm_type = JHI_VM_TYPE_BEIHAI_V1;
	}

	return vm_type;
}

/*
void test_JHI_stress(JHI_HANDLE hJOM)
{
    UINT8   txBuffer[BUFFER_SIZE] = {0x0},
            rxBuffer[BUFFER_SIZE] = {0x0};
    FILECHAR szCurDir [LEN_DIR];
    JVM_COMM_BUFFER txrx ;
    JHI_SESSION_HANDLE hSession;
    JHI_RET status;

    GetFullFilename(szCurDir, ECHO_FILENAME);

    // now install the echo applet in JOM
    fprintf( stderr, "\ninstalling the echo applet \n") ;
    status = JHI_Install2( hJOM, ECHO_APP_ID, szCurDir) ;
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI install failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // create a session
    fprintf( stderr, "creating session of the echo applet \n") ;
    status = JHI_CreateSession(hJOM,ECHO_APP_ID,0,NULL,&hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI create session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    // how to iterate thru
    fill_buffer(txBuffer, BUFFER_SIZE ) ;

    fprintf( stderr, "starting send and recieve sequence..\n") ;

    // send max buffer size
    txrx.TxBuf->length = BUFFER_SIZE ; // length
    txrx.TxBuf->buffer = txBuffer;
    txrx.RxBuf->length = BUFFER_SIZE ;
    txrx.RxBuf->buffer = rxBuffer ;

    fprintf( stderr, "Infinite loop: Sending and receiving buffer to JOM Size: %04d... ", BUFFER_SIZE );

    while(1)
    {
        status = JHI_SendAndRecv2(hJOM, hSession,0, &txrx,NULL);
        if( JHI_SUCCESS != status )
        {
            fprintf( stderr, "\nError sending buffer with size %d, error code: 0x%x (%s)\n",BUFFER_SIZE, status , JHIErrorToString(status));
            exit_test(EXIT_FAILURE);
        }
    }

    // close the session
    status = JHI_CloseSession(hJOM,&hSession);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI close session failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }

    status = JHI_Uninstall (hJOM, ECHO_APP_ID);
    if (status != JHI_SUCCESS)
    {
        fprintf( stdout, "JHI uninstall applet failed, error code: 0x%x (%s)\n", status, JHIErrorToString(status));
        exit_test(EXIT_FAILURE);
    }
    fprintf( stdout, "Send and Recieve test passed\n") ;
}
*/