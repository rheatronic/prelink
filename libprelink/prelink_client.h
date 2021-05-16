#ifndef __PRELINK_CLIENT_H__
#define __PRELINK_CLIENT_H__
/*
 Prelink (C) 2011 by bitflix.  All rights reserved.
*/

/**	\file	prelink_client.h
*	PrelinkClient, state transfer to previzion.
*/

#include "bitflix/bitflix_debug.h"

//--- Class declarations
#include "bitflix/bitflix_tcpip.h"
#include "bitflix/bitflix_time.h"
#include "bitflix/bitflix_math.h"

#include "previzion_status_stream.h"

#include <vector>
#include <string>

#define DllExport   __declspec( dllexport )

// Wraps a snapshot of one scene graph node's state
// in a format native to the application.
// Accessors do conversion (name and matrix)
// later (e.g. in a worker thread) but again *from a snapshot* 
// in a format native to the application.
// Accessors throws TransformException if a problem occurs.
class DllExport TransformState 
{
public:
	virtual void precheck() {}	// optional: throw exception if problems are detected (no further processing)
	virtual void getName(std::string &name) = 0;
	virtual void getVisibility(bool &visibility) = 0;
	virtual void getMatrix(BFMatrix44 &matrix) = 0;
};

// An exception during processing of the scene graph node snapshot
class DllExport TransformException {
};

#pragma warning( disable: 4251 )

// A group or frame of transform states at a given time.
struct DllExport FramedState 
{
	FramedState() : mHasTime(false), mTime(0, 0, 0, 0, 0.0) {}
	FramedState(BFTimecode &tc) : mHasTime(true), mTime(tc) {}
	bool mHasTime;
	BFTimecode mTime;
	std::vector<TransformState*> mStates;
};

#define DEFAULT_PRELINK_ADDRESS "127.0.0.1"
#define DEFAULT_PRELINK_PORT 4444

// Client interface to a prelink server
// Blocking operations (should be in thread)
class DllExport PrelinkClient
{
public:
	enum State {
		kUninitialized,
		kOpen,
		kClosed,
		kError,
		kLast
	};

	//
	PrelinkClient();
	~PrelinkClient();

	// Communication configuration
	void		SetNetworkAddress	(const char *pIPAddress){ mNetworkAddress =  pIPAddress;	}
	const char *GetNetworkAddress	()						{ return mNetworkAddress.c_str();	}
	void		SetNetworkPort		(unsigned short pPort)	{ mNetworkPort = pPort;				}
	int			GetNetworkPort		()						{ return mNetworkPort;				}

	// Opens and closes connection with data server. returns true if successful
	bool		Open();
	bool		IsConnected();
	bool		Close();

	// Following two methods should be safe to call (simultaneously) in separate threads

	// Send one frame's worth of transform states
	bool		Send				(FramedState *pState);

	// Receive status frames
	bool		Receive				();

private:
	BFTCPIP			mTCPIP;					//!< TCPIP interface.
	SOCKET			mNetworkSocket;			//!< Network socket.
	std::string		mNetworkAddress;		//!< Network address.
	unsigned short	mNetworkPort;			//!< Network port number.
	State			mState;
	
	StatusStream	mStatusStream;
	int				mCounter;
	CameraStatus	mStatus;
};

#endif /* __PRELINK_CLIENT_H__ */
