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

#include "AppletsManager.h"
#include "misc.h"
#include <stdio.h>
#include "AppletsPackageReader.h"
#include "reg.h"
#ifdef _WIN32
#include <io.h>
#else
#include <string.h>
#include <sys/stat.h>
#include "string_s.h"
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#endif //_WIN32


#include <fstream>
#include <iterator>
#include <algorithm>


namespace intel_dal
{
    const unsigned char BH_MSG_RESPONSE[] = { 0xff, 0xa5, 0xaa, 0x55 };
    const char* TL_VM_PORT = "3001";
    const int BH_VM_PORT = 10000;
    const int INVALID_TL_COMMAND_SIZE = 92; // The size of an invalid SCHANNEL6_OPEN_CLIENT_SESSION_COMMAND
    const int INVALID_TL_RESPONSE_SIZE = 100; // The size of an invalid response
    const int N_MESSAGE_SIZE_OFFSET = 0;
    const int N_MESSAGE_TYPE_OFFSET = 1;
    const int N_PARAM_TYPES_OFFSET = 2;
    const int S_PARAMS_N_SIZE_OFFSET = 44;
    
	AppletsManager::AppletsManager() : _appletTable()
	{
		memset(&_currentFwVersion, 0, sizeof(VERSION));
	}

	// Decide which plugin type to load (TL/BHv1/BHv2) according to FW version
	bool AppletsManager::setPluginToLoad()
	{
		_loadedPlugin = JHI_PLUGIN_TYPE_INVALID;

#ifdef __ANDROID__ // Android, always Beihai
		if (_currentFwVersion.Major == 1 && _currentFwVersion.Minor == 1)
		{
			TRACE0( "FW type - SEC\n" ) ;
			TRACE0( "According to FW version, BEIHAI Plugin should be loaded.\n" ) ;
			_fwType = SEC;
			_loadedPlugin = JHI_PLUGIN_TYPE_BEIHAI_V1;
			return true;
		}
#endif
		// VLV
		if (_currentFwVersion.Major == 1)
		{
			_fwType = SEC;
			if (_currentFwVersion.Minor == 0 || _currentFwVersion.Minor == 1)
			{
				_loadedPlugin = JHI_PLUGIN_TYPE_TL;
				TRACE0("FW type - SEC\nLoading plugin: TL\n");
			}
			else // VLV1.2 and onward
			{
				_loadedPlugin = JHI_PLUGIN_TYPE_BEIHAI_V1;
				TRACE0("FW type - SEC\nLoading plugin: Beihai v1\n");
			}
		}
		
		// CHV
		else if (_currentFwVersion.Major == 2)
		{
			_fwType = SEC;
			_loadedPlugin = JHI_PLUGIN_TYPE_BEIHAI_V1;
			TRACE0("FW type - SEC\nLoading plugin: Beihai v1\n");
		}

		// BXT
		else if (_currentFwVersion.Major == 3)
		{
			_fwType = CSE;
			_loadedPlugin = JHI_PLUGIN_TYPE_BEIHAI_V2;
			TRACE0("FW type - CSE (BXT)\nLoading plugin: Beihai v2\n");
		}

		// ME 7 and 8
		else if (_currentFwVersion.Major == 7 || _currentFwVersion.Major == 8)
		{
			_fwType = ME;
			_loadedPlugin = JHI_PLUGIN_TYPE_TL;
			TRACE0("FW type - ME\nLoading plugin: TL\n");

		}

		// ME 9
		else if (_currentFwVersion.Major == 9)
		{
			_fwType = ME;
			TRACE0("FW type - ME\n");
			
			JHI_VM_TYPE vmType = discoverVmType();
			if (vmType == JHI_VM_TYPE_TL)
			{
				_loadedPlugin = JHI_PLUGIN_TYPE_TL;
				TRACE0("Loading plugin: TL\n");
			}
			else if (vmType == JHI_VM_TYPE_BEIHAI)
			{
				_loadedPlugin = JHI_PLUGIN_TYPE_BEIHAI_V1;
				TRACE0("Loading plugin: Beihai v1\n");
			}
			else
			{
				_loadedPlugin = JHI_PLUGIN_TYPE_INVALID;
				TRACE0("ERROR: Failed to retrieve the VM type from the FW\n");
			}
		}

		// ME 10
		else if (_currentFwVersion.Major == 10)
		{
			_fwType = ME;
			_loadedPlugin = JHI_PLUGIN_TYPE_BEIHAI_V1;
			TRACE0("FW type - ME\nLoading plugin: Beihai v1\n");
		}

		// CSME
		else if (_currentFwVersion.Major >= 11)
		{
			_fwType = CSE;
			_loadedPlugin = JHI_PLUGIN_TYPE_BEIHAI_V2;
			TRACE0("FW type - CSE\nLoading plugin: Beihai v2\n");
		}

		// Unknown
		else
		{
			_fwType = INVALID_PLATFORM_ID;
			_loadedPlugin = JHI_PLUGIN_TYPE_INVALID;
			TRACE0("Failed to determine FW version from FU client\n");
			return false;
		}

		if (_loadedPlugin == JHI_PLUGIN_TYPE_INVALID)
			return false;
		else
			return true;
	}

	JHI_PLATFROM_ID AppletsManager::getFWtype()
	{
		return _fwType;
	}

	AppletsManager::~AppletsManager(void)
	{
	}

	bool AppletsManager::Initialize()
	{
		if (getFWVersionFromFW())
		{
			return setPluginToLoad();
		}

#if (VER_MAJOR==11) // w/a for SPT emulator
		_currentFwVersion.Major = 11;
		_loadedPlugin = JHI_PLUGIN_TYPE_BEIHAI_V2;
		_fwType = CSE;
		return true;
#endif //VER_MAJOR==11

#if (VER_MAJOR==1) // w/a for FWUpdate
		_currentFwVersion.Major = 1;
		_currentFwVersion.Minor = 1;
		_loadedPlugin = JHI_PLUGIN_TYPE_BEIHAI_V1;
		_fwType = SEC;
		return true;
#endif //VER_MAJOR==1

#if (VER_MAJOR==3) //workaroudn for Linux dev phase. //TODO: Sort out this function
        _currentFwVersion.Major = 3;
        _currentFwVersion.Minor = 0;
        _loadedPlugin = JHI_PLUGIN_TYPE_BEIHAI_V2;
        _fwType = CSE;
        return true;
#endif
		return false;
	}

	JHI_PLUGIN_TYPE AppletsManager::getPluginType()
	{
		return _loadedPlugin;
	}


	bool AppletsManager::getFWVersionFromFW()
	{
		IFirmwareInfo* fwInfo = FWInfoFactory::createInstance();
		uint8_t triesCount = 0;
		bool versionRecieved = false;

		if (fwInfo == NULL)
		{
			TRACE0("Failed to create IFirmwareInfo instance\n");
			return false;
		}

		while (triesCount < 3)
		{
			++triesCount;

			if (!fwInfo->Connect())
			{
				TRACE0("Failed to connect to FU client\n");
				continue;
			}

			if ( (fwInfo->GetFwVersion(&_currentFwVersion))
				&& (fwInfo->GetPlatformType(&_platformType))
				&& (_currentFwVersion.Major != 0) )
			{
				versionRecieved = true;
			}
			else
			{
				TRACE1("Failed to get FW Version, attempt number %d\n", triesCount);
			}

			if (!fwInfo->Disconnect())
			{
				TRACE0("Failed to disconnect from FU client\n");
			}

			if (versionRecieved)
				break;
		}

		if (fwInfo != NULL)
			JHI_DEALLOC_T(fwInfo);

		if (triesCount == 3) //failed getting the fw version
			return false;


		TRACE4("\nFW Version:\nMajor: %d\nMinor: %d\nHotfix: %d\nBuild: %d\n\n",
			_currentFwVersion.Major,
			_currentFwVersion.Minor,
			_currentFwVersion.Hotfix,
			_currentFwVersion.Build);

		return true;
	}

	bool AppletsManager::getFWVersionString(char fw_version[FW_VERSION_STRING_MAX_LENGTH])
	{
		if (sprintf_s(fw_version,FW_VERSION_STRING_MAX_LENGTH,"%d.%d.%d.%d",_currentFwVersion.Major,_currentFwVersion.Minor,_currentFwVersion.Hotfix,_currentFwVersion.Build) != 4)
			return false;

		return true;
	}

	VERSION AppletsManager::getFWVersion()
	{
		return _currentFwVersion;
	}

	JHI_RET AppletsManager::prepareInstallFromFile(const FILESTRING& file, list<vector<uint8_t> >& appletBlobs,const string& appletId, bool isAcp)
	{
		JHI_RET ulRetCode = JHI_SUCCESS;
		int copy_status;
		FILESTRING DstFile;

		do
		{
			// copy the file to the repository and set applet source
			DstFile = getPendingFileName(appletId, isAcp);

#ifdef _WIN32

			copy_status = CopyFile(file.c_str(),(LPCWSTR) DstFile.c_str(), FALSE);

			if (!copy_status) // zero if copy file fails
			{
				TRACE0 ("Copy file to repository failed!!\n");
				ulRetCode = JHI_FILE_ERROR_COPY;
				break;
			} 

			// remove all attributes (readonly, hidden etc.) from the copied file
			if (SetFileAttributes((LPCWSTR) DstFile.c_str(), FILE_ATTRIBUTE_NORMAL) == 0)
			{
				TRACE0 ("failed removing all attributes from file\n");
				ulRetCode = JHI_FILE_ERROR_COPY;
				break;
			}


#else //!_win32
			copy_status = JhiUtilCopyFile(DstFile.c_str(), file.c_str());
			if (copy_status)
			{
				TRACE0 ("Copy file to repository failed!!\n");
				ulRetCode = JHI_FILE_ERROR_COPY;
				break;
			}
			if (chmod(DstFile.c_str(), S_IRWXO | S_IRWXG | S_IRWXU) != 0)
			{
				TRACE0 ("failed removing all attributes from file\n");
				ulRetCode = JHI_FILE_ERROR_COPY;
				break;
			}
#endif //win32


			// 3. get the applet blob from the dalp file
			ulRetCode = getAppletBlobs(DstFile,appletBlobs, isAcp);
			if (ulRetCode != JHI_SUCCESS)
			{
				TRACE0("failed getting applet blobs from dalp file\n");
				break;
			}

			//4. if the applet is not installed (we dont have a record in the app table) 
			//   create entry for the applet under its ID and set its state to PENDING.
			//   otherwise do nothing (the applet is installed but we install it again in case there is a version update)
			if (getAppletState(appletId) == NOT_INSTALLED)
			{
				AppletRecord record;
				record.status = PENDING_INSTALL;
				record.sharedSessionSupport = false;
				record.sharedSessionSupportRetrievedFromFW = false;
				addAppRecordEntry(appletId,record);
			}
		}
		while(0);

		// cleanup
		if (ulRetCode != JHI_SUCCESS)
		{
			// delete the file copied to the repository
			if ((!DstFile.empty()) && (_waccess_s(DstFile.c_str(),0) == 0))
			{
				_wremove(DstFile.c_str());
			}
		}

		return ulRetCode;
	}

	JHI_RET AppletsManager::prepareInstallFromBuffer(vector<uint8_t>& appletBlob, const string& appletId)
	{
		JHI_RET ulRetCode = JHI_SUCCESS;
		FILESTRING DstFile;

		do
		{
			// copy the file to the repository and set applet source
			DstFile = getPendingFileName(appletId, true);

#ifdef _WIN32
			std::fstream fWriter(DstFile.c_str(), std::ios::out | std::ios::binary);
			fWriter.write((const char*)&appletBlob[0], appletBlob.size());
			fWriter.close();

			// verify the applet file
			if (_waccess_s(DstFile.c_str(), 0) != 0)
			{
				TRACE0("prepere install failed - applet file not written properly");
				ulRetCode = JHI_FILE_ERROR_COPY;
				break;
			}

			// remove all attributes (readonly, hidden etc.) from the copied file
			if (SetFileAttributes((LPCWSTR) DstFile.c_str(), FILE_ATTRIBUTE_NORMAL) == 0)
			{
				TRACE0 ("failed removing all attributes from file\n");
				ulRetCode = JHI_FILE_ERROR_COPY;
				break;
			}

#else //!_WIN32
			//TODO: linux copy //copy_status = JhiUtilCopyFile (DstFile.c_str(),(const char*) file.c_str());
			if (JhiUtilCreateFile_fromBuff (DstFile.c_str(),reinterpret_cast<const char*>(&appletBlob[0]), appletBlob.size())) {
				TRACE0("prepere install failed - applet file is not created");
				ulRetCode = JHI_FILE_ERROR_COPY;
				break;
			}
			if (_waccess_s(DstFile.c_str(), 0) != 0)
			{
				TRACE0("prepere install failed - applet file not written properly");
				ulRetCode = JHI_FILE_ERROR_COPY;
				break;
			}

			if (chmod(DstFile.c_str(), S_IRWXO | S_IRWXG | S_IRWXU) != 0)
			{
				TRACE0 ("failed removing all attributes from file\n");
				ulRetCode = JHI_FILE_ERROR_COPY;
				break;
			}

#endif //_WIN32

			//4. if the applet is not installed (we dont have a record in the app table) 
			//   create entry for the applet under its ID and set its state to PENDING.
			//   otherwise do nothing (the applet is installed but we install it again in case there is a version update)
			if (getAppletState(appletId) == NOT_INSTALLED)
			{
				AppletRecord record;
				record.status = PENDING_INSTALL;
				record.sharedSessionSupport = false;
				record.sharedSessionSupportRetrievedFromFW = false;
				addAppRecordEntry(appletId,record);
			}
		}
		while(0);

		// cleanup
		if (ulRetCode != JHI_SUCCESS)
		{
			// delete the file copied to the repository
			if ((!DstFile.empty()) && (_waccess_s(DstFile.c_str(),0) == 0))
			{
				_wremove(DstFile.c_str());
			}
		}

		return ulRetCode;
	}

	bool AppletsManager::compareFileExtention(const FILESTRING& file, const string& extention)
	{
		FILESTRING ext; 
		int index = file.rfind('.');

		if (index==string::npos)
			return false; // no extention found.

		ext = file.substr(index); // get the extention

		if ( ext.size() != extention.size() ) // compare sizes
			return false;

		for ( string::size_type i = 0; i < extention.size(); ++i ) //compare chars
			if (toupper(extention[i]) != toupper(ext[i]))
				return false;

		return true;
	}

	void AppletsManager::addAppRecordEntry(const string& AppId, const AppletRecord& record)
	{
		_locker.Lock();
		_appletTable.insert(pair<string, AppletRecord>(AppId, record));
		_locker.UnLock();
	}

	bool AppletsManager::completeInstall(const string& appletId, bool isAcp)
	{
		int result;

		// rename the applet file in the repository from PENNDING_<UUID>.dalp to <UUID>.dalp
		FILESTRING pendingFileName = getPendingFileName(appletId, isAcp);
		FILESTRING newfilename = getFileName(appletId, isAcp);
		FILESTRING otherExistingFilename = getFileName(appletId, !isAcp); // needed to remove old file in case it was with a different extension.

		// delete an exsiting file with newfilename
		_wremove(newfilename.c_str()); // no need to check since rename will fail
		_wremove(otherExistingFilename.c_str()); // no need to check since rename will fail

		// rename the temp file to the newfilename
		result = _wrename( pendingFileName.c_str() , newfilename.c_str() );

		if ( result != 0 )
		{
			TRACE0("rename file failed\n");
			return false;
		}

		// change the status in the applet table to INSTALLED
		_locker.Lock();
		_appletTable[appletId].status = INSTALLED;
		_locker.UnLock();

		return true;
	}

	bool AppletsManager::appletExistInRepository(IN const string& appletId, OUT FILESTRING* outFileName, OUT bool& isAcp)
	{
		bool exists = false;

		FILESTRING dalpfilename = getFileName(appletId, false);
		FILESTRING acpfilename = getFileName(appletId, true);

		if (_waccess_s(dalpfilename.c_str(), 0) == 0)
		{
			exists = true;
			isAcp = false;

			if (outFileName != NULL)
			{
				*outFileName = dalpfilename;
			}
		}
		else
		{
			if (_waccess_s(acpfilename.c_str(), 0) == 0)
			{
				exists = true;
				isAcp = true;

				if (outFileName != NULL)
				{
					*outFileName = acpfilename;
				}
			}
		}
		return exists;
	}

	bool AppletsManager::remove(const string& appletId)
	{
		_locker.Lock();
		int ret = _appletTable.erase(appletId);
		_locker.UnLock();

		return (ret != 0);
	}


	bool AppletsManager::get(const string& appletId, AppletRecord& appRecord)
	{
		bool status = true;

		_locker.Lock();

		do
		{
			if (!isAppletRecordPresent(appletId))
			{
				status = false;
				break;
			}
			appRecord = _appletTable[appletId];
		}
		while(0);

		_locker.UnLock();

		return status;
	}

	bool AppletsManager::isAppletRecordPresent(const string& appletId)
	{
		map<string, AppletRecord>::iterator it;

		it = _appletTable.find(appletId);

		return (it != _appletTable.end());
	}

	JHI_RET AppletsManager::readFileAsBlob(const FILESTRING& filepath, list< vector<uint8_t> >& appletBlobs)
	{
		JHI_RET ret = JHI_INVALID_PARAMS;
//#ifdef _WIN32
		std::ifstream is(filepath.c_str(), std::ios::binary);

		if (!is)
		{
			return JHI_INTERNAL_ERROR;
		}

		try
		{
			is >> std::noskipws;
			is.seekg (0, is.end);
			std::streamoff len = is.tellg();
			is.seekg (0, is.beg);

			if (len >= MAX_APPLET_BLOB_SIZE)
			{
				ret = JHI_INVALID_PACKAGE_FORMAT;
			}
			std::istream_iterator<uint8_t> start(is), end;
			vector<uint8_t> blob(start, end);
			appletBlobs.push_back(blob);
			is.close();
			ret = JHI_SUCCESS;
		}
		catch(...)
		{
			if (is.is_open())
			{
				is.close();				
			}
			ret =  JHI_INVALID_PARAMS;
		}
//#else //!WIN32
//		struct stat fileStats;
//		int fileSize;
//		if(stat(filepath.c_str(), &fileStats) != -1) {
//			fileSize = fileStats.st_size;
//		} else {
//			return JHI_INVALID_PARAMS;
//		}
//		std::ifstream appFile(filepath.c_str(), std::ios::binary);
//		if (!appFile)
//		{
//			return JHI_INTERNAL_ERROR;
//		}
//		vector<uint8_t> blob;
//
//		if (fileSize >= MAX_APPLET_BLOB_SIZE)
//		{
//			ret = JHI_INVALID_PACKAGE_FORMAT;
//			goto end;
//		}
//		blob.reserve(fileSize);
//		appFile.read( reinterpret_cast<char *>(&blob[0]), fileSize );
//		appletBlobs.push_back(blob);
//		ret = JHI_SUCCESS;
//end:
//		if (appFile.is_open())
//		{
//			appFile.close();
//		}
//#endif //WIN32
		return ret;
	}

	JHI_RET AppletsManager::getAppletBlobs(const FILESTRING& filepath, list< vector<uint8_t> >& appletBlobs, bool isAcp)
	{
		if (isAcp)
		{
			return readFileAsBlob(filepath, appletBlobs);
		}

		char FWVersionStr[FW_VERSION_STRING_MAX_LENGTH];

		if (compareFileExtention(filepath,dalpFileExt))
		{
			AppletsPackageReader reader(filepath);

			if (!reader.isPackageValid())
			{
				TRACE0 ("Invalid package file received\n");
				return JHI_INVALID_PACKAGE_FORMAT;
			}

			// create a fw version string to compare against the versions in the dalp file.
			sprintf_s(FWVersionStr, FW_VERSION_STRING_MAX_LENGTH, "%d.%d.%d",_currentFwVersion.Major,_currentFwVersion.Minor,_currentFwVersion.Hotfix);

			if (!reader.getAppletBlobs(FWVersionStr,appletBlobs))
			{
				TRACE0 ("get applet blob from dalp file failed!!\n");
				return JHI_READ_FROM_FILE_FAILED;
			}

			if (appletBlobs.empty())
			{
				TRACE0 ("No compatible applets where found in the dalp file\n");
				return JHI_INSTALL_FAILED;
			}
		}
		else
		{
			return JHI_INVALID_FILE_EXTENSION;
		}

		return JHI_SUCCESS;
	}


	JHI_APPLET_STATUS AppletsManager::getAppletState(const string& appletId)
	{
		JHI_APPLET_STATUS status;

		_locker.Lock();

		do
		{
			if (!isAppletRecordPresent(appletId))
			{
				status = NOT_INSTALLED;
				break;
			}

			status = _appletTable[appletId].status;
		}
		while(0);

		_locker.UnLock();

		return status;
	}

	bool AppletsManager::isSharedSessionSupported(const string& appletId)
	{
		bool shareSupported = false;

		_locker.Lock();

		if (isAppletRecordPresent(appletId))
		{
			if (!_appletTable[appletId].sharedSessionSupportRetrievedFromFW)
			{
				updateSharedSessionSupport(appletId);
			}
			shareSupported = _appletTable[appletId].sharedSessionSupport;
		}

		_locker.UnLock();

		return shareSupported;
	}

	ostream& operator <<(ostream& os, const AppletsManager& am)
	{	
		map<string, AppletRecord>::const_iterator it;

		for ( it = am._appletTable.begin() ; it != am._appletTable.end(); it++ )
		{
			os << "Applet ID: "<<(*it).first << "\n";
			os << "Session State: "<<(*it).second.status << "\n";
			os << "\n";
		}

		return os;
	}

	void AppletsManager::resetAppletTable()
	{
		_locker.Lock();
		_appletTable.clear();
		_locker.UnLock();
	}

	bool AppletsManager::UnloadUnusedApplets()
	{
		list<string> appidList;
		map<string, AppletRecord>::const_iterator it;
		bool unloaded = false;
		char Appid[LEN_APP_ID+1];

		_locker.Lock();

		for ( it = _appletTable.begin() ; it != _appletTable.end(); it++ )
		{
			if (it->second.status == INSTALLED)
				appidList.push_back(it->first);
		}

		for (list<string>::iterator list_it = appidList.begin(); list_it != appidList.end(); list_it++)
		{
			strcpy_s(Appid,LEN_APP_ID + 1, list_it->c_str());
			if (jhis_unload(Appid) == JHI_SUCCESS)
			{
				TRACE1("unloaded applet with appid: %s\n",Appid);
				unloaded = true;
			}
		}

		_locker.UnLock();

		return unloaded;
	}

	void AppletsManager::updateSharedSessionSupport(const string& appletId)
	{
		char appId[LEN_APP_ID+1];
		JVM_COMM_BUFFER ioBuffer;
		const char * appProperty = "applet.shared.session.support";
		const int responseLen = 6;
		char responseStr[responseLen];
		bool shareEnabled = false;

		JHI_RET status;

		strcpy_s(appId,LEN_APP_ID+1,appletId.c_str());

		ioBuffer.TxBuf->buffer = (void*)appProperty;
		ioBuffer.TxBuf->length = strlen(appProperty)+1;

		ioBuffer.RxBuf->buffer = responseStr;
		ioBuffer.RxBuf->length = responseLen;

		status = jhis_get_applet_property(appId,&ioBuffer);

		if (status == JHI_SUCCESS)
		{
			_appletTable[appletId].sharedSessionSupportRetrievedFromFW = true;
			if (strcmp(responseStr, "true") == 0)
				shareEnabled = true;
		}

		_locker.Lock();
		_appletTable[appletId].sharedSessionSupport = shareEnabled;
		_locker.UnLock();
	}

	void AppletsManager::updateAppletsList()
	{
		vector<string> uuidsInFw, uuidsInRepo;
		FILESTRING repositoryDir;
#ifdef _WIN32
		// code based on http://msdn.microsoft.com/en-us/library/windows/desktop/aa365200(v=vs.85).aspx
		WIN32_FIND_DATA ffd;
		FILESTRING searchStr;
		HANDLE hFind = INVALID_HANDLE_VALUE;

		GlobalsManager::Instance().getAppletsFolder(repositoryDir);
		repositoryDir.append(FILE_SEPERATOR);

		do	// search dalp files
		{
			TRACE0("Searching dalp TAs in the repository...");
			searchStr = FILESTRING(repositoryDir + FILEPREFIX("*") + ConvertStringToWString(dalpFileExt));

			// Gets the first file in the folder
			hFind = FindFirstFile(searchStr.c_str(), &ffd);

			if (INVALID_HANDLE_VALUE == hFind) 
			{
				TRACE0("FindFirstFile failed.");
				break;
			}

			do
			{
				ffd.cFileName[LEN_APP_ID] = FILEPREFIX('\0');
				string fileName(ConvertWStringToString(ffd.cFileName));
				if (validateUuidString(fileName))
				{
					TRACE1("The TA %s was found in the repository.", fileName.c_str());
					uuidsInRepo.push_back(fileName);
				}
			}
			// Continue on the next dalp file in the folder.
			while (FindNextFile(hFind, &ffd) != 0);
		} while(0);

		do	// search acp files
		{
			TRACE0("Searching acp TAs in the repository...");
			searchStr = FILESTRING(repositoryDir + FILEPREFIX("*") + ConvertStringToWString(acpFileExt));

			// Gets the first file in the folder
			hFind = FindFirstFile(searchStr.c_str(), &ffd);

			if (INVALID_HANDLE_VALUE == hFind) 
			{
				TRACE0("FindFirstFile failed.");
				break;
			}

			do
			{
				ffd.cFileName[LEN_APP_ID] = FILEPREFIX('\0');
				string fileName(ConvertWStringToString(ffd.cFileName));
				if (validateUuidString(fileName))
				{
					TRACE1("The TA %s was found in the repository.", fileName.c_str());
					uuidsInRepo.push_back(fileName);
				}
			}
			// Continue on the next acp file in the folder.
			while (FindNextFile(hFind, &ffd) != 0);
		} while (0);

		// Register the found applets.
		for (auto uuid = uuidsInRepo.begin(); uuid != uuidsInRepo.end(); ++uuid)
		{ 
			if (_stricmp(uuid->c_str(), SPOOLER_APPLET_UUID) == 0)
			{
				continue;
			}
			AppletRecord record;
			record.status = INSTALLED;
			record.sharedSessionSupport = false;
			record.sharedSessionSupportRetrievedFromFW = false;
			addAppRecordEntry(*uuid, record);
		}

#else //ANDROID
		DIR *dir;
		struct dirent *entry;
       		struct stat info;

		GlobalsManager::Instance().getAppletsFolder(repositoryDir);
		repositoryDir.append(FILE_SEPERATOR);

		if ((dir = opendir(const_cast < const char*>(repositoryDir.c_str()))) == NULL)
			TRACE2("Cannot open applets repository dir %s, %s\n",
			       repositoryDir.c_str(), strerror(errno));
		else {
			while ((entry = readdir(dir)) != NULL) {
				std::string filename (entry->d_name);
				std::string appName = repositoryDir + filename;
				if (stat(appName.c_str(), &info) != 0) {
					TRACE2 ("Can't stat %, %s\n", appName.c_str(),strerror(errno));
					continue;
				}

				if ((filename.find (dalpFileExt)) == LEN_APP_ID + 1) {
					uuidsInRepo.push_back(filename.substr (0, LEN_APP_ID));
					//std::string uuid = filename.substr (0, LEN_APP_ID);
					//jhis_install (const_cast <const char*>(uuid.c_str()), const_cast <const FILECHAR*>(appName.c_str()), true, false);
				} else if ((filename.find (acpFileExt)) == LEN_APP_ID + 1) {
					uuidsInRepo.push_back(filename.substr (0, LEN_APP_ID));
					//std::string uuid = filename.substr (0, LEN_APP_ID);
					//jhis_install (const_cast <const char*>(uuid.c_str()), const_cast <const FILECHAR*>(appName.c_str()), true, true);
				} else {
					 continue;
				}
			}
			closedir(dir);
		}
		for (int ii = 0; ii < uuidsInRepo.size(); ii++)
		{
			if (strcmp(uuidsInRepo[ii].c_str(), SPOOLER_APPLET_UUID) == 0)
			{
				continue;
			}
			AppletRecord record;
			record.status = INSTALLED;
			record.sharedSessionSupport = false;
			record.sharedSessionSupportRetrievedFromFW = false;
			addAppRecordEntry(uuidsInRepo[ii], record);
		}

#endif //_WIN32

		return;
	}



	void AppletsManager::getLoadedAppletsList(list<string>& appletsList)
	{
		map<string, AppletRecord>::const_iterator it;
		list<string> applets;

		_locker.Lock();

		for ( it = _appletTable.begin() ; it != _appletTable.end(); it++ )
		{
			if (it->second.status == INSTALLED)
				appletsList.push_back(it->first);
		}

		_locker.UnLock();
	}

	FILESTRING AppletsManager::getPendingFileName(const string& appletId, bool isAcp)
	{
		FILESTRING repositoryDir;
		GlobalsManager::Instance().getAppletsFolder(repositoryDir);
		string fileExt;
		if (isAcp)
		{
			fileExt = acpFileExt;
		}
		else
		{
			fileExt = dalpFileExt;
		}

		return repositoryDir + ConvertStringToWString(pendingHeader + appletId + fileExt);
	}

	FILESTRING AppletsManager::getFileName(const string& appletId, bool isAcp)
	{
		FILESTRING repositoryDir;
		GlobalsManager::Instance().getAppletsFolder(repositoryDir);
		string fileExt;
		if (isAcp)
		{
			fileExt = acpFileExt;
		}
		else
		{
			fileExt = dalpFileExt;
		}
		return repositoryDir + ConvertStringToWString("/" + appletId + fileExt);
	}

    JHI_VM_TYPE AppletsManager::discoverVmType()
    {
		JHI_VM_TYPE vmType = JHI_VM_TYPE_INVALID;
        TEE_TRANSPORT_TYPE internalTransportType = TEE_TRANSPORT_TYPE_INVALID;

        internalTransportType =	GlobalsManager::Instance().getTransportType();

        if ( internalTransportType == TEE_TRANSPORT_TYPE_SOCKET )
        {
			vmType = discoverVmTypeBySocket(); // Handle a socket connection
        }
        else if ( internalTransportType == TEE_TRANSPORT_TYPE_TEE_LIB )
        {
            vmType = discoverVmTypeByTeeLib(); // Handle a HECI connection
        }
        
        return vmType;
    }

	JHI_VM_TYPE AppletsManager::discoverVmTypeBySocket()
    {
        bool isConnected = false;
		JHI_VM_TYPE vmType = JHI_VM_TYPE_INVALID;
        TEE_TRANSPORT_INTERFACE teeTransportInteface = { 0 };
        TEE_TRANSPORT_HANDLE handle = TEE_TRANSPORT_INVALID_HANDLE_VALUE;
        TEE_COMM_STATUS teeCommStatus = TEE_COMM_INTERNAL_ERROR;


        TRACE0("AppletsManager::discoverVmType(), getVmTypeBySocket()\n");

        teeCommStatus = TEE_Transport_Create(::TEE_TRANSPORT_TYPE_SOCKET, &teeTransportInteface);

        if ( teeCommStatus != TEE_COMM_SUCCESS )
        {
            TRACE1("AppletsManager::discoverVmType(), failure in TEE_Transport_Create(), teeCommStatus = %d\n", teeCommStatus);
            return vmType;
        }

        // First try to connect to TL socket
        teeCommStatus = teeTransportInteface.pfnConnect(&teeTransportInteface, TEE_TRANSPORT_ENTITY_CUSTOM, TL_VM_PORT, &handle);

        if ( teeCommStatus == TEE_COMM_SUCCESS )
        {
            TRACE0("AppletsManager::discoverVmType(), Connected to TL VM socket. TEE plugin will be loaded.\n");
			vmType = JHI_VM_TYPE_TL;
            isConnected = true; 
        }
        else
        {
            // Couldn't connect to TL socekt, try to connect to BH socket
            teeCommStatus = teeTransportInteface.pfnConnect(&teeTransportInteface, (TEE_TRANSPORT_ENTITY)BH_VM_PORT, NULL, &handle);

            if ( teeCommStatus == TEE_COMM_SUCCESS )
            {
                TRACE0("AppletsManager::discoverVmType(), Connected to BH VM socket. BEIHAI_V1 plugin will be loaded.\n");
				vmType = JHI_VM_TYPE_BEIHAI;
                isConnected = true;
            }
            else
            {
                // Couldn't connect to BH socket as well, an error will be returned.
                TRACE0("AppletsManager::discoverVmType(), Couldn't connect to BH and TL sockets\n");
            }
        }

        if ( isConnected )
        {
            // Best effort behavior
            teeCommStatus = teeTransportInteface.pfnDisconnect(&teeTransportInteface, &handle);

            if ( teeCommStatus !=  TEE_COMM_SUCCESS )
            {
                TRACE1("AppletsManager::discoverVmType(), failure in pfnDisconnect(), teeCommStatus = %d\n", teeCommStatus);
            }
        }

        teeCommStatus = teeTransportInteface.pfnTeardown(&teeTransportInteface);

        if ( teeCommStatus != TEE_COMM_SUCCESS )
        {
			vmType = JHI_VM_TYPE_INVALID;
            TRACE1("AppletsManager::discoverVmType(), failure in pfnTeardown(), teeCommStatus = %d\n", teeCommStatus);
        }

        return vmType;
    }

	JHI_VM_TYPE AppletsManager::discoverVmTypeByTeeLib()
    {
        bool isConnected = false;
		JHI_VM_TYPE vmType = JHI_VM_TYPE_INVALID;
        uint8_t command[INVALID_TL_COMMAND_SIZE] = { 0 };
        uint8_t response[INVALID_TL_RESPONSE_SIZE] = { 0 };
		uint32_t bytesRead = sizeof(response); // get number bytes that were actually read
        UINT16* nParamTypesPtr = NULL;
        UINT32* nSizePtr = NULL;
        TEE_TRANSPORT_INTERFACE teeTransportInteface = { 0 };
        TEE_TRANSPORT_HANDLE handle = TEE_TRANSPORT_INVALID_HANDLE_VALUE;
        TEE_COMM_STATUS teeCommStatus = TEE_COMM_INTERNAL_ERROR;


        TRACE0("AppletsManager::discoverVmType(), getVmTypeByTeeLib()\n");

        // Build the message to be sent to TL/BH based VM.
        // The message is an invalid SCHANNEL6_OPEN_CLIENT_SESSION_COMMAND (for TL based VM).
        // If the VM is of BH type, then a response contains the prefix BH_MSG_RESPONSE will be returned from BH based VM.
        // Otherwise, a response from TL based VM will be returned without this prefix.

        // Create the command according to SCHANNEL6_OPEN_CLIENT_SESSION_COMMAND struct.

        command[N_MESSAGE_SIZE_OFFSET] = 0x4C; // nMessageSize
        command[N_MESSAGE_TYPE_OFFSET] = 0xF0; // nMessageType

        nParamTypesPtr = (UINT16*)(command + N_PARAM_TYPES_OFFSET); // nParamTypes
        *nParamTypesPtr = 0x0C;
 
        nSizePtr = (UINT32*)(command + S_PARAMS_N_SIZE_OFFSET); // sParams.nSize
        *nSizePtr = 0x08;

        do
        {
            teeCommStatus = TEE_Transport_Create(::TEE_TRANSPORT_TYPE_TEE_LIB, &teeTransportInteface);

            if ( teeCommStatus != TEE_COMM_SUCCESS )
            {
                TRACE1("AppletsManager::discoverVmType(), failure in TEE_Transport_Create(), teeCommStatus = %d\n", teeCommStatus);
                break;
            }

            teeCommStatus = teeTransportInteface.pfnConnect(&teeTransportInteface, TEE_TRANSPORT_ENTITY_IVM, NULL, &handle);

            if ( teeCommStatus != TEE_COMM_SUCCESS )
            {
                TRACE1("AppletsManager::discoverVmType(), failure in pfnConnect(), teeCommStatus = %d\n", teeCommStatus);
                break;
            }

            isConnected = true;

            teeCommStatus = teeTransportInteface.pfnSend(&teeTransportInteface, handle, command, sizeof(command));

            if ( teeCommStatus != TEE_COMM_SUCCESS )
            {
                TRACE1("AppletsManager::discoverVmType(), failure in pfnSend(), teeCommStatus = %d\n", teeCommStatus);
                break;
            }

            teeCommStatus = teeTransportInteface.pfnRecv(&teeTransportInteface, handle, response, &bytesRead);

            if ( teeCommStatus != TEE_COMM_SUCCESS )
            {
                TRACE1("AppletsManager::discoverVmType(), failure in pfnRecv(), teeCommStatus = %d\n", teeCommStatus);
                break;
            }

            if ( bytesRead < sizeof(BH_MSG_RESPONSE) )
            {
                TRACE1("AppletsManager::discoverVmType(), Failure. Number of bytes read from pfnRecv(): %d\n", bytesRead);
                break;
            }

            if ( memcmp(response, BH_MSG_RESPONSE, sizeof(BH_MSG_RESPONSE)) == 0 )
            {
                TRACE0("AppletsManager::discoverVmType(), found BH_MSG_RESPONSE within the received buffer. BEIHAI_V1 plugin will be loaded.\n");
				vmType = JHI_VM_TYPE_BEIHAI;
            }
            else
            {
                TRACE0("AppletsManager::discoverVmType(), did not find BH_MSG_RESPONSE within the received buffer. TEE plugin will be loaded.\n");
				vmType = JHI_VM_TYPE_TL;
            }

        } while(0);

	    // Close HECI/Socket connection

        if ( isConnected )
        {
            teeCommStatus = teeTransportInteface.pfnDisconnect(&teeTransportInteface, &handle);

            if ( teeCommStatus !=  TEE_COMM_SUCCESS )
            {
                // Best effort behavior
                TRACE1("AppletsManager::discoverVmType(), failure in pfnDisconnect(), teeCommStatus = %d\n", teeCommStatus);
            }
        }

        teeCommStatus = teeTransportInteface.pfnTeardown(&teeTransportInteface);

        if ( teeCommStatus != TEE_COMM_SUCCESS )
        {
			vmType = JHI_VM_TYPE_INVALID;
            TRACE1("AppletsManager::discoverVmType(), failure in pfnTeardown(), teeCommStatus = %d\n", teeCommStatus);
        }

        return vmType;
    }
}