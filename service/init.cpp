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
**    @file init.cpp
**
**    @brief  Defines functions for the JHI Init interface
**
**    @author Elad Dabool
**
********************************************************************************
*/
#include "jhi_service.h"
#include "dbg.h"
#include "reg.h"
#include "GlobalsManager.h"
#include "AppletsManager.h"
#include "SessionsManager.h"
#include "EventManager.h"
#include "EventLog.h"
#include "string_s.h"

#ifdef _WIN32
#include "Win32Service.h" // for heci driver events
#endif // _WIN32

using namespace intel_dal;




//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
JHI_RET_I
	JhiGetRegistryValues()
{
	UINT32  ulRetCode = JHI_INTERNAL_ERROR;
	FILECHAR   appletsFileLocation[FILENAME_MAX+1]={0};
	FILECHAR   jhiFileLocation[FILENAME_MAX+1]={0};
	TEE_TRANSPORT_TYPE transportType = TEE_TRANSPORT_TYPE_TEE_LIB;

#ifndef _WIN32
	FILECHAR   jhiPluginLocation[FILENAME_MAX+1]={0};
	FILECHAR   jhiSpoolerLocation[FILENAME_MAX+1]={0};
#endif

	JhiQueryLogLevelFromRegistry (&g_jhiLogLevel);

	// If prints are not completely off, print the log level
	if (g_jhiLogLevel == JHI_LOG_LEVEL::JHI_LOG_LEVEL_RELEASE)
		LOG0("JHI service release prints are enabled\n");
	else if (g_jhiLogLevel == JHI_LOG_LEVEL::JHI_LOG_LEVEL_DEBUG)
		TRACE0("JHI service debug trace and release prints are enabled\n");


	//Read app repository location
	if( JHI_SUCCESS != JhiQueryAppFileLocationFromRegistry(
		appletsFileLocation,
		(FILENAME_MAX-1) * sizeof(FILECHAR)))
	{
		// Can fail on Windows
		LOG0( "unable to find applets repository location from registry") ;
		WriteToEventLog(JHI_EVENT_LOG_ERROR, MSG_REGISTRY_READ_ERROR);
		ulRetCode = JHI_ERROR_REGISTRY;
		goto error;
	}

	//verify the applet repository exist
	if (_waccess_s(appletsFileLocation,0) != 0)
	{
		LOG0("Init failed - cannot find applet repository directory. Searched location:");
#ifndef _WIN32        
		LOG0(appletsFileLocation);
#endif
		WriteToEventLog(JHI_EVENT_LOG_ERROR, MSG_REPOSITORY_NOT_FOUND);
		ulRetCode = JHI_ERROR_REPOSITORY_NOT_FOUND;
		goto error;
	}

	if (!GlobalsManager::Instance().setAppletsFolder(appletsFileLocation))
	{
		TRACE0("Init failed - setAppletsFolder failed.");
		ulRetCode = JHI_INTERNAL_ERROR;
		goto error;
	}

	//Read jhi service file location
	if( JHI_SUCCESS != JhiQueryServiceFileLocationFromRegistry(
		jhiFileLocation,
		(FILENAME_MAX-1) * sizeof(FILECHAR)))
	{
		LOG0( "unable to query file location from registry") ;
		WriteToEventLog(JHI_EVENT_LOG_ERROR, MSG_REGISTRY_READ_ERROR);
		ulRetCode = JHI_ERROR_REGISTRY;
		goto error;
	}

	//verify the jhi service file location exist
	if (_waccess_s(jhiFileLocation,0) != 0)
	{
		LOG0("Init failed - the service file location does not exist");
		ulRetCode = JHI_INTERNAL_ERROR;
		goto error;
	}

	if (!GlobalsManager::Instance().setServiceFolder(jhiFileLocation))
	{
		LOG0("Init failed - setServiceFolder failed.");
		ulRetCode = JHI_INTERNAL_ERROR;
		goto error;
	}

#ifndef _WIN32
	if( JHI_SUCCESS != JhiQueryPluginLocationFromRegistry(
		jhiPluginLocation,
		(FILENAME_MAX-1) * sizeof(FILECHAR)))
	{
		LOG0( "unable to find Plugin location from registry") ;
		WriteToEventLog(JHI_EVENT_LOG_ERROR, MSG_REGISTRY_READ_ERROR);
		ulRetCode = JHI_ERROR_REGISTRY;
		goto error;
	}
/*
	// On Linux, no need to supply the absolute location of the plugin .so file
	if (_waccess_s(jhiPluginLocation,0) != 0)
	{
		TRACE0("Init failed - cannot find Plugin directory. Searched location:");
        TRACE0(jhiPluginLocation);
		WriteToEventLog(JHI_EVENT_LOG_ERROR, MSG_REPOSITORY_NOT_FOUND);
		ulRetCode = JHI_VM_DLL_FILE_NOT_FOUND;
		goto error;
	}
*/
	if (!GlobalsManager::Instance().setPluginFolder(jhiPluginLocation))
	{
		LOG0("Init failed - setPluginFolder failed.");
		ulRetCode = JHI_INTERNAL_ERROR;
		goto error;
	}
	if( JHI_SUCCESS != JhiQuerySpoolerLocationFromRegistry(
		jhiSpoolerLocation,
		(FILENAME_MAX-1) * sizeof(FILECHAR)))
	{
		LOG0( "unable to query Spooler location from registry") ;
		WriteToEventLog(JHI_EVENT_LOG_ERROR, MSG_REGISTRY_READ_ERROR);
		ulRetCode = JHI_ERROR_REGISTRY;
		goto error;
	}
	if (_waccess_s(jhiSpoolerLocation,0) != 0)
	{
		LOG0("Init failed - the Spooler file location does not exist");
		ulRetCode = JHI_INTERNAL_ERROR;
		goto error;
	}
	if (!GlobalsManager::Instance().setSpoolerFolder(jhiSpoolerLocation))
	{
		LOG0("Init failed - setSpoolerFolder failed.");
		ulRetCode = JHI_INTERNAL_ERROR;
		goto error;
	}
#endif //!_WIN32

	//Read the transport type
	if(JHI_SUCCESS != JhiQueryTransportTypeFromRegistry((uint32_t*)&transportType))
	{
		TRACE0( "Unable to query transport type from registry, keeping default (TEE LIB).") ;
		// Ignore error, keep default (TEE LIB)
		transportType = TEE_TRANSPORT_TYPE_TEE_LIB; // just in case the other APIs changed it.
	}

	GlobalsManager::Instance().setTransportType(transportType);

	ulRetCode  = JHI_SUCCESS;

error:
	return ulRetCode ;
}

//-------------------------------------------------------------------------------
// Function: jhis_init
//		  First interface to be called by IHA or any external vendor
//        to initialize data structs and set up COMMS with JoM
// RETURN : JHI_RET - success or any failure returns
//-------------------------------------------------------------------------------
//1.	Init performs initialization of the Global data 
//		and all related variables
//2.	Creating App Management tables and registry initializations
//3.	COMMS initialization where the HECI connection is established
//4.	TL Plugin functions and corresponding function pointers are initialized 
//		and registered.
//5.	TL Initialization sequence:
//		a.	Creating schannel setup by passing Rd/Wr handles to SMAPI 
//		b.	Creating and allocation of device context 
//		c.	Opening Service Manager handle and store it in the global struct
//			for future SMAPI calls
//		d.	Creating session management tables
//		e.	TL FW Reset is performed to get host in-sync with JoM. Existing applets 
//			from the JoM are queried, their sessions closed and subsequently 
//			unloaded from JoM to enable a fresh start for both host and JoM. 
//			Note that, the corresponding PACK files will remain in disk on the host.
//-------------------------------------------------------------------------------
JHI_RET_I jhis_init()
{
	UINT32 ulRetCode = JHI_SUCCESS;

	VM_Plugin_interface* plugin = NULL;
	TEE_TRANSPORT_TYPE transportType;
	VERSION fwVersion;
	memset(&fwVersion, 0, sizeof(fwVersion));
	JHI_PLATFROM_ID fwType = INVALID_PLATFORM_ID;
	bool do_vm_reset = true;

	//Init done already
	if (GlobalsManager::Instance().getJhiState() != JHI_STOPPED) 
		goto end;

	ulRetCode = JhiGetRegistryValues();
	if (ulRetCode != JHI_SUCCESS)
	{
		TRACE0("Error: JhiGetRegistryValues() failed\n");
		goto end;
	}

	transportType =	GlobalsManager::Instance().getTransportType();

#ifdef _WIN32
	if (transportType != TEE_TRANSPORT_TYPE_SOCKET)
	{
		// register for heci driver events
		if (!RegisterHeciDeviceEvents())
		{
			LOG0("failed to register for HECI events\n");
			WriteToEventLog(JHI_EVENT_LOG_ERROR, MSG_FW_COMMUNICATION_ERROR);
			ulRetCode = JHI_NO_CONNECTION_TO_FIRMWARE;
			goto end;
		}
	}
#endif // _WIN32

	if (!AppletsManager::Instance().Initialize())	// initializing the AppletsManager
	{
		TRACE0("AppletsManager Initialize failed\n");
		ulRetCode = JHI_NO_CONNECTION_TO_FIRMWARE;
		goto end;
	}	

	// Register the plugin (TEE vs. BEIHAI)
	if (!GlobalsManager::Instance().isPluginRegistered())
	{
		ulRetCode = GlobalsManager::Instance().PluginRegister();
		if (ulRetCode != JHI_SUCCESS)
		{
			LOG0("Error: JhiPlugin_Register() failed\n");
			goto end;
		}
	}
	else
	{
		// do not register the plugin more than once;
		TRACE0("VM Plugin is already registered, skipping registration\n");
	}

	GlobalsManager::Instance().getPluginTable(&plugin);
	if (plugin == NULL)
	{
		ulRetCode = JHI_INTERNAL_ERROR;
		goto end;
	}

	JHI_PLUGIN_MEMORY_API plugin_memory_api;

#ifdef JHI_MEMORY_PROFILING
	plugin_memory_api.allocateMemory = (PFN_JHI_ALLOCATE_MEMORY) JHI_ALLOC1;
	plugin_memory_api.freeMemory = (PFN_JHI_FREE_MEMORY) JHI_DEALLOC1;
#else
	plugin_memory_api.allocateMemory = (PFN_JHI_ALLOCATE_MEMORY)JHI_ALLOC;
	plugin_memory_api.freeMemory = (PFN_JHI_FREE_MEMORY)JHI_DEALLOC;
#endif



	// Delivers the transport type & memory APIs to the plugin
	ulRetCode = plugin->JHI_Plugin_Set_Transport_And_Memory(transportType, &plugin_memory_api);
	if (ulRetCode != JHI_SUCCESS)
	{
		TRACE0("Error: pfnSetTransport() failed\n");
		goto end;
	}

	// In case KDI is present we don't want to do a reset since it can kill already opened KDI sessions.
	// KDI can have its own sessions only over BHv2.
	if (transportType == TEE_TRANSPORT_TYPE_DAL_DEVICE && AppletsManager::Instance().getPluginType() == JHI_PLUGIN_TYPE_BEIHAI_V2)
		do_vm_reset = false;

	// Call plugin Init
	ulRetCode = plugin->JHI_Plugin_Init(do_vm_reset);

	if (ulRetCode != JHI_SUCCESS)
	{
		TRACE1 ("VM plugin Init failure, with ret code: %08x\n", ulRetCode);
		goto end;
	}

	ulRetCode = EventManager::Instance().Initialize(); // initializing the EventManager (spooler applet)
	if (ulRetCode != JHI_SUCCESS)
	{
		TRACE0("EventManager Initialize failed\n");
		WriteToEventLog(JHI_EVENT_LOG_ERROR, MSG_INVALID_SPOOLER);
		goto end;
	}

	fwVersion = AppletsManager::Instance().getFWVersion();
	fwType = AppletsManager::Instance().getFWtype();
		
#ifdef IPT_UB_RCR
	if ((fwVersion.Major) > 2 && (fwVersion.Major) < 10)
	{
		updateIPTStatus();
	}
#endif //IPT_UB_RCR

	if (fwType == CSE)
	{
		// Updates the applets installed in the repository.
		AppletsManager::Instance().updateAppletsList();
	}

	GlobalsManager::Instance().setJhiState(JHI_INITIALIZED);


end:

	if(ulRetCode != JHI_SUCCESS)
	{
		if ( (GlobalsManager::Instance().getPluginTable(&plugin)) && (plugin != NULL) )
		{
			plugin->JHI_Plugin_DeInit(do_vm_reset);
		}

		if(GlobalsManager::Instance().isPluginRegistered())
		{
			GlobalsManager::Instance().PluginUnregister();
		}

		// Init failed. Log an error.
		WriteToEventLog(JHI_EVENT_LOG_ERROR, MSG_SERVICE_STOP);
	}

	return ulRetCode;
}

void JhiReset()
{

	VM_Plugin_interface* plugin = NULL;
#ifdef _WIN32
	TEE_TRANSPORT_TYPE transportType;
#endif //_WIN32

	UINT32 ret;

	// release all blocked request by closing all opened sessions in the VM
	SessionsManager::Instance().closeSessionsInVM();

	// wait for all previous requests to end before reseting JHI
	GlobalsManager::Instance().initLock.aquireWriterLock();

	if (GlobalsManager::Instance().getJhiState() == JHI_STOPPED)
	{
		// no need to reset since JHI is not initialized
		GlobalsManager::Instance().initLock.releaseWriterLock();
		return;
	}

	LOG0("jhi reset starting");
	WriteToEventLog(JHI_EVENT_LOG_INFORMATION, MSG_SERVICE_RESET);

	//App Table reset
	AppletsManager::Instance().resetAppletTable();

	//reset sessions table
	SessionsManager::Instance().resetSessionManager();

	EventManager::Instance().Deinit();

	// Deinit Plugin
	if ( (GlobalsManager::Instance().getPluginTable(&plugin)) && (plugin != NULL) )
	{
		// In case KDI is present we don't want to do a reset since it can kill already opened KDI sessions.
		// KDI can have its own sessions only over BHv2.
		bool do_vm_reset = true;
		TEE_TRANSPORT_TYPE transportType = GlobalsManager::Instance().getTransportType();
		JHI_PLUGIN_TYPE    pluginType    = AppletsManager::Instance().getPluginType();

		if (transportType == TEE_TRANSPORT_TYPE_DAL_DEVICE &&  pluginType == JHI_PLUGIN_TYPE_BEIHAI_V2)
			do_vm_reset = false;

		ret = plugin->JHI_Plugin_DeInit(do_vm_reset);


		if (ret != JHI_SUCCESS)
		{
			TRACE1("Error: VM Plugin Deinit failed: 0x%X\n",ret);
		}

		GlobalsManager::Instance().PluginUnregister();
	}

#ifdef _WIN32
	transportType =	GlobalsManager::Instance().getTransportType();
	if (transportType != TEE_TRANSPORT_TYPE_SOCKET)
	{
		// unregister heci driver events
		if (!UnRegisterHeciDeviceEvents())
		{
			TRACE0("Error: failed to unregister heci events\n");
		}
	}
#endif // _WIN32

	GlobalsManager::Instance().setJhiState(JHI_STOPPED);

	// we signal that the reset is done in order to awake waiting threads
	// [ either MEI disable event thread, or JHI main thread ] 
	GlobalsManager::Instance().sendResetCompleteEvent();

	GlobalsManager::Instance().initLock.releaseWriterLock();
}
