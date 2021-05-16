// server.cpp : send packets.
//

#define FPS 30

#include "previzion_packet.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib

class PacketError {
public:
	PacketError(bool enabled, float freq) : enabled(enabled), freq(freq) {}
	bool process(Packet *packet) { if( this->randomf() < this->freq ) { this->inject(packet); return true; } else return false; }
protected:
	virtual void inject(Packet *) = 0;
	int random(int max) { return rand() * max / RAND_MAX; }
	float randomf() { return (float)rand() / (float)RAND_MAX; }
private:
	bool enabled;
	float freq;
};

class ChecksumError : public PacketError {
public:
	ChecksumError(bool enabled, float freq) : PacketError(enabled, freq) {}
protected:
	virtual void inject(Packet *packet) { packet->bytes[random(sizeof(packet->bytes))] = this->random(0xFF); }
};

class ShortError : public PacketError {
public:
	ShortError(bool enabled, float freq) : PacketError(enabled, freq) {}
protected:
	virtual void inject(Packet *packet) { packet->length = FRAME_SIZE - random(FRAME_SIZE-1); }
};

class LongError : public PacketError {
public:
	LongError(bool enabled, float freq) : PacketError(enabled, freq) {}
protected:
	virtual void inject(Packet *packet) { packet->length = FRAME_SIZE + random(FRAME_SIZE-1); }
};

class HeaderError : public PacketError {
public:
	HeaderError(bool enabled, float freq) : PacketError(enabled, freq) {}
protected:
	virtual void inject(Packet *packet) { packet->bytes[0] = random(0xFF); }
};

int __cdecl main(void) 
{
	// checksum errors
	ChecksumError checksumError(true, 0.0f);
	ShortError shortError(true, 0.1f);
	LongError longError(true, 0.1f);
	HeaderError headerError(true, 0.1f);

	// empty packet errors
	// oversize packet errors

    WSADATA wsaData;
	int retval;
	SOCKET sd;
	struct sockaddr_in server;
	char *ip_address= DEFAULT_HOST;

    // Initialize Winsock
    retval = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (retval != 0) {
        printf("WSAStartup failed: %d\n", retval);
        return 1;
    }

	/* Open a datagram socket */
	sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sd == INVALID_SOCKET)
	{
		fprintf(stderr, "Could not create socket.\n");
		WSACleanup();
		exit(0);
	}

	BOOL bBroadcastOpt = TRUE;
	if (setsockopt(sd, SOL_SOCKET, SO_BROADCAST, (char*)&bBroadcastOpt, sizeof(BOOL)) == SOCKET_ERROR)
	{
		fprintf(stderr, "Could not set broadcast option %d\n", WSAGetLastError());
		WSACleanup();
		exit(0);
	}

	memset((void *)&server, '\0', sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = (!ip_address) ? INADDR_ANY : inet_addr(ip_address);
	server.sin_port = htons(DEFAULT_PORT);

	Frame frame;

	frame.header = FRAME_HEADER;
	frame.camera = 1;
	frame.hour = 0;
	frame.minute = 0;
	frame.second = 0;
	frame.frame = 0;
	frame.pan = 0.0;
	frame.tilt = 0.0;
	frame.roll = 0.0;
	frame.x = 0.0;
	frame.y = 0.0;
	frame.z = 0.0;
	frame.focus = 0.0;
	frame.zoom = 0.0;
	frame.opticalCenterX = 0.0;
	frame.opticalCenterY = 0.0;
	frame.status[0] = 0;
	frame.status[1] = 0;
	frame.checksum = 0;

	while(1)
	{
		Packet packet;
		packet.frame = frame;
		packet.set_checksum();
		
		if( checksumError.process(&packet) ) fprintf(stdout, "injected checksum error ");
		if( shortError.process(&packet) ) fprintf(stdout, "injected short packet error ");
		else if( longError.process(&packet) ) fprintf(stdout, "injected long packet error ");
		if( headerError.process(&packet) ) fprintf(stdout, "injected header error ");

		retval = sendto(sd, (char *)packet.bytes, packet.length, 0, (SOCKADDR*)&server, sizeof(server));
		fprintf(stdout, "%02d:%02d:%02d:%02d pan %f\n", frame.hour, frame.minute, frame.second, frame.frame, frame.pan);
		if ( retval != packet.length)
		{
			fprintf(stderr, "Error sending datagram %d\n", WSAGetLastError());
			closesocket(sd);
			WSACleanup();
			exit(0);
		}

		// simulator

		frame.frame++;
		if (frame.frame >= FPS)
		{
			frame.frame = 0;
			frame.second++;
		}
		if (frame.second >= 60)
		{
			frame.second = 0;
			frame.minute++;
		}
		if (frame.minute >= 60)
		{
			frame.minute = 0;
			frame.hour++;
		}

		frame.pan += 0.1f;
		if (frame.pan >= 360.0)
			frame.pan -= 360.0;

		//
		Sleep(1000/FPS);
	}

    return 0;
}
