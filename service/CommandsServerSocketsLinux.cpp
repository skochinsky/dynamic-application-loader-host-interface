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

#include "CommandsServerSocketsLinux.h"
#include "dbg.h"
#include "reg.h"
#include <iostream>
#include <sstream>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <cstddef>
#include <sys/stat.h>
#include <misc.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

namespace intel_dal
{
	CommandsServerSocketsLinux::CommandsServerSocketsLinux(ICommandDispatcher* dispatcher, uint8_t maxClientNum)
		: ICommandsServer(dispatcher, maxClientNum)
	{
		_socket = INVALID_SOCKET;
	}

	CommandsServerSocketsLinux::~CommandsServerSocketsLinux()
	{
		TRACE0("in ~CommandsServerSocketsLinux()\n");
		if (_socket != INVALID_SOCKET)
			::close(_socket);
	}

	bool CommandsServerSocketsLinux::open()
	{
        int iResult;

        char socket_path[] = "/tmp/jhi_socket";
        // TODO: Change to this path
        // char socket_path[] = "/var/run/jhi_socket";

        sockaddr_un addr;

		//sockaddr_in6 ipv6_socket_data;
		//sockaddr_in  ipv4_socket_data;

		//void* socket_data = NULL;

		//socklen_t socket_data_size = 0;
		//int port_number;
		bool status = false;


		//struct addrinfo hints;
		//struct addrinfo *result = NULL;
		//struct addrinfo *ptr = NULL;

		//memset( &hints, 0, sizeof(hints));
		//hints.ai_family = AF_UNSPEC;
		//hints.ai_socktype = SOCK_STREAM;
		//hints.ai_protocol = IPPROTO_TCP;

		//memset(&ipv4_socket_data, 0, sizeof(ipv4_socket_data));
		//memset(&ipv6_socket_data, 0, sizeof(ipv6_socket_data));

		do
		{

			if (!_dispatcher->init())
			{
				TRACE0("dispatcher init failed\n");
				break;
			}


			//if (getaddrinfo("localhost",NULL,&hints,&result) != 0)
			//{
			//	TRACE0("failed to get adderss info\n");
			//	break;
			//}

			//if (result == NULL)
//			{
//				TRACE0("no adderss info recieved\n");
//				break;
//			}

			// select address of ipv4 or ipv6 family
//			for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
//			{
//				if (ptr->ai_family == AF_INET || ptr->ai_family == AF_INET6)
//					break;
//			}

//			if (ptr == NULL)
//			{
//				TRACE0("failed to find IPV4 or IPV6 address\n");
//				break;
//			}

//			_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            _socket = socket(AF_UNIX, SOCK_STREAM, PF_UNSPEC);

			if (_socket == INVALID_SOCKET)
			{
				TRACE1("socket() failed with error: %d\n", errno);
				break;
			}

//			if (bind(_socket, ptr->ai_addr, ptr->ai_addrlen) == SOCKET_ERROR)
//			{
//				TRACE1("bind() failed with error: %d\n", errno);
//				break;
//			}

            addr.sun_family = AF_UNIX;
            strcpy(addr.sun_path, socket_path);


            // From the second run on an unlink is needed to clear the socket from the previous run.
            unlink(socket_path);

            if (bind(_socket, (sockaddr *)&addr, offsetof(sockaddr_un, sun_path) + strlen(addr.sun_path) + 1) == SOCKET_ERROR)
			{
				TRACE1("bind() failed with error: %d\n", errno);
				break;
			}

            // TODO: Fix permissions
            chmod(socket_path, 0777);

//			if (ptr->ai_family == AF_INET)
//			{
//				socket_data = &ipv4_socket_data;
//				socket_data_size = sizeof(ipv4_socket_data);
//			}
//			else // ipv6
//			{
//				socket_data = &ipv6_socket_data;
//				socket_data_size = sizeof(ipv6_socket_data);
//			}
//
//			if (getsockname(_socket,(sockaddr*)socket_data,&socket_data_size) != 0)
//			{
//				TRACE1("getsockname() failed with error: %d\n", errno);
//				break;
//			}
//
//			if (ptr->ai_family == AF_INET)
//			{
//				port_number = ntohs(ipv4_socket_data.sin_port);
//			}
//			else // ipv6
//			{
//				port_number = ntohs(ipv6_socket_data.sin6_port);
//			}
//
//			iResult = JhiWritePortNumberToRegistry(port_number);
//			if (iResult != JHI_SUCCESS)
//			{
//				TRACE0("failed to write service port at registry.");
//				WriteToEventLog(JHI_EVENT_LOG_ERROR, MSG_REGISTRY_WRITE_ERROR);
//				break;
//			}
//
//			iResult = JhiWriteAddressTypeToRegistry(ptr->ai_family);
//			if (iResult != JHI_SUCCESS)
//			{
//				TRACE0("failed to write address type at registry.");
//				WriteToEventLog(JHI_EVENT_LOG_ERROR, MSG_REGISTRY_WRITE_ERROR);
//				break;
//			}
//
			iResult = listen(_socket, SOMAXCONN);
			if (iResult == SOCKET_ERROR)
			{
				TRACE1("listen failed with error: %d\n", errno);
				break;
			}

			status = true;
		}
		while (0);

		// cleanup
		if (status == false)
		{
			if (_socket != INVALID_SOCKET)
			{
				::close(_socket);
				_socket = INVALID_SOCKET;
			}
		}

//		if (result != NULL)
//			freeaddrinfo(result);

		return status;
	}

	bool CommandsServerSocketsLinux::close()
	{
		if (::close(_socket) == SOCKET_ERROR)
		{
			TRACE0("failed to close socket\n:");
			return false;
		}

		if (!_dispatcher->deinit())
		{
			TRACE0("dispatcher deinit has failed\n:");
			return false;
		}

		return true;
	}

	void CommandsServerSocketsLinux::waitForRequests()
	{

		SOCKET clientSocket;

		while (_socket != INVALID_SOCKET)
		{
			// acquire max client semaphore
			getSemaphore()->Acquire();

			// Accept a client socket
			clientSocket = accept(_socket, NULL, NULL);
			if (clientSocket == INVALID_SOCKET) {
				TRACE1("accept failed with error: %d\n", errno);
				getSemaphore()->Release();
				break;
			}

			startClientSession(clientSocket);
		}
	}

	uint32_t blockedRecv(SOCKET socket, uint8_t* buffer, uint32_t length)
	{
		uint32_t bytesRecieved = 0;
		int count;

		while (bytesRecieved != length)
		{
				count = recv(socket, buffer + bytesRecieved, length - bytesRecieved, 0);

				if (count == SOCKET_ERROR || count == 0) // client closed the connection
					break;

				bytesRecieved += count;
		}

		return bytesRecieved;
	}

	uint32_t blockedSend(SOCKET socket, char* buffer, uint32_t length)
	{
		uint32_t bytesSent = 0;
		int count;

		while (bytesSent != length)
		{
				count = send(socket, buffer + bytesSent, length - bytesSent, 0);

				if (count == SOCKET_ERROR) // client closed the connection
					break;

				bytesSent += count;
		}

		return bytesSent;
	}

	void* ClientSessionThread(void* threadParam)
	{
		uint iResult;
		uint32_t inputBufferSize = 0;
		uint8_t* inputBuffer = NULL;
		uint8_t* outputBuffer = NULL;
		uint32_t outputBufferSize = 0;


		CS_ClientThreadParams* params = (CS_ClientThreadParams*) threadParam;
		SOCKET clientSocket = params->clientSocket;
		ICommandDispatcher* dispatcher = params->dispatcher;
		Semaphore* semaphore = params->semaphore;

		JHI_DEALLOC_T(params);
		params = NULL;

		do
		{

			iResult = blockedRecv(clientSocket, (uint8_t*) &inputBufferSize, sizeof(uint32_t));
			if (iResult !=  sizeof(uint32_t))
			{
				TRACE1("recv inputBufferSize failed with error: %d\n", errno);
				break;
			}

			if ((inputBufferSize < sizeof(JHI_COMMAND)) || (inputBufferSize > JHI_MAX_TRANSPORT_DATA_SIZE))
				break;

			// allocate new buffer
			inputBuffer = (uint8_t*) JHI_ALLOC(inputBufferSize);
			if (NULL == inputBuffer)
			{
				TRACE0("malloc of InputBuffer failed .");
				break;
			}

			iResult = blockedRecv(clientSocket, inputBuffer, inputBufferSize);
			if (iResult != inputBufferSize)
			{
				TRACE1("recv InputBuffer failed with error: %d\n", errno);
				break;
			}


			// prosess command here using the dispatcher
			dispatcher->processCommand((const uint8_t*) inputBuffer,inputBufferSize,(uint8_t**) &outputBuffer,&outputBufferSize);

			// sending the OutputBufferSize
			iResult = blockedSend(clientSocket,(char*) &outputBufferSize,sizeof(uint32_t));
			if (iResult != sizeof(uint32_t))
			{
				TRACE1("send outputBufferSize failed with error: %d\n", errno);
				break;
			}

			if (outputBufferSize > 0)
			{
				// sending the outputBuffer
				iResult = blockedSend(clientSocket,(char*) outputBuffer, outputBufferSize);
				if (iResult != outputBufferSize)
				{
					TRACE1("send outputBuffer failed with error: %d\n", errno);
					break;
				}

			}

			// closing the sockets for send operations, since no more data will be sent
			if (shutdown(clientSocket, SHUT_WR) == SOCKET_ERROR)
			{
				TRACE1("shutdown for send operations failed with error: %d\n", errno);
				break;
			}

		}
		while(0);

		//cleanup:

		if (inputBuffer != NULL)
		{
			JHI_DEALLOC(inputBuffer);
			inputBuffer = NULL;
		}
		if (outputBuffer != NULL)
		{
			JHI_DEALLOC(outputBuffer);
			outputBuffer = NULL;
		}

		// closing the conection to client
		if (::close(clientSocket) == SOCKET_ERROR)
		{
			TRACE1("close client socket failed: %d\n", errno);
		}
		clientSocket = INVALID_SOCKET;

		//release Max Clients semaphore
		semaphore->Release();

		return 0;
	}

	void CommandsServerSocketsLinux::startClientSession(SOCKET clientSocket)
	{
		// create a thread to process the client request
		pthread_t clientThread;

		CS_ClientThreadParams* params = NULL;

		params = JHI_ALLOC_T(CS_ClientThreadParams);
		if (params == NULL)
		{
			TRACE0("CS_ClientThreadParams memory allocation failed");
			getSemaphore()->Release();
			return;
		}

		params->clientSocket = clientSocket;
		params->dispatcher = _dispatcher;
		params->semaphore = getSemaphore();

		int rc = pthread_create(&clientThread, NULL, ClientSessionThread, (void*)params);
		if(rc)
		{
			TRACE0("failed creating thread for client request\n");
		}
		else
		{
			pthread_detach(clientThread);
		}
	}
}//namespace intel_dal
