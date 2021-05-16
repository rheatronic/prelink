#ifndef __PRELINK_HARDWARE_H__
#define __PRELINK_HARDWARE_H__
/*
 Prelink (C) 2010 by bitflix.  All rights reserved.
*/

/**	\file	prelink_hardware.h
*	PrelinkHardware, encapsulates state transfer to previzion.
*/

#include "bitflix/bitflix_debug.h"

//--- Class declarations
#include <deque>
#include "prelink_packet.h"
#include <fbsdk/fbsdk.h>
#include "bitflix/bitflix_tcpip.h"

//! Simple hardware.
class PrelinkHardware
{
public:
	//! Constructor.
	PrelinkHardware();

	//! Destructor.
	~PrelinkHardware();

	//--- Opens and closes connection with data server. returns true if successful
	bool	Open();		//!< Open the device.
	bool	Close();	//!< Close the device.

	/**	Channel information.
	*	After the connection has been established with the Open call
	*	This call will retreive all channel information (type, name, etc...)
	*/
	bool	GetSetupInfo();

	bool	FetchDataPacket	(FBTime &pTime);			//!< Fetch a data packet (not used yet).
	bool	SendDataPacket	(FBTime &pTime);			//!< Send a data packet.
	bool	PollData		();

	// Write transform information
	void	EnqueuePacket( Packet &pPacket );

	// Communications configuration
	void		SetNetworkAddress	(char *pIPAddress)		{ mNetworkAddress =  pIPAddress;	}
	char *		GetNetworkAddress	()						{ return (char *)mNetworkAddress;	}
	void		SetNetworkPort		(int pPort)				{ mNetworkPort = pPort;				}
	int			GetNetworkPort		()						{ return mNetworkPort;				}

private:
	BFTCPIP		mTCPIP;					//!< TCPIP interface.
	SOCKET		mNetworkSocket;			//!< Network socket.
	FBString	mNetworkAddress;		//!< Network address.
	unsigned short	mNetworkPort;		//!< Network port number.

	FBString	mLastError;

	std::deque<std::string>	mPacketDeque;
};

#endif /* __PRELINK_HARDWARE_H__ */
