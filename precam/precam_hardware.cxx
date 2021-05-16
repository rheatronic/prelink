//--- Class declaration
#include "precam_hardware.h"
#include <math.h>
#include "previzion_status_stream.h"

#if defined( KARCH_ENV_WIN )
#include <winsock2.h>
#endif

#if defined( KARCH_ENV_UNIX )
#include <netinet/in.h>
#endif

/************************************************
 *	Constructor.
 ************************************************/
Precam_Hardware::Precam_Hardware()
{
	mNetworkAddress		= DEFAULT_HOST;
	mNetworkPort		= DEFAULT_PORT;

	mCounter			= 0;

	mLastError          = NULL;
}

/************************************************
 *	Destructor.
 ************************************************/
Precam_Hardware::~Precam_Hardware()
{
}

/************************************************
 *	Open device communications.
 ************************************************/
bool Precam_Hardware::Init()
{
	return true;
}

/************************************************
 *	Open device communications.
 ************************************************/
bool Precam_Hardware::Open()
{
	bool lSuccess = false;

	if( !mTCPIP.CreateSocket( mNetworkSocket, SOCK_STREAM ))
	{
		char msg[128];
		sprintf(msg, "CreateSocket failed");
		mLastError = msg;
		FBTrace("%s\n", msg);
		return false;
	}

	if( !mTCPIP.Connect( mNetworkSocket, mNetworkAddress, mNetworkPort ))
	{
		char msg[128];
		sprintf(msg, "Connect failed");
		mLastError = msg;
		FBTrace("%s\n", msg);
		return false;
	}

	// any errors ready on the socket?
	bool ready = mTCPIP.Select( mNetworkSocket, false, false, true, 1000 );
	if( !ready )
	{
		lSuccess = true;
	}
	else
	{
		FBTrace("Socket select found error\n");
		mTCPIP.CloseSocket( mNetworkSocket );
	}

	return lSuccess;
}

/************************************************
 *	Get device setup information.
 ************************************************/
bool Precam_Hardware::GetSetupInfo()
{
	// Could get service info (e.g. version of Previzion and protocol in use)
	return true;
}

/************************************************
 *	Start device data stream.
 ************************************************/
bool Precam_Hardware::StartDataStream()
{
	mCounter = 0;
	mStatusStream.Reset();

	FBString lBuffer;
	mStatusStream.SetMode(lBuffer, true, "camera");

	int lpNumberOfBytesWritten = 0;
	return mTCPIP.Write(mNetworkSocket, lBuffer, lBuffer.GetLen(), lpNumberOfBytesWritten);
}

/************************************************
 *	Fetch a data packet from the device.
 ************************************************/
bool Precam_Hardware::FetchDataPacket()
{
	if( !mTCPIP.Select(mNetworkSocket, true, false, false) )
		return false;

	bool lSuccess = false;

	const int lpBufferSize = 2048;
	char lpBuffer[lpBufferSize];
	int lpNumberOfBytesRead;
	if(!mTCPIP.Read(mNetworkSocket, lpBuffer, lpBufferSize-1, lpNumberOfBytesRead ))
		return false;

	lpBuffer[lpNumberOfBytesRead] = '\0';

	StatusRecord *lRecord = mStatusStream.ParseBuffer(lpBuffer, lpNumberOfBytesRead);
	if( lRecord /* and type is camera status */ )
	{
		mStatus = lRecord->mStatus[0];
		delete lRecord;

		mCounter++;
		lSuccess = true;
	}

	return lSuccess;
}

/************************************************
 *	Stop device data stream.
 ************************************************/
bool Precam_Hardware::StopDataStream()
{
	FBString lBuffer;
	mStatusStream.SetMode(lBuffer, false, "camera");

	int lpNumberOfBytesWritten = 0;
	mTCPIP.Write(mNetworkSocket, lBuffer, lBuffer.GetLen(), lpNumberOfBytesWritten);

	return true;
}

/************************************************
 *	Close device communications.
 ************************************************/
bool Precam_Hardware::Close()
{
	mTCPIP.CloseSocket( mNetworkSocket );
	return true;
}

/************************************************
 *	Shut down hardware.
 ************************************************/
bool Precam_Hardware::Done()
{
	return true;
}

/************************************************
 *	Record query.
 ************************************************/
FBTime Precam_Hardware::GetTime()
{
	// As of Mobu 2012 FBTime's kFBTimeModeCustom option does not work (jumpy values, accuracy issue?)
	// Calculating directly in milliseconds also does not work
	// Using FBTimeCode to get the time also does not work
	// Possibly a loss of precision bug in the FB code.
	FBTime lTime(mStatus.hour, mStatus.minute, mStatus.second, mStatus.frame);
	return lTime;
}

double Precam_Hardware::GetTCRate()
{
	return mStatus.rate;
}

FBVector3d Precam_Hardware::GetRotation()
{
	FBVector3d lRotation(mStatus.rotation);
	return lRotation;
}

FBVector3d Precam_Hardware::GetPosition()
{ 
	FBVector3d lPosition(mStatus.position);
	return lPosition;
}

double Precam_Hardware::GetFocus()
{
	return mStatus.focus;
}

double Precam_Hardware::GetZoom()
{
	return mStatus.zoom;
}

FBVector2d Precam_Hardware::GetFieldOfView()
{
	FBVector2d lFOV(mStatus.fov);
	return lFOV;
}

double Precam_Hardware::GetSubjectDistance()
{
	return mStatus.subject;
}

double Precam_Hardware::GetDistortionR2()
{
	return mStatus.distortion;
}

double Precam_Hardware::GetIris()
{
	return mStatus.iris;
}
