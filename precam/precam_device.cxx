//--- Class declaration
#include "precam_device.h"
#include "bitflix/mobu_time.h"
#include <math.h>

//--- Registration defines
#define PRECAM__CLASS		PRECAM__CLASSNAME
#define PRECAM__NAME		PRECAM__CLASSSTR
#define PRECAM__LABEL		"Precam"
#define PRECAM__DESC		"Precam - Previzion Streaming Camera device"
#define PRECAM__PREFIX		"Precam"
#define PRECAM__ICON		"devices_netclient.png"

#define PRECAM__HARDWARE_VERSION_INFO	"Precam 0.8.0"

//--- FiLMBOX implementation and registration
FBDeviceImplementation	(	PRECAM__CLASS	);
FBRegisterDevice		(	PRECAM__NAME,
							PRECAM__CLASS,
							PRECAM__LABEL,
							PRECAM__DESC,
							PRECAM__ICON		);

#include <fstream>

#ifdef LOG_TIMING
std::fstream dcann, dced;
#endif

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool Precam::FBCreate()
{
	ModelTemplatePrefix = PRECAM__PREFIX;
	IconFilename		= PRECAM__ICON;
	PacketBufferSize	= 3;
	CommType			= kFBCommTypeNetworkTCP;
	SyncDelay			= 1;
	mFirstPacket		= true;

	//dcann.open("timing_dcann.log", std::ios_base::out);
	//dced.open("timing_dced.log", std::ios_base::out);

	if( FBDeviceCamera::FBCreate() )
	{
		// Raw channels from stream
		mNodeCamera1_SubjectDistance = AnimationNodeOutCreate( 0, "Camera 1 Subject Distance",	ANIMATIONNODE_TYPE_NUMBER );
		mNodeCamera1_DistortionR2 = AnimationNodeOutCreate( 1, "Camera 1 Distortion R2",	ANIMATIONNODE_TYPE_NUMBER );
		mNodeCamera1_Iris = AnimationNodeOutCreate( 2, "Camera 1 Iris",	ANIMATIONNODE_TYPE_NUMBER );
		mNodeCamera1_TC_Hour = AnimationNodeOutCreate( 3, "Camera 1 TC Hour",	ANIMATIONNODE_TYPE_NUMBER );
		mNodeCamera1_TC_Minute = AnimationNodeOutCreate( 4, "Camera 1 TC Minute",	ANIMATIONNODE_TYPE_NUMBER );
		mNodeCamera1_TC_Second = AnimationNodeOutCreate( 5, "Camera 1 TC Second",	ANIMATIONNODE_TYPE_NUMBER );
		mNodeCamera1_TC_Frame = AnimationNodeOutCreate( 6, "Camera 1 TC Frame",	ANIMATIONNODE_TYPE_NUMBER );
		mNodeCamera1_TC_Rate = AnimationNodeOutCreate( 7, "Camera 1 TC Rate",	ANIMATIONNODE_TYPE_NUMBER );

		FBTime	lPeriod;
		lPeriod.SetMilliSeconds(1000.0 / ( ( 1000.0 * 60.0 ) / 1001.0 ) );	// 60 fps video (59.94 fps) frame time in ms
		SamplingPeriod	= lPeriod;
		SamplingMode	= kFBHardwareTimestamp;

		BuildQueue();

		return true;
	}
	return false;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void Precam::FBDestroy()
{
	//dcann.close();
	//dced.close();

	DeleteQueue();
	FBDeviceCamera::FBDestroy();
}

void Precam::BuildQueue()
{
	// Register packets in packetbuffer
	for( int i=0; i<PacketBufferSize; i++ )
	{
		FBDeviceSync::PacketRegister( new PrecamPacket );
	}
}

void Precam::DeleteQueue()
{
	while( FBDeviceSync::PacketGetCount() > 0 )
	{
		PrecamPacket* lPacket = (PrecamPacket*)FBDeviceSync::PacketRemove(0);
		delete lPacket;
	}
}

void Precam::ResetQueue()
{
	DeleteQueue();
	BuildQueue();
}

/************************************************
 *	Device operation.
 ************************************************/
bool Precam::DeviceOperation( kDeviceOperations pOperation )
{
	switch (pOperation)
	{
		case kOpInit:	return Init();
		case kOpStart:	return Start();
		case kOpStop:	return Stop();
		case kOpReset:	Stop(); return Start();
		case kOpDone:	return Done();
	}
	return FBDeviceCamera::DeviceOperation( pOperation );
}

/************************************************
 *	Initialization of device.
 ************************************************/
bool Precam::Init()
{
    FBProgress	lProgress;
    lProgress.Caption	= "Initializing Device";

	// Step 1: Init device communications
	lProgress.Text		= "Initializing device...";
	Status				= "Initializing device...";
	if(!mHardware.Init())
	{
		lProgress.Text		= "Could not init device!";
		Status				= "Could not init device!";
		return false;
	}

	lProgress.Text		= "Initialized";
	HardwareVersionInfo = PRECAM__HARDWARE_VERSION_INFO;
	Information			= "";
	Status				= "Initialized";

	return true;
}

/************************************************
 *	Device is put online.
 ************************************************/
bool Precam::Start()
{
	FBProgress	lProgress;
	lProgress.Caption	= "Device Startup";

	// Step 1: Open device communications
	lProgress.Text	= "Opening device communications...";
	Status			= "Opening device communications...";
	if(!mHardware.Open())
	{
		lProgress.Text	= "Opening device communications";
		Information		= mHardware.GetLastError();
		Status			= "Could not open device.";
		return false;
	}

	// Step 2: Get device information
	lProgress.Text		= "Getting Setup Info...";
	Status				= "Getting Setup Info...";
	if(!mHardware.GetSetupInfo())
	{
		lProgress.Text		= "Could not get setup info!";
		Information			= mHardware.GetLastError();
		Status				= "Could not get setup info!";
		return false;
	}

	// Step 3: Start data stream.
	lProgress.Text		= "Starting Streaming Mode...";
	Status				= "Starting Streaming Mode...";
	if(!mHardware.StartDataStream())
	{
		lProgress.Text		= "Could not start streaming!";
		Information			= mHardware.GetLastError();
		Status				= "Could not start streaming!";
		return false;
	}

	lProgress.Text		= "Device started.";
	HardwareVersionInfo = PRECAM__HARDWARE_VERSION_INFO;
	Information			= "";
	Status				= "Started";
	return true;
}

/************************************************
 *	Device is stopped (offline).
 ************************************************/
bool Precam::Stop()
{
	FBProgress	lProgress;
	lProgress.Caption	= "Device Shutdown";

	// Step 1: Stop streaming data
	lProgress.Text	= "Stopping device streaming";
	Information		= "";
	Status			= "Stopping device streaming";
	if(!mHardware.StopDataStream())
	{
		Status = "Could not stop streaming";
		return false;
	}

	// Step 2: Close communications
	lProgress.Text	= "Stopping device communications";
	Information		= "";
	Status			= "Stopping device communications";
	if(!mHardware.Close())
	{
		Information	= mHardware.GetLastError();
		Status		= "Could not close device";
		return false;
	}

	HardwareVersionInfo = PRECAM__HARDWARE_VERSION_INFO;
	Information = "";
	Status = "Stopped";
    return false;
}

/************************************************
 *	Removal of device.
 ************************************************/
bool Precam::Done()
{
	FBProgress lProgress;
	lProgress.Caption = "Device Removal";

	// Step 1: Stop hardware
	lProgress.Text	= "Stopping device...";
	Information		= "";
	Status			= "Stopping device...";
	if(!mHardware.Close())
	{
		lProgress.Text	= "Could not stop device!";
		Information		= mHardware.GetLastError();
		Status			= "Could not stop device!";
		return true;
	}

	HardwareVersionInfo = PRECAM__HARDWARE_VERSION_INFO;
	Information = "";
	Status = "Done";
	return false;
}

/************************************************
 *	Real-Time Engine Evaluation. This would get called once for each camera, with a different pDstPacket appearing?
 ************************************************/
bool Precam::DeviceCameraAnimationNodeNotify( HFBEvaluateInfo pEvaluateInfo, HFBDeviceCameraPacket pDstPacket, HFBDeviceSyncPacket pSrcPacket )
{
#ifdef LOG_TIMING
	int syncCount = pEvaluateInfo->GetSyncCount();
	FBTime systemStart = pEvaluateInfo->GetSystemTime();
	
	dcann << systemStart.GetMilliSeconds() << " " << syncCount;
#endif

	// Parent call, transfer all data for the cameras because we don't use custom cameras
	FBDeviceCamera::DeviceCameraAnimationNodeNotify( pEvaluateInfo, pDstPacket, pSrcPacket );

	// Continue for custom data, to be sent to the extension channels on the device.
	//PrecamPacket* lPacket = (PrecamPacket*) FBDeviceSync::PacketFetch( pCorrectedSync );
	PrecamPacket* lPacket = (PrecamPacket*) pSrcPacket;
	if( lPacket )
	{
		mNodeCamera1_SubjectDistance->WriteData( &(lPacket->SubjectDistance), pEvaluateInfo );
		mNodeCamera1_DistortionR2->WriteData(&(lPacket->DistortionR2), pEvaluateInfo );
		mNodeCamera1_Iris->WriteData(&(lPacket->Iris), pEvaluateInfo );

		FBTime fbt = lPacket->Time;
#if ORSDK2012
		kLongLong lLLHour, lLLMinute, lLLSecond, lLLFrame, lLLField, lLLMSec;
		fbt.GetTime(lLLHour, lLLMinute, lLLSecond, lLLFrame, lLLField, lLLMSec, kFBTimeModeCustom, lPacket->TCRate);
#else
		int lLLHour, lLLMinute, lLLSecond, lLLFrame, lLLField, lLLMSec;
		fbt.GetTime(lLLHour, lLLMinute, lLLSecond, lLLFrame, lLLField, lLLMSec, timeModeFromRate(lPacket->TCRate));
#endif
		double lDHour = (double)lLLHour, lDMinute = (double)lLLMinute, lDSecond = (double)lLLSecond, lDFrame = (double)lLLFrame;
		mNodeCamera1_TC_Hour->WriteData(&lDHour, pEvaluateInfo);
		mNodeCamera1_TC_Minute->WriteData(&lDMinute, pEvaluateInfo);
		mNodeCamera1_TC_Second->WriteData(&lDSecond, pEvaluateInfo);
		mNodeCamera1_TC_Frame->WriteData(&lDFrame, pEvaluateInfo);
		mNodeCamera1_TC_Rate->WriteData(&(lPacket->TCRate), pEvaluateInfo);
		
#ifdef LOG_TIMING
		dcann << " " << (char*)(fbt.GetTimeString());
#endif
	}
#ifdef LOG_TIMING
	else 
		dcann << " NP";
	
	dcann << std::endl;
#endif

	//FBDeviceSync::PacketRelease( lPacket );

    return true;
}

static const char* ACTION_NAMES[] = 
{
	"SR", "PR", "SW", "PW", 0
};

/************************************************
 *	Camera hardware IO (isolates read calls from DeviceIONotify)
 ************************************************/
void Precam::DeviceCameraEvalData( FBDeviceNotifyInfo &pDeviceNotifyInfo )
{
	int		lSyncCount		= pDeviceNotifyInfo.GetSyncCount();
	int		lPacketCount	= 0;
	FBTime	lRecordTime;

#ifdef LOG_TIMING
	dced << pDeviceNotifyInfo.GetSystemTime().GetMilliSeconds() << " " << lSyncCount;
#endif

	// Lock a packet
	PrecamPacket* lPacket = (PrecamPacket*)FBDeviceSync::PacketLock();
	{
		if (!lPacket)
		{
			Status = "Could not lock packet";
			FBTrace("Could not lock packet\n");
			ResetQueue();
		}
		while( lPacket && mHardware.FetchDataPacket() )
		{
			// Debug info
			//PrintDebugIOInfo( lPacket, pDeviceNotifyInfo );

			// Store packet
			lPacket->OriginalSync	= lSyncCount;
			lPacket->DeviceSync		= mHardware.GetSync();
			lPacket->Position		= mHardware.GetPosition();
			lPacket->Rotation		= mHardware.GetRotation();
			lPacket->Focus			= mHardware.GetFocus();
			lPacket->Zoom			= mHardware.GetZoom();
			lPacket->FieldOfView	= mHardware.GetFieldOfView();
			lPacket->SubjectDistance = mHardware.GetSubjectDistance();
			lPacket->DistortionR2	= mHardware.GetDistortionR2();
			lPacket->Iris			= mHardware.GetIris();
			lPacket->Time			= mHardware.GetTime();
			lPacket->TCRate			= mHardware.GetTCRate();

#ifdef LOG_TIMING
			dced << " " << (char*)(mHardware.GetTime().GetTimeString());
#endif

            //
            // Set instrument packet in order to create the fcurve keys when recording
            //
            PrecamPacket* lRecPacket = (PrecamPacket*) FBDeviceSync::PacketFetch( lSyncCount-SyncDelay );            
	        if( lRecPacket )
            {
				// Establish the FiLMBOX recording time
			    if( mNodeCamera1_SubjectDistance->GetAnimationToRecord() )
			    {
					if( mFirstPacket )
				    {
					    mFirstPacket		= false;
					    mRecordStartTime	= pDeviceNotifyInfo.GetLocalTime();
					    mFirstPacketTime	= lRecPacket->Time;
				    }
			    }

			    // Copy the generic stuff into the instrument's RecordPacket, used by parent class DeviceRecordFrame
			    FBDeviceCameraInstrument*	lInstrument = (FBDeviceCameraInstrument*)Instruments[0];
			    HFBDeviceCameraPacket		lIPacket	= lInstrument->RecordPacket;

			    lIPacket->Position		= FBVector3d( lRecPacket->Position );
			    lIPacket->Rotation		= FBVector3d( lRecPacket->Rotation );
			    lIPacket->FieldOfView	= FBVector2d( lRecPacket->FieldOfView );
			    lIPacket->OpticalCenter	= FBVector2d( lRecPacket->OpticalCenter );
			    lIPacket->Focus			= (double)lRecPacket->Focus;
			    lIPacket->Zoom			= (double)lRecPacket->Zoom;

				// Copy the extended stuff into our extended packet
				PrecamPacket*				lPPacket	= &RecordPacket;
			    lPPacket->Position		= FBVector3d( lRecPacket->Position );
			    lPPacket->Rotation		= FBVector3d( lRecPacket->Rotation );
			    lPPacket->FieldOfView	= FBVector2d( lRecPacket->FieldOfView );
			    lPPacket->OpticalCenter	= FBVector2d( lRecPacket->OpticalCenter );
			    lPPacket->Focus			= (double)lRecPacket->Focus;
			    lPPacket->Zoom			= (double)lRecPacket->Zoom;
				lPPacket->SubjectDistance = (double)lRecPacket->SubjectDistance;
				lPPacket->DistortionR2  = (double)lRecPacket->DistortionR2;
				lPPacket->Iris			= (double)lRecPacket->Iris;
				lPPacket->TCRate		= lRecPacket->TCRate;

				FBTime lRecPacketTime = lRecPacket->Time;
                lRecordTime = (lRecPacketTime - mFirstPacketTime) + mRecordStartTime;
				
				// this is calling below
                DeviceCameraRecordFrame( lRecordTime , pDeviceNotifyInfo, lRecPacket );
                FBDeviceSync::PacketRelease( lRecPacket );
            }
#ifdef LOG_TIMING
			else
				dced << " NRP";
#endif

			AckOneSampleReceived();

			// Cleanup & prep for next packet
			lPacketCount++;
			FBDeviceSync::PacketUnlock( lPacket, true );
			lPacket = (PrecamPacket*)FBDeviceSync::PacketLock();
		}
	}
	// If a packet is still locked, free packet
	if( lPacket )
	{
		FBDeviceSync::PacketUnlock( lPacket, false );
	}

#ifdef LOG_TIMING
	dced << std::endl;
#endif

	//PrintDebugIONotifyInfo( pDeviceNotifyInfo, lPacketCount );
}

void Precam::DeviceCameraRecordFrame( FBTime &pTime, FBDeviceNotifyInfo& pNotifyInfo, PrecamPacket* pPacket )
{
	// Parent call, note this WILL read from the RecordPacket on the Instrument
	FBDeviceCamera::DeviceCameraRecordFrame( pTime, pNotifyInfo );

	// Continue with custom data, note this must use the (extended, custom) RecordPacket on this device
	if( mPlayerControl.GetTransportMode() == kFBTransportPlay )
	{
		HFBAnimationNode node;

		// subject
		node = mNodeCamera1_SubjectDistance->GetAnimationToRecord();
		if (node) node->KeyAdd(pTime, &(pPacket->SubjectDistance));

		// distortion r2
		node = mNodeCamera1_DistortionR2->GetAnimationToRecord();
		if (node) node->KeyAdd(pTime, &(pPacket->DistortionR2));

		// iris
		node = mNodeCamera1_Iris->GetAnimationToRecord();
		if (node) node->KeyAdd(pTime, &(pPacket->Iris));

		// timecode
		FBTime fbt = pPacket->Time;
#ifdef ORSDK2012
		kLongLong lLLHour, lLLMinute, lLLSecond, lLLFrame, lLLField, lLLMSec;
		fbt.GetTime(lLLHour, lLLMinute, lLLSecond, lLLFrame, lLLField, lLLMSec, kFBTimeModeCustom, pPacket->TCRate);
#else
		int lLLHour, lLLMinute, lLLSecond, lLLFrame, lLLField, lLLMSec;
		fbt.GetTime(lLLHour, lLLMinute, lLLSecond, lLLFrame, lLLField, lLLMSec, timeModeFromRate(pPacket->TCRate));
#endif
		double lDHour = (double)lLLHour, lDMinute = (double)lLLMinute, lDSecond = (double)lLLSecond, lDFrame = (double)lLLFrame;

		node = mNodeCamera1_TC_Hour->GetAnimationToRecord();
		if (node) node->KeyAdd(pTime, &lDHour);

		node = mNodeCamera1_TC_Minute->GetAnimationToRecord();
		if (node) node->KeyAdd(pTime, &lDMinute);

		node = mNodeCamera1_TC_Second->GetAnimationToRecord();
		if (node) node->KeyAdd(pTime, &lDSecond);

		node = mNodeCamera1_TC_Frame->GetAnimationToRecord();
		if (node) node->KeyAdd(pTime, &lDFrame);

		node = mNodeCamera1_TC_Rate->GetAnimationToRecord();
		if (node) node->KeyAdd(pTime, &(pPacket->TCRate));
	}
}

// Called once for each animation node on the device
void Precam::RecordingDoneAnimation( HFBAnimationNode pAnimationNode )
{
	// Parent call
	FBDeviceCamera::RecordingDoneAnimation( pAnimationNode );

	mFirstPacket = true;
}

#define GETSTAT(NAME) bool stat##NAME = GetObjectStatus(kFBStatus##NAME)

/************************************************
 *	Add new model binding.
 ************************************************/
bool Precam::ModelTemplateBindNotify( HFBModel pModel, int pIndex, HFBModelTemplate pModelTemplate )
{
	static int lOnMyWayOut = 0;
	if( pModel )
	{
		// GetObjectStatus(kFBStatusNone) may have been meant to ignore init when device itself has any non-default status
		// which may mean this should be !GetObjectStatus(kFBStatusNone) && ...
		// however it appears that StatusNone is not set by default
		// the net effect is therefore never to run the code

		if( GetObjectStatus(kFBStatusNone) && pModel->Is( FBCamera::TypeInfo ) )
		{
			HFBCamera	lCamera = (HFBCamera) pModel;
			lCamera->ResolutionMode		= kFBResolutionHD;
		}
	}
	// Recursive propagation to children.
	for ( int Count=0; Count<pModel->Children.GetCount(); Count++ )
	{
		lOnMyWayOut++;
		Precam::ModelTemplateBindNotify( pModel->Children[Count], pIndex, pModelTemplate );
	}

	if( lOnMyWayOut==0 )
	{
		FBDeviceCamera::ModelTemplateBindNotify( pModel, pIndex, pModelTemplate );
	}
	else
	{
		lOnMyWayOut--;
	}
	return true;
}

#define PRECAM_VERSION_491				0x0491
//#define RADAMEC_VERSION_400			0x0400
#define CAMERATEMPLATE_VERSION_NUMBER	PRECAM_VERSION_491

#define CAMERATEMPLATE_FBX_VERSION		"Version"
#define CAMERATEMPLATE_FBX_NETWORK		"Network"

/************************************************
 *	Store data in FBX.
 ************************************************/
bool Precam::FbxStore(HFBFbxObject pFbxObject,kFbxObjectStore pStoreWhat)
{
	if (pStoreWhat & kAttributes)
	{
		pFbxObject->FieldWriteI(CAMERATEMPLATE_FBX_VERSION, CAMERATEMPLATE_VERSION_NUMBER);
		pFbxObject->FieldWriteBegin(CAMERATEMPLATE_FBX_NETWORK);
		{
			pFbxObject->FieldWriteC( this->GetNetworkAddress()	);
			pFbxObject->FieldWriteI( this->GetNetworkPort()		);
		}
		pFbxObject->FieldWriteEnd();
	}
	return FBDeviceCamera::FbxStore( pFbxObject, pStoreWhat );
}

/************************************************
 *	Retrieve data from FBX.
 ************************************************/
bool Precam::FbxRetrieve(HFBFbxObject pFbxObject,kFbxObjectStore pStoreWhat)
{
	int lVersion;
	if (pStoreWhat & kAttributes)
	{
		lVersion = pFbxObject->FieldReadI(CAMERATEMPLATE_FBX_VERSION);
		if( lVersion >= CAMERATEMPLATE_VERSION_NUMBER )
		{
			if(pFbxObject->FieldReadBegin(CAMERATEMPLATE_FBX_NETWORK) )
			{
				this->SetNetworkAddress	( pFbxObject->FieldReadC() );
				this->SetNetworkPort	( pFbxObject->FieldReadI() );
				pFbxObject->FieldReadEnd();
			}
		}
	}
	return FBDeviceCamera::FbxRetrieve( pFbxObject, pStoreWhat );
}
