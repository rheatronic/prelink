/*
 Prelink (C) 2010 by bitflix.  All rights reserved.
*/

/**	\file	prelink_hardware.cxx
*	PrelinkHardware class, a network output to Previzion.
*/

//--- Class declaration
#include "prelink_hardware.h"

#if defined( KARCH_ENV_WIN )
#include <winsock2.h>
#endif

#if defined( KARCH_ENV_UNIX )
#include <netinet/in.h>
#endif

#include "prelink_adapters.h"


/************************************************
 *	Constructor.
 ************************************************/
PrelinkHardware::PrelinkHardware()
{
	char lSuggestedMcastAddr[64];

	lSuggestedMcastAddr[0] = '\0';
	suggestMcastAddr(lSuggestedMcastAddr);

	mNetworkAddress		= lSuggestedMcastAddr;
	mNetworkPort		= DEFAULT_PORT;
}


/************************************************
 *	Destructor.
 ************************************************/
PrelinkHardware::~PrelinkHardware()
{
}


/************************************************
 *	Open device communications.
 ************************************************/
bool PrelinkHardware::Open()
{
	if( !mTCPIP.CreateSocket( mNetworkSocket, SOCK_STREAM ))
	{
		char msg[128];
		sprintf_s(msg, 128, "CreateSocket failed");
		mLastError = msg;
		FBTrace("%s\n", msg);
		return false;
	}

	if( !mTCPIP.Connect( mNetworkSocket, mNetworkAddress, mNetworkPort ) )
	{
		char msg[128];
		sprintf_s(msg, 128, "Connect failed");
		mLastError = msg;
		FBTrace("%s\n", msg);
		mTCPIP.CloseSocket( mNetworkSocket );
		return false;
	}

	mPacketDeque.clear();

	FBTrace("Open succeeded\n");
	return true;
}


/************************************************
 *	Get device setup information.
 ************************************************/
bool PrelinkHardware::GetSetupInfo()
{
	// TODO: Get hardware information
	return true;
}


/************************************************
 *	Close device communications.
 ************************************************/
bool PrelinkHardware::Close()
{
	FBTrace("Closed\n");
	mTCPIP.CloseSocket(mNetworkSocket);
	return true;
}


/************************************************
 *	Fetch a data packet from the device.
 ************************************************/
bool PrelinkHardware::FetchDataPacket(FBTime &pTime)
{
	//Returns true if a new data packet is ready
	// TODO: insert time stamp from data packet

	// TODO: Replace this bogus code with real NON-BLOCKING TCP, UDP or serial calls
	// to your data server.
	PollData();

	return true; // data for all elements have been found.

	// return false; // incomplete data packet has been read.
}


/************************************************
 *	Poll device.
 *	Device should get itself to send more data.
 ************************************************/
bool PrelinkHardware::PollData()
{
	return true;
}


/************************************************
 *	Send a data packet over the network to update Previzion.
 ************************************************/
bool PrelinkHardware::SendDataPacket(FBTime &pTime)
{
	// Send to hardware
	// High priority IO thread, must have minimal CPU usage.
	// Non-blocking IO, send data to network.

	while(!mPacketDeque.empty())
	{
		std::string lXML = mPacketDeque.front();
		int lBytesWritten;
		mTCPIP.Write(mNetworkSocket, lXML.c_str(), (int)lXML.size(), lBytesWritten);
		if ((int)lXML.size() != lBytesWritten) 
		{
			//FBTrace("No send: Packet size %d wrote %d\n", (int)lXML.size(), lBytesWritten);
			return false;
		}

		mPacketDeque.pop_front();
	}

	return true;
}


/************************************************
 *	Write the new position values into the hardware abstraction.
 ************************************************/
void PrelinkHardware::EnqueuePacket( Packet &pPacket )
{
	std::string lXML = pPacket.result();
	mPacketDeque.push_back(lXML);
}
