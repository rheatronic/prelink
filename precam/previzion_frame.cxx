#include "previzion_frame.h"
#include "previzion_packet.h"
#include <fbsdk/fbsdk.h>

#if defined( KARCH_ENV_WIN )
#include <winsock2.h>
#endif

#if defined( KARCH_ENV_UNIX )
#include <netinet/in.h>
#endif

Connection::Connection()
: mTCPIP(0)
{
}

void Connection::attach(FBTCPIP *tcpip)
{
	mTCPIP = tcpip;
}

bool Connection::open(unsigned short port)
{
	mPort = port;
	mTCPIP->CreateSocket(mNetworkSocket, kFBTCPIP_DGRAM);
	mTCPIP->Bind(mNetworkSocket, INADDR_ANY, port);
	if (!mTCPIP->Select(mNetworkSocket, true, false, false, 1000))
	{
		mTCPIP->CloseSocket(mNetworkSocket);
		return false;
	} 
	else 
	{
		return true;
	}
}

void Connection::close()
{
	mTCPIP->CloseSocket(mNetworkSocket);
}

// Provided count buffers for previzion_udp packets
// Returns number actually retrieved
int Connection::get(int count, Frame *frame)
{
	if (count == 0)
		return 0;

	if (mTCPIP->Select(mNetworkSocket, true, false, false, 0))
	{
		Packet packet;
		if (mTCPIP->ReadDatagram(mNetworkSocket, packet.bytes, sizeof(packet.bytes)))
		{
			packet.set(frame);
			return 1;
		}
	}
	return 0;
}

int Connection::put(int count, const Frame *frame)
{
	if (count == 0)
		return 0;

	Packet packet;
	int bytesWritten = 0;
	packet.get(frame);
	mTCPIP->WriteDatagram(mNetworkSocket, packet.bytes, sizeof(packet.bytes), &bytesWritten, INADDR_ANY, mPort);
	if (bytesWritten)
	{
		return 1;
	}
	return 0;
}
