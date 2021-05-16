// client.cpp : Receive packets.
//

#include "previzion_packet.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
	int retval;

    // Request Winsock version 2.2
    if ((retval = WSAStartup(0x202, &wsaData)) != 0)
    {
        fprintf(stderr,"WSAStartup() failed with error %d\n", retval);
        WSACleanup();
        return -1;
    }
 
	SOCKET cs;
	cs = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (cs == INVALID_SOCKET){
        fprintf(stderr,"socket() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

	struct sockaddr_in catcher;
	memset((void *)&catcher, '\0', sizeof(struct sockaddr_in));
    catcher.sin_family = AF_INET;
    catcher.sin_addr.s_addr = htonl(INADDR_ANY);
    catcher.sin_port = htons(DEFAULT_PORT);

	if (bind(cs, (SOCKADDR *) &catcher, sizeof(catcher)) == SOCKET_ERROR)
	{
		fprintf(stderr,"bind() failed: error %d\n", WSAGetLastError());
		closesocket(cs);
		WSACleanup();
		exit(0);
	}

    // Receive until the peer closes the connection
    do {
		Packet packet;
		retval = recv(cs, packet.bytes, sizeof(packet.bytes), 0);
        if (retval == SOCKET_ERROR)
        {
			int errcode = WSAGetLastError();
			if( errcode == 10040 )
			{
				fprintf(stderr, "Packet too long\n");
				retval = 1;
				continue;
			}
            fprintf(stderr,"recvfrom() failed: error %d\n", errcode);
            closesocket(cs);
            continue;
        }
        if (retval == 0)
        {
            printf("server closed connection.\n");
            break;
        }
		if (retval != FRAME_SIZE) {
			fprintf(stderr, "Bad length %d\n", retval);
		}
		if (packet.frame.header != FRAME_HEADER) {
			fprintf(stderr, "Bad header %x\n", packet.frame.header);
		}
		printf("%02d:%02d:%02d:%02d\n", packet.frame.hour, packet.frame.minute, packet.frame.second, packet.frame.frame);
    } while( retval > 0 );

    // cleanup
    closesocket(cs);
    WSACleanup();

    return 0;
}
