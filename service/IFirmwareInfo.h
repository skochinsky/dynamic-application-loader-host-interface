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
**    @file IFirmwareInfo.h
**
**    @brief  Contains interface for retrieving information from FW
**
**    @author Elad Dabool
**
********************************************************************************
*/
#ifndef _IFIRMWARE_INFO_H_
#define _IFIRMWARE_INFO_H_

#include "typedefs.h"
#include "jhi_version.h"

namespace intel_dal
{

	typedef union _ME_PLATFORM_TYPE
	{
	   UINT32    Data;
	   struct
	   {
		  UINT32   Mobile:   1;
		  UINT32   Desktop:  1; 
		  UINT32   Server:   1;
		  UINT32   WorkStn:  1;
		  UINT32   Corporate:1;
		  UINT32   Consumer: 1;
		  UINT32   SuperSKU: 1;
		  UINT32   IS_SEC:	 1;
		  UINT32   ImageType:4;
		  UINT32   Brand:    4;
		  UINT32   CpuType: 4;
		  UINT32   Chipset: 4;
		  UINT32   CpuBrandClass:    4;
		  UINT32   PchNetInfraFuses :3;
		  UINT32   Rsvd1:  1;
	   }Fields;
	}ME_PLATFORM_TYPE;

// types used in both heci and devplatform for the IPT RCR
#ifdef IPT_UB_RCR

	typedef enum{
	 CPT_CHIPSET= 1,
	 PBG_CHIPSET = 2,
	 PPT_CHIPSET = 4,
	 CHIPSET_TYPE_DONT_CARE = 0xF,
	 INVALID_CHIPSET = 0xF
	}CHIPSET_FAMILY;


	// following types are taken from FwCapsMsgs.h:
	typedef enum{

	 SNB_CPU_FAMILY = 1,
	 IVB_CPU_FAMILY,
	 UNKNOWN_CPU_FAMILY = 0xF
	}CPU_FAMILY;

	// Brand values
	#define ME_PLATFORM_TYPE_BRAND_AMT_PRO                 1
	#define ME_PLATFORM_TYPE_BRAND_STANDARD_MANAGEABILITY  2
	#define ME_PLATFORM_TYPE_BRAND_L3_MANAGEABILITY        3
	#define ME_PLATFORM_TYPE_BRAND_RPAT                    4
	#define ME_PLATFORM_TYPE_BRAND_LOCAL_MANAGEABILITY     5
	#define ME_PLATFORM_TYPE_BRAND_NO_BRAND                0

	// ImageType values  
	#define IMAGE_TYPE_NO_ME        0   // No ME FW
	#define IMAGE_TYPE_IGNITION_FW  1   // Ignition FW 
	#define IMAGE_TYPE_ME_LITE      2   // Ignition FW 
	#define IMAGE_TYPE_ME_FULL_4MB  3   // ME FW 4MB image 
	#define IMAGE_TYPE_ME_FULL_8MB  4   // ME FW 8MB image 



	typedef enum {
		LOCAL_MNG = 1, // 0 0 1    Local Mng
		Reserved1 = 2, //0 1 0     Reserved
		Reserved2 = 3,  // 0 1 1   Reserved
		FULL_MNG = 4, // 1 0 0    Full Manageability
		STD_MNG = 5, // 1 0 1   Std Manageability
		L3_UPGRADE =  6, // 1 1 0   L3 upgrade
		NO_MNG =  7 // 1 1 1   NO manageability
	}NetInfraFuses;

#endif

	class IFirmwareInfo
	{
	public:
	   virtual bool Connect() = 0;
	   virtual bool Disconnect() = 0;
	   virtual bool GetFwVersion(VERSION* fw_version) = 0;
	   virtual ~IFirmwareInfo() { }
	   virtual bool GetPlatformType(ME_PLATFORM_TYPE* platform_type) = 0;
	};
}
#endif