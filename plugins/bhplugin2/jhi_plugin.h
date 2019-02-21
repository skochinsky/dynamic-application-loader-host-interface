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
**
**    @file jhi_plugin.h
**
**    @brief  Defines BEIHAI Client plugin functions
**
**    @author Elad Dabool
**
********************************************************************************
*/

#ifndef __JHI_PLUGIN_H__
#define __JHI_PLUGIN_H__

// Affects plugin_interface.h
#define JHI_PLUGIN

#include "bhp_exp.h"
#include "jhi.h"
#include "jhi_plugin_internal.h"
#include "plugin_interface.h"
#include "Singleton.h"
#include "teetransport.h"

using namespace intel_dal;

namespace Jhi_Plugin
{
	class BeihaiPlugin : public VM_Plugin_interface, public Singleton<BeihaiPlugin>
	{
		friend class Singleton<BeihaiPlugin>;

	public:
		// Plugin Interface Implementation
		UINT32 JHI_Plugin_Init(bool do_vm_reset = true);

		UINT32 JHI_Plugin_DeInit(bool do_vm_reset = true);
		UINT32 JHI_Plugin_Set_Transport_And_Memory(unsigned int transportType, JHI_PLUGIN_MEMORY_API* plugin_memory_api);

		UINT32 JHI_Plugin_GetPluginType();

		UINT32 JHI_Plugin_DownloadApplet (const char *pAppId, uint8_t* pAppBlob, unsigned int BlobSize);
		UINT32 JHI_Plugin_UnloadApplet (const char *AppId );

		UINT32 JHI_Plugin_GetAppletProperty (const char *AppId, JVM_COMM_BUFFER *pIOBuffer);

		UINT32 JHI_Plugin_CreateSession (const char *AppId, VM_SESSION_HANDLE* pSession, const uint8_t* pAppBlob, unsigned int BlobSize, JHI_SESSION_ID SessionID,DATA_BUFFER* initBuffer);
		UINT32 JHI_Plugin_CloseSession (VM_SESSION_HANDLE* pSession);
		UINT32 JHI_Plugin_ForceCloseSession(VM_SESSION_HANDLE* pSession);

		UINT32 JHI_Plugin_WaitForSpoolerEvent (VM_SESSION_HANDLE SpoolerSession,JHI_EVENT_DATA** ppEventData,JHI_SESSION_ID* targetSession);
		UINT32 JHI_Plugin_SendAndRecv (VM_SESSION_HANDLE Session, INT32 nCommandId, JVM_COMM_BUFFER *pIOBuffer,INT32* pResponseCode);

		UINT32 JHI_Plugin_OpenSDSession(const string& SD_ID, SD_SESSION_HANDLE* pSession);
		UINT32 JHI_Plugin_CloseSDSession(SD_SESSION_HANDLE* pSession);
		UINT32 JHI_Plugin_ListInstalledTAs (const SD_SESSION_HANDLE handle, vector<string>& UUIDs);
		UINT32 JHI_Plugin_ListInstalledSDs(const SD_SESSION_HANDLE handle, vector<string>& UUIDs);
		UINT32 JHI_Plugin_SendCmdPkg (const SD_SESSION_HANDLE handle, vector<uint8_t>& blob);
		UINT32 JHI_Plugin_QueryTeeMetadata(unsigned char** metadata, unsigned int* length);
		UINT32 JHI_Plugin_ParsePackage(uint8_t* cmd_pkg, uint32_t pkg_len, OUT PACKAGE_INFO& pkgInfo);

		UINT32 JHI_Plugin_ProvisionOemMasterKey(const char * key); // tee_asym_key_material
		UINT32 JHI_Plugin_SetTAEncryptionKey(const char * key);   // tee_key_material
		
#ifdef _WIN32
		void JHI_Plugin_SetLogSettings(JHI_LOG_LEVEL log_level, JHI_LOG_TARGET log_target) { g_jhiLogLevel = log_level; g_jhiLogTarget = log_target; }
#endif
	private:
		BeihaiPlugin();
		JHI_PLUGIN_TYPE plugin_type;
		JHI_PLUGIN_MEMORY_API memory_api;
		BHP_TRANSPORT bh_transport_APIs;

		SD_SESSION_HANDLE intel_sd_handle;
		volatile bool is_intel_sd_open;

		SD_SESSION_HANDLE oem_sd_handle;
		volatile bool is_oem_sd_open;
		string oem_sd_id;

		static TEE_TRANSPORT_INTERFACE  transport_interface;

		//internal functions
		void uninstallAll();
		BH_RET openIntelSD();
		BH_RET closeIntelSD();
		unsigned int getTotalSessionsCount();
		int sendSessionIDtoApplet(VM_SESSION_HANDLE* pSession, JHI_SESSION_ID SessionID, int* appletResponse);
		bool convertAppProperty_Version(char** output);
		void setUninstallPack(const char *pAppId, char** uninstallPkg);
		bool isTAinstalled(const char *pAppId);
		UINT32 getTA_SessionCount(const char *pAppId);

		// translate errors from Beihai to JHI errors
		UINT32 beihaiToJhiError(int bhError, UINT32 defaultError);
		UINT32 beihaiToTeeError(int bhError, UINT32 defaultError);
		const char* BHErrorToString(UINT32 retVal);

		static int sendWrapper(uintptr_t handle, uint8_t* buffer, uint32_t length);
		static int recvWrapper(uintptr_t handle, uint8_t* buffer, uint32_t* length);
		static int connectWrapper(int heci_port, uintptr_t *handle);
		static int closeWrapper(uintptr_t handle);
	};
}
#endif //__JHI_PLUGIN_H__