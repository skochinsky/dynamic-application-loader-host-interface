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
**    @file FWInfoWin32Sockets.h
**
**    @brief  Contains implementation for IFirmwareInfo using Devplatform Sockets log file.
**
**    @author Elad Dabool
**
********************************************************************************
*/
#ifndef _FW_INFO_WIN32_SOCKETS_H_
#define _FW_INFO_WIN32_SOCKETS_H_

#include "IFirmwareInfo.h"

#include <Windows.h>
#include "Mailbox.h"
#include "MkhiMsgs.h"
		
#define DEVPLATFORM_MAILBOX_NAME "AMLT\\JOM_MailBox" 
#define JHI_MAILBOX_NAME "AMLT\\JHI_MailBox"

#define JHI_FW_VERSION_REQUEST "FWVersion"

#ifdef IPT_UB_RCR

#define JHI_VPRO_ENABLED_REQUEST "VPRO_Enabled"
#define JHI_CPU_BRAND_STRING_REQUEST "CPU_Brand"
#define JHI_CPU_TYPE_REQUEST "CPU_Type"
#define JHI_CHIPSET_REQUEST "Chipset"

#define MAX_BRAND_STRING 64

#endif

namespace intel_dal
{

	class FWInfoWin32Sockets : public IFirmwareInfo
	{
	public:
		bool GetFwVersion(VERSION* fw_version);
		bool GetPlatformType(ME_PLATFORM_TYPE* platform_type);

#ifdef IPT_UB_RCR
		bool getVproEnabled(bool* vpro);
		bool getCpuBrandString(char brandString[MAX_BRAND_STRING]);
		bool getCpuType(int* cpuType);
		bool  getChipset(int* chipset);
#endif
		bool Connect();
		bool Disconnect();

	private:
			MailBox jhi_mailbox;
	};

}

#endif