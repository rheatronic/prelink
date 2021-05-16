#include "bitflix/bitflix_debug.h"

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <malloc.h>

void __cdecl suggestMcastAddr(char pAddrString[64])
{
	WSADATA wsaData;
	SOCKET s = INVALID_SOCKET;
	DWORD i, size, result;
	DWORD bytesreturned;
	LPINTERFACE_INFO interfaceArray;
	DWORD interfaceCount;
	LPVOID buffer;

	/* Initialise Winsock */
	result = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(result != 0)
	{
		printf("Error at WSAStartup(): %d\n", result);
		WSACleanup();
		return;
	}
	
	/* Create a socket */
	s = WSASocket( AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, 0 );
	if( s == INVALID_SOCKET )
	{
		printf( "WSASocketFailed %d\n", WSAGetLastError());
		return;
	}

	/* Calculate size and allocate memory */
#define MAX_INTERFACES 64
	size = sizeof(INTERFACE_INFO) * MAX_INTERFACES;
	buffer = malloc( size );
	if (buffer == NULL)
	{
		printf( "malloc failed\n" );
		return;
	}

	/* Get the IPv4 address list */
	result = WSAIoctl( s, SIO_GET_INTERFACE_LIST, NULL, 0, buffer, size, &bytesreturned, NULL, NULL );
	if( result == SOCKET_ERROR)
	{
		printf( "WSAIoctl failed %d\n", WSAGetLastError() );
		return;
	}
	closesocket(s);

	interfaceArray = (INTERFACE_INFO*)buffer;
	interfaceCount = bytesreturned/sizeof(INTERFACE_INFO);

	for(i = 0; i < interfaceCount; i++)
	{
		// ignore interfaces that are currently down
		if (! ( interfaceArray[i].iiFlags & IFF_UP ) )
			continue;

		// ignore the loopback interface
		if ( interfaceArray[i].iiFlags &  IFF_LOOPBACK )
			continue;

		IN_ADDR addr = interfaceArray[i].iiAddress.AddressIn.sin_addr;
		IN_ADDR bcast = interfaceArray[i].iiBroadcastAddress.AddressIn.sin_addr;
		IN_ADDR netmask = interfaceArray[i].iiNetmask.AddressIn.sin_addr;

		IN_ADDR mcast;
		mcast.s_addr = (addr.s_addr & netmask.s_addr) | (bcast.s_addr & ~netmask.s_addr);

		strcpy_s(pAddrString, 64, inet_ntoa( mcast ));
	}

	/* cleanup */
	WSACleanup();

	free(buffer);
	
    return;
}
