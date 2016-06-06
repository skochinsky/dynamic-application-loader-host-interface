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
**    @file createSession.cpp
**
**    @brief  Defines functions for the JHI session creation interface
**
**    @author Elad Dabool
**
********************************************************************************
*/
#include "jhi_service.h"
#include "misc.h"
#include "dbg.h"     
#include "SessionsManager.h"
#include "AppletsManager.h"
#include "EventLog.h"

using namespace intel_dal;

//-------------------------------------------------------------------------------
// Function: jhis_create_session
//		    Used to create a new session of an installed applet 
// IN	  : pAppId - AppId of package to be used for the creation of the session
// IN     : flags - detrmine the session properties
// RETURN : JHI_RET - success or any failure returns
//-------------------------------------------------------------------------------
JHI_RET_I
	jhis_create_session(
	const char*				pAppId,
	JHI_SESSION_ID*		pSessionID,
	UINT32				flags,
	DATA_BUFFER*		initBuffer,
	JHI_PROCESS_INFO*	processInfo
#ifdef MAX_SESSIONS_W_A
	, bool validateSessionCount
#endif
	)
{
	VM_Plugin_interface* plugin = NULL;

	list<vector<uint8_t> > appletBlobs;
	SessionsManager& Sessions = SessionsManager::Instance();
	AppletsManager&  Applets = AppletsManager::Instance();

	JHI_APPLET_STATUS appStatus;
	UINT32 ulRetCode = JHI_INTERNAL_ERROR;

	VM_SESSION_HANDLE VMSessionHandle;
	JHI_SESSION_ID newSessionID; 

	JHI_SESSION_FLAGS sessionFlags;
	sessionFlags.value = flags;
	bool isAcp = false;

	JHI_PLATFROM_ID fwType = AppletsManager::Instance().getFWtype();

	if (GlobalsManager::Instance().loggingEnabled())
	{
		JHI_LOGGER_ENTRY_MACRO("JHISVC",JHISVC_CREATESESSION_ENTER);
	}

	do
	{		
		// verify the applet is installed before trying to create a session

		appStatus = Applets.getAppletState(pAppId);

		ASSERT ( (appStatus >= 0) && (appStatus < MAX_APP_STATES) );
		if ( !( (appStatus >= 0) && (appStatus < MAX_APP_STATES) ) )
		{
			TRACE2 ("AppState incorrect: %d for appid: %s \n", appStatus, pAppId);
			ulRetCode = JHI_INTERNAL_ERROR;
			break;
		}

		FILESTRING filename;
		if (!Applets.appletExistInRepository(pAppId, &filename, isAcp))
		{
			ulRetCode = JHI_APPLET_NOT_INSTALLED;
			break;
		}
		if (appStatus == NOT_INSTALLED)
		{
			// applet is not installed but applet file exists in the repository, try to install it.
			ulRetCode = jhis_install(pAppId, filename.c_str(), true, isAcp);

			if (ulRetCode != JHI_SUCCESS)
			{
				ulRetCode = JHI_APPLET_NOT_INSTALLED;
				break;
			}
		}

		// verify all sessions owenrs and
		// perform abandoned non shared sessions clean-up
		Sessions.ClearSessionsDeadOwners();
		Sessions.ClearAbandonedNonSharedSessions();

		//check if shared session requested and there is already such session
		if (sessionFlags.bits.sharedSession)
		{
			uint16_t fw_version_major = AppletsManager::Instance().getFWVersion().Major;
			
			// In CSE (CSME and BXT), checking for Shared Session support is too heavy to be practical.
			// In these cases the check is disabled since it is not mandatory.
			// In BKL and later the enforcement is disabled completely.
			if (fw_version_major < 11 && fw_version_major != 3)
			{
				if (!Applets.isSharedSessionSupported(pAppId))
				{
					ulRetCode = JHI_SHARED_SESSION_NOT_SUPPORTED;
					break;
				}
			}

			if (Sessions.getSharedSessionID(pSessionID,pAppId))
			{
				// add the calling application to the session owners
				if (Sessions.addSessionOwner(*pSessionID,processInfo))
					ulRetCode = JHI_SUCCESS;
				else
				{	
					ulRetCode = JHI_MAX_SHARED_SESSION_REACHED;
				}

				break; // no need to create a new session
			}
		}

		// create a new session

#ifdef MAX_SESSIONS_W_A
		if (fwType == CSE) // Enforcement only for CSE
		{
			if (validateSessionCount && Sessions.sessionCount >= MAX_SESSIONS_COUNT)
			{
				Sessions.TryRemoveUnusedSharedSession(true);
				if (validateSessionCount && Sessions.sessionCount >= MAX_SESSIONS_COUNT)
				{
					ulRetCode = JHI_MAX_SESSIONS_REACHED;
					break;
				}
			}
		}
#endif


		if (!Sessions.generateNewSessionId(&newSessionID))
		{
			ulRetCode = JHI_INTERNAL_ERROR;
			break;
		}

		if ( (!GlobalsManager::Instance().getPluginTable(&plugin)) || (plugin == NULL) )
		{
			// probably we had a reset
			ulRetCode = JHI_NO_CONNECTION_TO_FIRMWARE;
			break;					
		}

		if (fwType == CSE) // Create the session for CSE.
		{
			ulRetCode = Applets.getAppletBlobs(filename, appletBlobs, isAcp);  // in CSE we need the blobs for the create session API
			if (ulRetCode != JHI_SUCCESS)
			{
				TRACE0("failed getting applet blobs from dalp file\n");
				break;
			}
			// if ulRetCode == JHI_SUCCESS appletBlobs will not be empty and VMSessionHandle will always be initialized
			for (list<vector<uint8_t> >::iterator it = appletBlobs.begin(); it != appletBlobs.end(); ++it)
			{
				ulRetCode = plugin->JHI_Plugin_CreateSession(pAppId, &VMSessionHandle, &(*it)[0], (*it).size(), newSessionID, initBuffer);
				if (ulRetCode == JHI_SUCCESS || ulRetCode == JHI_MAX_INSTALLED_APPLETS_REACHED)
				{
					break;
				}
			}
		}
		else // not CSE
		{
			ulRetCode = plugin->JHI_Plugin_CreateSession(pAppId, &VMSessionHandle, NULL, 0, newSessionID, initBuffer);
		}

		if (ulRetCode == JHI_MAX_INSTALLED_APPLETS_REACHED || ulRetCode == JHI_MAX_SESSIONS_REACHED)
		{
			if (Sessions.TryRemoveUnusedSharedSession(true))
			{
				// try to create the session again.

				if (fwType == CSE) // Create the session for CSE.
				{
					for (list<vector<uint8_t> >::iterator it = appletBlobs.begin(); it != appletBlobs.end(); ++it)
					{
						ulRetCode = plugin->JHI_Plugin_CreateSession(pAppId, &VMSessionHandle, &(*it)[0], (*it).size(), newSessionID, initBuffer);
						if (ulRetCode == JHI_SUCCESS || ulRetCode == JHI_MAX_INSTALLED_APPLETS_REACHED)
						{
							break;
						}
					}
				}
				else // not CSE
				{
					ulRetCode = plugin->JHI_Plugin_CreateSession(pAppId, &VMSessionHandle, NULL, 0, newSessionID, initBuffer);
				}
			}
		}

		if (ulRetCode == JHI_MAX_INSTALLED_APPLETS_REACHED)
		{
			ulRetCode = JHI_MAX_SESSIONS_REACHED;
		}

		if (ulRetCode == JHI_SUCCESS)
		{
			// session created in FW, add an entry in the session table and return a session handle
			if (Sessions.add(pAppId,VMSessionHandle,newSessionID,sessionFlags,processInfo))
			{
				*pSessionID = newSessionID;

#ifdef MAX_SESSIONS_W_A
				if (validateSessionCount)
				{
					++Sessions.sessionCount;
				}
#endif
				break; //success, end loop
			}
			else
			{
				ulRetCode = JHI_INTERNAL_ERROR;
				plugin->JHI_Plugin_CloseSession(&VMSessionHandle);
				break;
			}
		}
	}
	while(0);

	if (GlobalsManager::Instance().loggingEnabled())
	{
		JHI_LOGGER_EXIT_MACRO("JHISVC",JHISVC_CREATESESSION_EXIT,ulRetCode);
	}

	if (ulRetCode != JHI_SUCCESS)
	{
		WriteToEventLog(JHI_EVENT_LOG_WARNING, MSG_CREATE_SESSION_FAILURE);
	}

	return ulRetCode ;
}