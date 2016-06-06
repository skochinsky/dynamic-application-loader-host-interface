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
**    @file FWInfoWin32.h
**
**    @brief  Contains implementation for IFirmwareInfo using FU client.
**
**    @author Elad Dabool
**
********************************************************************************
*/
#ifndef _FW_INFO_WIN32_H_
#define _FW_INFO_WIN32_H_

#include "IFirmwareInfo.h"

#include <Windows.h>
#include <setupapi.h>
#include <initguid.h>
#include <WinIoCtl.h>
#include "MkhiMsgs.h"

namespace intel_dal
{
	typedef enum
	{
	   MKHI_CBM_GROUP_ID = 0,
	   MKHI_PM_GROUP_ID,
	   MKHI_PWD_GROUP_ID,
	   MKHI_FWCAPS_GROUP_ID,
	   MKHI_APP_GROUP_ID,      // Reserved (no longer used).
	   MKHI_FWUPDATE_GROUP_ID, // This is for manufacturing downgrade
	   MKHI_FIRMWARE_UPDATE_GROUP_ID,
	   MKHI_BIST_GROUP_ID,
	   MKHI_MDES_GROUP_ID,
	   MKHI_ME_DBG_GROUP_ID,
	   MKHI_MAX_GROUP_ID,
	   MKHI_GEN_GROUP_ID = 0xFF
	}MKHI_GROUP_ID;

	#define FWCAPS_GET_RULE_CMD            0x02
	#define FWCAPS_GET_RULE_CMD_ACK        0x82

	#define ME_RULE_FEATURE_ID                           0
	#define MEFWCAPS_PCV_OEM_PLAT_TYPE_CFG_RULE          29

	typedef union _RULE_ID
	{
	   UINT32      Data;
	   struct
	   {
		  UINT32   RuleTypeId     :16;
		  UINT32   FeatureId      :8;
		  UINT32   Reserved       :8;
	   }Fields;
	}RULE_ID;

	typedef struct _GET_RULE_DATA
	{
	   RULE_ID  RuleId;
	}GET_RULE_DATA;

	typedef struct _GET_RULE_ACK_DATA
	{
	   RULE_ID  RuleId;
	   UINT8    RuleDataLen;
	   UINT8    RuleData[0];
	}GET_RULE_ACK_DATA;

	typedef struct _FWCAPS_GET_RULE
	{
	   MKHI_MESSAGE_HEADER     Header;
	   GET_RULE_DATA           Data;
	}FWCAPS_GET_RULE;

	typedef struct _FWCAPS_GET_RULE_ACK
	{
	   MKHI_MESSAGE_HEADER     Header;
	   GET_RULE_ACK_DATA       Data;
	}FWCAPS_GET_RULE_ACK;

#ifdef IPT_UB_RCR
#ifdef IPT_VERIFIER_TOOL	// API used only by the IPT Verifier tool.

	typedef union _MEFWCAPS_SKU
	{
	   UINT32   Data;
	   struct
	   {
		  UINT32   MngFull          :1; // BIT 0:  Full network manageability 
		  UINT32   MngStd           :1; // BIT 1:  Standard network manageability 
		  UINT32   Amt              :1; // BIT 2:  Consumer manageability      
		  UINT32   LocalMng             :1; // BIT 3:  Repurposed from IRWT, Local Mng a.k.a Treasurelake
		  UINT32   L3Mng              :1; // BIT 4:  Repurposed from Qst
		  UINT32   Tdt              :1; // BIT 5:  AT-p (Anti Theft PC Protection aka Tdt)
		  UINT32   SoftCreek        :1; // BIT 6:  Intel Capability Licensing Service aka CLS
		  UINT32   Ve               :1; // BIT 7:  Virtualization Engine
		  UINT32   Nand35           :1; // BIT 8:  Tacoma Pass 35mm
		  UINT32   Nand29           :1; // BIT 9:  Tacoma Pass 29mm
		  UINT32   ThermReport      :1; // BIT 10: Thermal Reporting
		  UINT32   IccOverClockin   :1; // BIT 11: 
		  UINT32   Pav              :1; // BIT 12: Protected Audio Video Path (**Reserved for external documentation***)
		  UINT32   Spk              :1; // BIT 13:
		  UINT32   Rca              :1; // BIT 14:
		  UINT32   Rpat             :1; // BIT 15:      
		  UINT32   Hap              :1; // BIT 16: HAP_Platform
		  UINT32   Ipv6             :1; // BIT 17:
		  UINT32   Kvm              :1; // BIT 18: 
		  UINT32   Och              :1; // BIT 19: 
		  UINT32   MEDAL        :1; // BIT 20
		  UINT32   Tls              :1; // BIT 21: 
		  UINT32   Cila             :1; // BIT 22: 
		  UINT32   Wlan             :1; // BIT 23: 
		  UINT32   WirelessDisp     :1; // BIT 24: Wireless Display
		  UINT32   USB3             :1; // BIT 25: USB 3.0
		  UINT32   Nap              :1;  //BIT 26
		  UINT32   AlarmClk         :1; //BIT 27
		  UINT32   CbRaid              :1;//Bit 28
		  UINT32   MediaVault          :1;//Bit 29
		  UINT32   mDNSProxy          :1;//Bit 30
		  UINT32   Nfc        :1; //Bit 31 NFC   
	   }Fields;
	}MEFWCAPS_SKU;

	#define MEFWCAPS_FEATURE_ENABLE_RULE 32

#endif //IPT_VERIFIER_TOOL
#endif //IPT_UB_RCR


	// HECI GUID(Global Unique ID): {E2D1FF34-3458-49A9-88DA-8E6915CE9BE5}
	DEFINE_GUID(GUID_DEVINTERFACE_HECI, 0xE2D1FF34, 0x3458, 0x49A9,
	  0x88, 0xDA, 0x8E, 0x69, 0x15, 0xCE, 0x9B, 0xE5);

	// HCI HECI DYNAMIC CLIENT GUID
	DEFINE_GUID(HCI_HECI_DYNAMIC_CLIENT_GUID,0x8e6a6715, 0x9abc, 0x4043, 0x88, 0xef, 0x9e, 0x39, 0xc6, 0xf6, 0x3e, 0xf);

	#define FILE_DEVICE_HECI  0x8000

	// for connecting to HECI client in the FW(e.g LME)
	#define IOCTL_HECI_CONNECT_CLIENT \
		CTL_CODE(FILE_DEVICE_HECI, 0x801, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)

	typedef struct _HECI_CLIENT_PROPERTIES
	{  
	   BYTE  ProtocolVersion;
	   DWORD  MaxMessageSize;
	} HECI_CLIENT_PROPERTIES;

	#pragma pack(1)
	typedef struct _HECI_CLIENT
	{
		UINT32                  MaxMessageLength;
		UINT8                   ProtocolVersion;
	} HECI_CLIENT;
	#pragma pack()

	class FWInfoWin32 : public IFirmwareInfo
	{
	public:
	   FWInfoWin32();
	   ~FWInfoWin32();
	   bool GetFwVersion(VERSION* fw_version);
	   bool Connect();
	   bool Disconnect();
	   
	   bool GetHeciDeviceDetail(PSP_DEVICE_INTERFACE_DETAIL_DATA &DeviceDetail);
	   HANDLE GetHandle (PSP_DEVICE_INTERFACE_DETAIL_DATA &DeviceDetail);

	   bool GetPlatformType(ME_PLATFORM_TYPE* platform_type);

#ifdef IPT_UB_RCR
#ifdef IPT_VERIFIER_TOOL // API used only by the IPT Verifier tool.
	   bool GetMEFWCAPS_SKU(MEFWCAPS_SKU* fw_caps_sku);
#endif //IPT_VERIFIER_TOOL

#endif //IPT_UB_RCR

	private:
		bool isconnected;
		int ConnectionAttemptNum;

		HANDLE hDevice;
		int MAX_BUFFER_SIZE;

		bool HeciConnectHCI( HANDLE * pHandle,HECI_CLIENT_PROPERTIES * pProperties);
		bool HeciWrite(HANDLE Handle, void * pData, DWORD DataSize, DWORD msTimeous);
		bool HeciRead(HANDLE Handle, void * pBuffer, DWORD BufferSize, DWORD * pBytesRead, DWORD msTimeous);

		bool SendGetFwVersionRequest();
		bool ReceiveGetFwVersionResponse(VERSION* fw_version);
	};

}

#endif //_FW_INFO_WIN32_H_