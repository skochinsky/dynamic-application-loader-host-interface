/* Copyright 2014 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <assert.h>
#include <windows.h>
#include <SetupAPI.h>
#include <initguid.h>
#include <tchar.h>
#include <libtee\helpers.h>
#include "Public.h"
#include <libtee\libtee.h>

/**********************************************************************
 **                          TEE Lib Function                         *
 **********************************************************************/
TEESTATUS TEEAPI TeeInit(IN OUT PTEEHANDLE handle, IN const UUID *uuid, IN OPTIONAL const char *device)
{
	TEESTATUS       status               = INIT_STATUS;
	TCHAR           devicePath[MAX_PATH] = {0};
	HANDLE          deviceHandle         = INVALID_HANDLE_VALUE;
	LPCGUID         currentUUID          = NULL;

	FUNC_ENTRY();

	if (NULL == uuid || NULL == handle) {
		status = TEE_INVALID_PARAMETER;
		ERRPRINT("One of the parameters was illegal");
		goto Cleanup;
	}

	TEE_INIT_HANDLE(*handle);

	if (device != NULL) {
		currentUUID = (LPCGUID)device;
	}
	else {
		currentUUID = &GUID_DEVINTERFACE_HECI;
	}

	// get device path
	status = GetDevicePath(currentUUID, devicePath, MAX_PATH);
	if (status) {
		ERRPRINT("Error in GetDevicePath, error: %d\n", status);
		goto Cleanup;
	}

	// create file
	deviceHandle = CreateFile(devicePath,
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_FLAG_OVERLAPPED,
					NULL);

	if (deviceHandle == INVALID_HANDLE_VALUE) {
		status = TEE_DEVICE_NOT_READY;
		ERRPRINT("Error in CreateFile, error: %d\n", GetLastError());
		goto Cleanup;
	}

	status = TEE_SUCCESS;

Cleanup:

	if (TEE_SUCCESS == status) {
		handle->handle = deviceHandle;
		error_status_t result  = memcpy_s(&handle->uuid, sizeof(handle->uuid), uuid, sizeof(UUID));
		if (result != 0) {
			ERRPRINT("Error in in uuid copy: result %d\n", result);
			status = TEE_UNABLE_TO_COMPLETE_OPERTAION;
		}
	}
	else {
		CloseHandle(deviceHandle);
	}

	FUNC_EXIT(status);

	return status;
}

TEESTATUS TEEAPI TeeConnect(OUT PTEEHANDLE handle)
{
	TEESTATUS       status        = INIT_STATUS;
	DWORD           bytesReturned = 0;
	FW_CLIENT       fwClient      = {0};


	FUNC_ENTRY();

	if (NULL == handle) {
		status = TEE_INVALID_PARAMETER;
		ERRPRINT("One of the parameters was illegal");
		goto Cleanup;
	}

	status = SendIOCTL(handle->handle, (DWORD)IOCTL_TEEDRIVER_CONNECT_CLIENT,
						(LPVOID)&handle->uuid, sizeof(GUID),
						&fwClient, sizeof(FW_CLIENT),
						&bytesReturned);
	if (status) {
		DWORD err = GetLastError();
		status = Win32ErrorToTee(err);
		ERRPRINT("Error in SendIOCTL, error: %d\n", err);
		goto Cleanup;
	}

	handle->maxMsgLen  = fwClient.MaxMessageLength;
	handle->protcolVer = fwClient.ProtocolVersion;

	status = TEE_SUCCESS;

Cleanup:

	FUNC_EXIT(status);

	return status;
}

TEESTATUS TEEAPI TeeRead(IN PTEEHANDLE handle, IN OUT void* buffer, IN size_t bufferSize,
			 OUT OPTIONAL size_t* pNumOfBytesRead)
{
	TEESTATUS       status = INIT_STATUS;
	EVENTHANDLE     evt    = NULL;

	FUNC_ENTRY();

	if (IS_HANDLE_INVALID(handle) || NULL == buffer || 0 == bufferSize) {
		status = TEE_INVALID_PARAMETER;
		ERRPRINT("One of the parameters was illegal");
		goto Cleanup;
	}

	status = BeginReadInternal(handle->handle, buffer, (ULONG)bufferSize, &evt);
	if (status) {
		ERRPRINT("Error in BeginReadInternal, error: %d\n", status);
		goto Cleanup;
	}

	handle->evt = evt;

	status = EndReadInternal(handle->handle, evt, INFINITE, (LPDWORD)pNumOfBytesRead);
	if (status) {
		ERRPRINT("Error in EndReadInternal, error: %d\n", status);
		goto Cleanup;
	}

	status = TEE_SUCCESS;

Cleanup:
	handle->evt = NULL;

	FUNC_EXIT(status);

	return status;
}

TEESTATUS TEEAPI TeeWrite(IN PTEEHANDLE handle, IN const void* buffer, IN size_t bufferSize,
			  OUT OPTIONAL size_t* numberOfBytesWritten)
{
	TEESTATUS       status = INIT_STATUS;
	EVENTHANDLE     evt    = NULL;

	FUNC_ENTRY();

	if (IS_HANDLE_INVALID(handle) || NULL == buffer || 0 == bufferSize) {
		status = TEE_INVALID_PARAMETER;
		ERRPRINT("One of the parameters was illegal");
		goto Cleanup;
	}

	status = BeginWriteInternal(handle->handle, (PVOID)buffer, (ULONG)bufferSize, &evt);
	if (status) {
		ERRPRINT("Error in BeginWrite, error: %d\n", status);
		goto Cleanup;
	}

	handle->evt = evt;

	status = EndWriteInternal(handle->handle, evt, INFINITE, (LPDWORD)numberOfBytesWritten);
	if (status) {
		ERRPRINT("Error in EndWrite, error: %d\n", status);
		goto Cleanup;
	}

	status = TEE_SUCCESS;

Cleanup:
	handle->evt = NULL;
	FUNC_EXIT(status);

	return status;
}

TEESTATUS TEEAPI TeeCancel(IN PTEEHANDLE handle)
{
	TEESTATUS status = INIT_STATUS;
	DWORD ret;

	FUNC_ENTRY();

	if (!CancelIo(handle->handle)) {
		status = (TEESTATUS)GetLastError();
		goto Cleanup;
	}

	ret = WaitForSingleObject(handle->evt, CANCEL_TIMEOUT);
	if (ret != WAIT_OBJECT_0) {
		ERRPRINT("Error in WaitForSingleObject, return: %d, error: %d\n", ret, GetLastError());
		status = TEE_INTERNAL_ERROR;
		goto Cleanup;
	}

	status = TEE_SUCCESS;

Cleanup:

	FUNC_EXIT(status);

	return status;
}

VOID TEEAPI TeeDisconnect(IN PTEEHANDLE handle)
{
	FUNC_ENTRY();
	if (handle && handle->handle) {
		CloseHandle(handle->handle);
		handle->handle = INVALID_HANDLE_VALUE;
	}
	FUNC_EXIT(0);
}
