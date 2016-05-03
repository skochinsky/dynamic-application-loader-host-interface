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
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <string_s.h>
#include "dbg.h"
#include "socket.h"
#include "errno.h"

#include "reg.h"

DWORD SocketSetup()
{
   /*
   WSADATA wsaData = {0};

   if (0 != WSAStartup(MAKEWORD(2,2), &wsaData)) 
   {
      return ERROR_INTERNAL_ERROR;
   }
    */
    return SOCKET_STATUS_SUCCESS;
}

DWORD SocketTeardown()
{
//    if (0 != WSACleanup())
//    {
//      return ERROR_INTERNAL_ERROR;
//    }
    return SOCKET_STATUS_SUCCESS;
}

DWORD SocketConnect(const char *c_ip, int port, SOCKET* sock)
{
// TODO: Remove unnecessary comments
//    int status = -1;
//
//    do
//    {
//        int _socket = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
//        if (_socket == INVALID_SOCKET)
//        {
//            TRACE1("Couldn't create a socket. error: %d\n", errno);
//            break;
//        }
//        struct sockaddr_in serv_addr;
//
//        serv_addr.sin_family = AF_UNSPEC;
//        serv_addr.sin_port = port;
//        serv_addr.sin_addr.s_addr = INADDR_ANY;
//
//        if(connect(_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
//        {
//            TRACE1("Couldn't connect. Error: %d\n", errno);
//            break;
//        }
//
//        status = SOCKET_STATUS_SUCCESS;
//    }while(0);
//
//    return status;

    struct addrinfo *addr = NULL;
    struct addrinfo hints;
    char port_cstr[20];
    int iResult = 0;

    // Since TEETransport doesn't allow passing an IP address along with the port
    // and adding it requires changes all over the code hierarchy, the IP address
    // retrieval is implemented here locally.
    char ip[16];
    strcpy(ip, SOCK_DEFAULT_IP_ADDRESS);

    JhiQuerySocketIpAddressFromRegistry(ip);

    if(!sock)
    {
      return SOCKET_STATUS_FAILURE;
    }

    // set default value
    *sock = INVALID_SOCKET;

    // check if port is in valid range
    if((port < SOCK_MIN_PORT_VALUE) || (port > SOCK_MAX_PORT_VALUE))
    {
      return SOCKET_STATUS_FAILURE;
    }

    memset( &hints, 0, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if(-1 == sprintf_s(port_cstr, 20, "%d", port))
    {
      return SOCKET_STATUS_FAILURE;
    }

    // Resolve the server address and port
    iResult = getaddrinfo(ip, port_cstr, &hints, &addr);
    if ( iResult != 0 )
    {
        return SOCKET_STATUS_FAILURE;
    }

    if(!addr)
    {
      return SOCKET_STATUS_FAILURE;
    }

    // Create a SOCKET for connecting to server
    *sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (INVALID_SOCKET == *sock)
    {
      freeaddrinfo(addr);
      return SOCKET_STATUS_FAILURE;
    }

    // Connect to server.
    iResult = connect(*sock, addr->ai_addr, (int)addr->ai_addrlen);
    if (iResult == -1)
    {
        TRACE1("Couldn't connect. errno: %d\n", errno);
        close(*sock);
        freeaddrinfo(addr);
        return SOCKET_STATUS_FAILURE;
    }

    freeaddrinfo(addr);

    if (INVALID_SOCKET == *sock)
    {
        return SOCKET_STATUS_FAILURE;
    }

    return SOCKET_STATUS_SUCCESS;
}

DWORD SocketDisconnect(SOCKET sock)
{
    if (INVALID_SOCKET == sock)
    {
        return SOCKET_STATUS_FAILURE;
    }
    else
    {
        close(sock);
        return SOCKET_STATUS_SUCCESS;
    }
}

DWORD SocketSend(SOCKET sock, const char* buffer, int* length)
{
    ssize_t bytes_written = 0;

    if ((INVALID_SOCKET == sock) || (NULL == buffer) || (NULL == length))
    {
        return SOCKET_STATUS_FAILURE;
    }

    bytes_written = send(sock, buffer, *length, 0);

    if (bytes_written == -1)
    {
        TRACE1("send failed. errno: %d", errno);
        *length = 0;
        return SOCKET_STATUS_FAILURE;
    }

    *length = bytes_written;

    // TODO: Remove these prints
//    if(buffer)
//    {
//        printf("\nSocketSend %d: ", *length);
//        for (int i = 0; i < *length; i++)
//            printf("%02x", ((uint8_t *) buffer)[i]);
//        printf("\n\n");
//    }
    // up to here

    return SOCKET_STATUS_SUCCESS;
}

DWORD SocketRecv(SOCKET sock, char* buffer, int* length)
{
    int iResult = 0;

    if ((INVALID_SOCKET == sock) || (NULL == buffer) || (NULL == length))
    {
      return SOCKET_STATUS_FAILURE;
    }

    iResult = recv(sock, buffer, *length, MSG_WAITALL);

    // TODO: Remove these prints
//    if(*length >=16)
//    {
//        printf("\nSocketRecv %d: ", *length);
//        for (int i = 0; i < *length; i++)
//            printf("%02x", ((uint8_t *) buffer)[i]);
//        printf("\n\n");
//    }
    // up to here

    if ((iResult == SOCKET_ERROR) || (iResult < 0))
    {
        *length = 0;
        return SOCKET_STATUS_FAILURE;
    }

    if (iResult > 0)
    {
      // recv - return the number of bytes received
      *length = iResult;
    }
    else if (iResult == 0)
    {
      // recv - connection has been gracefully closed
      *length = 0;
    }

    return SOCKET_STATUS_SUCCESS;
}
