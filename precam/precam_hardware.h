#ifndef __PRECAM_HARDWARE_H__
#define __PRECAM_HARDWARE_H__

#include "previzion_status_stream.h"

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <fbsdk/fbtracker.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

#include "../bitflix/bitflix_tcpip.h"

#define DEFAULT_HOST "192.168.1.1"
#define DEFAULT_PORT 4444

//! Camera packet class.
class PrecamPacket : public FBDeviceCameraPacket
{
public:
	PrecamPacket() : FBDeviceCameraPacket()
	{
		SubjectDistance = 0.0;
		Iris = 0.0;
		DistortionR2 = 0.0;
		TCRate = 0.0;
	}
	double SubjectDistance, Iris, DistortionR2, TCRate;
};

//! Simple hardware.
class Precam_Hardware
{
public:
	//! Constructor.
	Precam_Hardware();

	//! Destructor.
	virtual ~Precam_Hardware();

	//--- Opens and closes connection with data server. returns true if successful
	bool	Init	();				//!< Initialize hardware.
	bool	Open	();				//!< Open the connection.
	bool	Close	();				//!< Close connection.
	bool	Done	();				//!< Close down hardware.

	//--- Channel information
	//	After the connection has been established with the Open call
	//	This call will retreive all channel information (type, name, etc...)
	bool	GetSetupInfo();						//!< Get the setup information.

	// Streaming functions
	//	Once connection is established and information packets have been received
	//	these functions start and stops the data streaming
	bool	StartDataStream();					//!< Start data streaming.
	bool	StopDataStream();					//!< Stop data streaming.

	//--- Non blocking read of data stream which fills mChannel[] with a new data packet.
	//	Returns true if a new data frame is ready
	bool	FetchDataPacket		();						//!< Fetch a data packet from the computer.

	//--- Communications
	void		SetNetworkAddress	(const char *pIPAddress){ mNetworkAddress =  pIPAddress;	}
	char *		GetNetworkAddress	()						{ return (char *)mNetworkAddress;	}
	void		SetNetworkPort		(int pPort)				{ mNetworkPort = pPort;				}
	int			GetNetworkPort		()						{ return mNetworkPort;				}

	//--- Packet query
	FBTime	    GetTime();
	double		GetTCRate();
	FBVector3d	GetRotation();
	FBVector3d  GetPosition();
	double	    GetFocus();
	double	    GetZoom();
	FBVector2d	GetFieldOfView();
	double		GetSubjectDistance();
	double		GetDistortionR2();
	double		GetIris();
	long	    GetSync()			{ return mCounter;						}

	FBString	GetLastError()		{ return mLastError;					}

private:
	StatusStream	mStatusStream;
	
	FBSystem	mSystem;								//!< System interface.
	int			mCounter;								//!< Counter for sync.

	//--- Communications
	BFTCPIP				mTCPIP;									//!< TCPIP interface.
	SOCKET				mNetworkSocket;							//!< Network socket.
	FBString			mNetworkAddress;						//!< Network address.
	int					mNetworkPort;							//!< Network port number.

	// reading information
	CameraStatus		mStatus;
	HFBDevice			mDevice;

	FBString		mLastError;
};

#endif /* __PRECAM_HARDWARE_H__ */
