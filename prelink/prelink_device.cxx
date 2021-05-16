/*
 Prelink (C) 2010 by bitflix.  All rights reserved.
*/

/**	\file	prelink_device.cxx
*	Prelink class, manages operational state of the plugin.
*/

#include "bitflix/bitflix_debug.h"

#include <process.h>
#include <time.h>
#include <strstream>
#include <ppl.h>
#include <concurrent_vector.h>
#include <algorithm>

#include "prelink_outer.h"
#include "prelink_frame.h"
#include "prelink_node.h"
#include "prelink_matrix.h"
#include "prelink_properties.h"

//--- Class declaration
#include "prelink_device.h"

#include "prelink_badgroup.h"

//--- Device strings
#define PRELINK__CLASS	PRELINK__CLASSNAME
#define PRELINK__NAME	PRELINK__CLASSSTR
#define PRELINK__LABEL	"Prelink"
#define PRELINK__DESC	"Prelink - Previzion scene link"
#define PRELINK__PREFIX	"Prelink"

#define PRELINK__HARDWARE_VERSION_INFO	"Prelink 1.7.6 (TCP, Threads)"

//--- FiLMBOX implementation and registration
FBDeviceImplementation	(	PRELINK__CLASS	);
FBRegisterDevice		(	PRELINK__NAME,
							PRELINK__CLASS,
							PRELINK__LABEL,
							PRELINK__DESC,
							PRELINK__ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool Prelink::FBCreate()
{
	IconFilename	= PRELINK__ICON;
	mExportRate		= 24.0;
	mSpaceInfo		= kLocalSpace;
	mClockInfo		= kLocalClock;
	mSendVisibility	= false;
	BadGroup::CriticalSectionInitialize();

	return true;
}


/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void Prelink::FBDestroy()
{
	BadGroup::CriticalSectionFinalize();
}


/************************************************
 *	Device operation.
 ************************************************/
bool Prelink::DeviceOperation( kDeviceOperations pOperation )
{
	switch (pOperation)
	{
		case kOpInit:	return Init();
		case kOpStart:	return Start();
		case kOpStop:	return Stop();
		case kOpReset:	return Reset();
		case kOpDone:	return Done();
	}
	return FBDevice::DeviceOperation( pOperation );
}


/************************************************
 *	Initialization of device.
 ************************************************/
bool Prelink::Init()
{
	HardwareVersionInfo = PRELINK__HARDWARE_VERSION_INFO;
	Information			= "";
	Status				= "Initialized";
	return true;
}


static _CrtMemState LEAK_DEBUG_MEMSTATE_1;
static unsigned int LEAK_DEBUG_DEN, LEAK_DEBUG_DION;


/************************************************
 *	Device is put online.
 ************************************************/
bool Prelink::Start()
{
	FBProgress	lProgress;
	lProgress.Caption	= "Setting up device";

#ifdef HASP_CHECK
	// Step 0: Licensing
	const hasp_feature_t feature = 10;

	mHaspHandle = HASP_INVALID_HANDLE_VALUE;
	hasp_status_t status;

	unsigned char vendor_code[] =
	"xs8V5jliexqjvh6ZHHDdzpO3dPsezbM3/koXg+DPRhZ8/yfeq6LGBU96ZeaGntYwZJ8UXfV6q1N8E7kz"
	"5HXKAAKQQdg9ZlxCeKh9Wphmw7DxuV/X8XJGV6yFcaSPNaxArXKzPArW9/dU1ypfUdu7t1xKfWQOmzKI"
	"kS8aWec6cqB6BSYYpCGWihXLJjJPR3KymgpWmaTvteZXrKjSfbPhoFSWsD6BHW3QbR5Bstll1aHeqtrv"
	"TIxcD98OhN1LTJB1DLIawewW2FFHCp9NFq3MevpUMIqtXC0KCbJIUUEy9tvngnMkFjOFV3yrj2T5TQGM"
	"UnAcLvg80dOSgLWU8T9WrC3ZerUtmUtEto50W3+4bGdYA0hNGPBN1xVTuiVNePupkViKctQj0DNCfTih"
	"21hNcpWUntbpK2ZFGEwG9/+FMcaEiIM8T4+qIgGRFRCxWiAKqXhiAjBMbuLUzv7Rxm0KToVW7+9WZ9H0"
	"Zpvny2FNNHZ3H6wfx6B0V4f90QxMW1GIE+NNr5keZ+osTvTR1Qup0uYYfnBlB1pWWuPNXVlgTYc/AlhY"
	"E4LTb8VnYGOGq439LJDY7xrL3qfamF9lZmas+xnwJk+X5j7IkTvWwmVtJRJp1pUvEtY0Dnw17ehBSGVv"
	"izzmJDqKJHnkAZ3iVm/W7oJ3fpmaslPUOzRfiHSzolEFnqzygcoSrPlIoyALEEK3g8JtpO4xE/mW5MiL"
	"o9MY9l1Qe0k9OH2EqC7YsFCipuT/g/od+v/1uCpANdMxEXVoYYk1n3KV0mjtuskHI94xDfyGNhsylJCG"
	"tpwU2MWGZmxMe1STEKebC6h664Jp/kf43aVj1w94I7P5I+/MK6bnUDM4QoW8bVEn5hQSNcJJHzuUq9ST"
	"Bu6YDMrC0UKRGKx21V3RnYX0YtXexw3OOe9ikwzXEE7cRLCFUj7HL8mL99A=";

	status = hasp_login(feature, vendor_code, &mHaspHandle);

	/* check if operation was successful */
	if (status != HASP_STATUS_OK) {
		switch (status)
		{
		case HASP_FEATURE_NOT_FOUND:
			Status = "License feature not found.";
			break;
		case HASP_HASP_NOT_FOUND:
			Status = "Dongle not found.";
			break;
		case HASP_OLD_DRIVER:
			Status = "Old HASP driver.";
			break;
		case HASP_NO_DRIVER:
			Status = "No HASP driver installed.";
			break;
		case HASP_INV_VCODE:
			Status = "Invalid Vendor Code.";
			break;
		case HASP_FEATURE_TYPE_NOT_IMPL:
			Status = "Feature not implemented.";
			break;
		case HASP_TMOF:
			Status = "HASP_TMOF";
			break;
		case HASP_TS_DETECTED:
			Status = "HASP_TS_DETECTED";
			break;
		default:
			std::ostrstream msg;
			msg << "Status code " << status << "" << std::endl;
			Status = msg.str();
			msg.freeze(false);
			break;
		}

		//handle error
		lProgress.Text		= "Prelink disabled.";
		HardwareVersionInfo = PRELINK__HARDWARE_VERSION_INFO;
		Information			= "";
		
		FBMessageBox("License", "Previzion Prelink cannot find license", "Continue");
		return false;
	}
#endif

	// Step 1: Open device
	lProgress.Text	= "Opening communication to device";
	Information		= "Opening device";
	if(!mHardware.Open())
	{
		Information = "Could not open device";
		return false;
	}

	// Step 2: Ask hardware to get channel information
	lProgress.Text	= "Device found, scanning for channel information...";
	Information		= "Retrieving channel information";
	if(!mHardware.GetSetupInfo())
	{
		Information = "Could not get channel information from device.";
		return false;
	}

	// Step 3: Set the sampling rate
	lProgress.Text	= "Setting sampling rate";
	Information	= "Setting sampling rate";

	FBTime	lTime;
	lTime.SetSecondDouble(1.0 / mExportRate);
	SamplingPeriod	= lTime;

	// Step 4: Add the scene change callback
	mSystem.Scene->OnChange.Add( this, (FBCallback)&Prelink::EventSceneChange );

	lProgress.Text		= "Prelink started.";
	HardwareVersionInfo = PRELINK__HARDWARE_VERSION_INFO;
	Information			= "";
	Status				= "Started";

#ifdef _LEAK_DEBUG
	_RPT0( _CRT_WARN, "Prelink device Start\n" );
	_CrtMemCheckpoint( &LEAK_DEBUG_MEMSTATE_1 );
	LEAK_DEBUG_DEN = 0;
	LEAK_DEBUG_DION = 0;
#endif

	return true;
}


/************************************************
 *	Device is stopped (offline).
 ************************************************/
bool Prelink::Stop()
{
#ifdef _LEAK_DEBUG
	_CrtMemState s2;
	_CrtMemCheckpoint( &s2 );
	_CrtMemState s3;
	if ( _CrtMemDifference( &s3, &LEAK_DEBUG_MEMSTATE_1, &s2) )
	{
		_RPT0( _CRT_WARN, "Prelink device Stop\n" );
		_RPT2( _CRT_WARN, "Cycles: DEN %d DION %d\n", LEAK_DEBUG_DEN, LEAK_DEBUG_DION );
		_CrtMemDumpStatistics( &s3 );
		_CrtMemDumpAllObjectsSince( &LEAK_DEBUG_MEMSTATE_1 );
		_RPT0( _CRT_WARN, "\n" );
		//debug_dump_blocks_in_heaps();
	}
#endif

	FBProgress	lProgress;
	lProgress.Caption	= "Shutting down device";

	// Shutdown device
	lProgress.Text	= "Closing communication to device";
	Information		= "Closing device";
	if(! mHardware.Close() )
	{
		Information = "Could not close device";
	}

	mSystem.Scene->OnChange.Remove( this, (FBCallback)&Prelink::EventSceneChange );

	HardwareVersionInfo = PRELINK__HARDWARE_VERSION_INFO;
	Information = "";
	Status = "Stopped";

#ifdef HASP_CHECK
	// Licensing
	hasp_status_t status = hasp_logout( mHaspHandle );

	if ( status != HASP_STATUS_OK )
	{
		//handle error
	}
#endif

	return false;
}


/************************************************
 *	Removal of device.
 ************************************************/
bool Prelink::Done()
{
	return false;
}


/************************************************
 *	Reset of device.
 ************************************************/
bool Prelink::Reset()
{
    Stop();
    return Start();
}


/************************************************
 * Check for bad models on export list
 ************************************************/

bool Prelink::BadExportModels()
{
	FBSystem lSys;
	bool lBadModels = false;

	for(int i=0;i<mExportModels.GetCount();i++)
	{
		HFBModel lModel = (HFBModel)mExportModels[i];
		if(BadModel(lModel))
		{
			BadGroup::add(lSys.Scene, lModel);
			mExportModels.Remove(lModel);
			lBadModels = true;
		} else {
			BadGroup::remove(lSys.Scene, lModel);
		}
	}
	return lBadModels;
}


/************************************************
 *	Real-Time Engine Evaluation.
 ************************************************/
bool Prelink::AnimationNodeNotify(HFBAnimationNode pAnimationNode ,HFBEvaluateInfo pEvaluateInfo)
{
	// Currently there's no readback from Previzion
	// We don't use the "pump through" pattern for output devices
    return true;
}


FBString StringFromTime(FBTime pTime)
{
#ifdef _PREFBSDK2013
	kLongLong lHour, lMin, lSec, lFrame, lField, lMilli;
	pTime.GetTime(lHour, lMin, lSec, lFrame, lField, lMilli);
	char lBuffer[128];
	sprintf_s(lBuffer, 128, "%02lld:%02lld:%02lld:%02lld", lHour, lMin, lSec, lFrame);
	return FBString(lBuffer);
#else
	int lHour, lMin, lSec, lFrame, lField, lMilli;
	pTime.GetTime(lHour, lMin, lSec, lFrame, lField, lMilli);
	char lBuffer[128];
	sprintf_s(lBuffer, 128, "%02d:%02d:%02d:%02d", lHour, lMin, lSec, lFrame);
	return FBString(lBuffer);
#endif
}


void Prelink::EnqueueListUpdate(FBModelList &pModelList, HFBEvaluateInfo pEvaluateInfo, bool pCheckModels)
{
	if(!pModelList.GetCount())
		return;

	Packet lPacket;

	MakePacket( lPacket, pModelList, pEvaluateInfo, pCheckModels );
	mHardware.EnqueuePacket( lPacket );
}

bool packetWorker(Packet &pPacket, HFBModel lModel, HFBEvaluateInfo pEvaluateInfo, bool pCheckModels, bool mSendVisibility, bool lGlobalInfo)
{
	if (!lModel) return false;

	if (pCheckModels)
	{
		FBSystem lSys;
		if (BadModel(lModel))
		{
			BadGroup::add(lSys.Scene, lModel);
			return false;
		} else {
			BadGroup::remove(lSys.Scene, lModel);
		}
	}

	FBString lName = lModel->LongName;	// name with namespace
	Node lNode(lName);
	pPacket.begin(lNode);
	{
		if (mSendVisibility)
		{
			//
#if 0
			PropertyBool lVisible("visible", lModel->Show && lModel->Visibility > 0.001);
#else
			PropertyBool lVisible("visible", lModel->Show && lModel->Visibility);
#endif
			pPacket.add(lVisible);
		}

		/*
		(From the FBX documentation)

		Here is how FBX SDK computes the transformation matrix for a node:

		Transform = T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1

		Where:		Is:
		Transform	Transformation matrix (4 X 4)
		T			Translation
		Roff		Rotation offset
		Rp			Rotation pivot
		Rpre		Pre-rotation
		R			Rotation
		Rpost		Post-rotation
		Rp-1		Inverse of the rotation pivot
		Soff		Scaling offset
		Sp			Scaling pivot
		S			Scaling
		Sp-1		Inverse of the scaling pivot

		Notes:
		Computations are performed from left to right.
		The effect of the formula is that any given vector is first translated, then rotated, then scaled.
		The R matrix takes into account the rotation order. Because of the mathematical properties of the matrices,
		R is the result of one of the possible combinations of Ry, Ry and Rz (each being matrices also). 
		For example, for the default rotation order of XYZ, R = Rx * Ry * Rz
		*/

		FBMatrix lTrans;
		lModel->GetMatrix(lTrans, kModelTranslation, lGlobalInfo, pEvaluateInfo);

		FBMatrix lRot;
		lModel->GetMatrix(lRot, kModelRotation, lGlobalInfo, pEvaluateInfo);

		FBMatrix lScale;
		lModel->GetMatrix(lScale, kModelScaling, lGlobalInfo, pEvaluateInfo);

		// Unhandled: rotation and scale both offset and pivot.

		if(lModel->RotationActive)
		{
			FBRotationOrder lRotOrder = (FBRotationOrder)(int)lModel->RotationOrder;

			FBMatrix lPreRot;
			FBRotationToMatrix(lPreRot, lModel->PreRotation, lRotOrder);

			FBMatrix lPostRot;
			FBRotationToMatrix(lPostRot, lModel->PostRotation, lRotOrder);

			FBMatrixMult(lRot, lPreRot, lRot);
			FBMatrixMult(lRot, lRot, lPostRot);
		}

		FBMatrix lFBMatrix;
		FBMatrixMult(lFBMatrix, lTrans, lRot);
		FBMatrixMult(lFBMatrix, lFBMatrix, lScale);

		double lCopy[4][4];
		for(int row=0;row<4;row++)
			for(int col=0;col<4;col++)
				lCopy[row][col] = lFBMatrix(row, col);

		Matrix lMatrix(lCopy);
		pPacket.add(lMatrix);
	}
	pPacket.end();	// Node

	return true;
}

void ErrorHandler(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code.

    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message.

	FBTrace((char*)lpMsgBuf);

    // Free error-handling buffer allocations.

    LocalFree(lpMsgBuf);
}

void Prelink::MakePacket(Packet &pPacket, FBModelList &pModelList, HFBEvaluateInfo pEvaluateInfo, bool pCheckModels)
{
	//BadGroup::CriticalSectionInitialize();

	Outer lOuter;
	pPacket.begin(lOuter);
	
	Frame lFrame;
	if(pEvaluateInfo)
	{
		switch(GetClockInfo())
		{
		case kNoClock:
			break;
		case kLocalClock:
#ifdef _PREFBSDK2012
			lFrame.setTime(StringFromTime(pEvaluateInfo->GetLocalStart()));
#else
			lFrame.setTime(StringFromTime(pEvaluateInfo->GetLocalTime()));
#endif
			break;
		case kSystemClock:
#ifdef _PREFBSDK2012
			lFrame.setTime(StringFromTime(pEvaluateInfo->GetSystemStart()));
#else
			lFrame.setTime(StringFromTime(pEvaluateInfo->GetSystemTime()));
#endif
			break;
		}
	}
	pPacket.begin(lFrame);

	int nModels = pModelList.GetCount();
	bool useThreads = mNumberOfThreads > 0;
	bool lGlobalInfo = GetSpaceInfo() == kGlobalSpace;

	if( !useThreads )
	{
		for(int i = 0; i < nModels; i++)
			packetWorker(pPacket, pModelList.GetAt(i), pEvaluateInfo, pCheckModels, mSendVisibility, lGlobalInfo);
	} else 
	{
		// compute the transforms in parallel
		Concurrency::concurrent_vector<Packet*> results(nModels);
		Concurrency::parallel_for(0, nModels, 
			[=, &pModelList, &results](int i)
			{
				Packet *p = new Packet;
				if( packetWorker( *p, pModelList.GetAt( i ), pEvaluateInfo, pCheckModels, mSendVisibility, lGlobalInfo ) )
					results[i] = p;
				else
				{
					delete p;
					results[i] = 0;
				}
			}
		);

		// assemble thread packets into pPacket
		std::for_each( results.begin(), results.end(),
			[&pPacket](Packet *tPacket)
			{
				if(tPacket)
				{
					pPacket.add(*tPacket);
					delete tPacket;
				}
			}
		);
	}

	pPacket.end();	// Frame

	pPacket.end();	// Outer

	//BadGroup::CriticalSectionFinalize();
}


/************************************************
 *	Device Evaluation Notify.
 ************************************************/
bool Prelink::DeviceEvaluationNotify(kTransportMode pMode, HFBEvaluateInfo pEvaluateInfo)
{
	switch(pMode)
	{
		case kPreparePlay:
		case kPlayReady:
		case kPlayStop:
		case kJog:
		{
		}
		break;
		case kStop:
		case kPlay:
		{
			LEAK_DEBUG_DEN++;
			EnqueueListUpdate(mExportModels, pEvaluateInfo);
			AckOneSampleReceived();		// should be "Sent" but that does not update display.
		}
		break;
	}

	return true;
}


/************************************************
 *	Real-Time Synchronous Device IO.
 ************************************************/
void Prelink::DeviceIONotify(kDeviceIOs  pAction, FBDeviceNotifyInfo &pDeviceNotifyInfo)
{
	FBTime lEvalTime;
    switch (pAction)
	{
		// Output devices
		case kIOPlayModeWrite:
		case kIOStopModeWrite:
		{
			LEAK_DEBUG_DION++;
			lEvalTime = pDeviceNotifyInfo.GetSystemTime();
			if( !mHardware.SendDataPacket(lEvalTime) )
				FBTrace("Packet not sent %s\n", StringFromTime(lEvalTime));
		}
		break;
		// Input devices
		case kIOStopModeRead:
		case kIOPlayModeRead:
		{
			//lEvalTime = pDeviceNotifyInfo.GetSystemTime();
			//mHardware.FetchDataPacket(lEvalTime);
		}
		break;
	}
}


//
void Prelink::EnqueueSelectionUpdate()
{
	FBModelList lSelection;
	FBGetSelectedModels(lSelection);
	
	// Any models on mExportModels to be removed from list.
	// This is a fix for transform corruption which occurs when 
	// a model in the export list is ALSO sent by GUI event.
	for(int i = 0; i < mExportModels.GetCount(); i++)
	{
		HFBModel lModel = mExportModels[i];
		if (lSelection.Find(lModel) != -1)
			lSelection.Remove(lModel);
	}

	EnqueueListUpdate(lSelection, 0, true);
}


//--- Events
void Prelink::EventSceneChange(HISender pSender, HKEvent pEvent)
{
	// This interface is not versatile enough to meet user expectations
	FBEventSceneChange lEvent(pEvent);
	switch(lEvent.Type)
	{
	case kFBSceneChangeSelect:
	case kFBSceneChangeUnselect:
	case kFBSceneChangeTransactionBegin:
		break;
	case kFBSceneChangeTransactionEnd:
		//if(mGUIEvents) 
		//	EnqueueSelectionUpdate();
		break;
	default:
		break;
	}
}


//--- FBX load/save tags
#define FBX_TAG_SECTION		PRELINK__CLASSSTR
#define FBX_TAG_VERSION		"Version"

#define FBX_TAG_RATE		"Rate"
#define FBX_TAG_SPACE		"SpaceInfo"
#define FBX_TAG_CLOCK		"ClockInfo"
#define FBX_TAG_MODELS		"Models"

#define FBX_TAG_VISIBILITY	"Visibility"
#define FBX_TAG_PACKETSPERFRAME	"PacketsPerFrame"
#define FBX_TAG_NUMBEROFTHREADS "NumberOfThreads"

#define FBX_TAG_ADDRESS		"NetworkAddress"
#define FBX_TAG_PORT		"NetworkPort"

/************************************************
 *	Store data in FBX.
 ************************************************/
bool Prelink::FbxStore(HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat)
{
	if (pStoreWhat & kAttributes)
	{
		pFbxObject->FieldWriteI(FBX_TAG_VERSION, 100);
		pFbxObject->FieldWriteC(FBX_TAG_ADDRESS, mHardware.GetNetworkAddress());
		pFbxObject->FieldWriteI(FBX_TAG_PORT, mHardware.GetNetworkPort());
		pFbxObject->FieldWriteD(FBX_TAG_PORT, mExportRate);
		pFbxObject->FieldWriteI(FBX_TAG_SPACE, (int)GetSpaceInfo());
		pFbxObject->FieldWriteI(FBX_TAG_CLOCK, (int)GetClockInfo());
		pFbxObject->FieldWriteI(FBX_TAG_VISIBILITY, (int)GetSendVisibility());
		pFbxObject->FieldWriteI(FBX_TAG_PACKETSPERFRAME, 1);	// compatability
		pFbxObject->FieldWriteI(FBX_TAG_NUMBEROFTHREADS, GetNumberOfThreads());

		pFbxObject->FieldWriteBegin(FBX_TAG_MODELS);
        for (int i=0; i<mExportModels.GetCount(); i++)
		{
			pFbxObject->FieldWriteObjectReference(mExportModels[i]);
        }
	    pFbxObject->FieldWriteEnd();
	}
	return true;
}


/************************************************
 *	Retrieve data from FBX.
 ************************************************/
bool Prelink::FbxRetrieve(HFBFbxObject pFbxObject,kFbxObjectStore pStoreWhat)
{
	int Version;
	if (pStoreWhat & kAttributes)
	{
		Version = pFbxObject->FieldReadI(FBX_TAG_VERSION);
		mHardware.SetNetworkAddress((char*)(pFbxObject->FieldReadC(FBX_TAG_ADDRESS)));
		mHardware.SetNetworkPort(pFbxObject->FieldReadI(FBX_TAG_PORT));
		mExportRate = pFbxObject->FieldReadD(FBX_TAG_PORT);
		SetSpaceInfo((Space)pFbxObject->FieldReadI(FBX_TAG_SPACE));
		SetClockInfo((Clock)pFbxObject->FieldReadI(FBX_TAG_CLOCK));
		SetSendVisibility(pFbxObject->FieldReadI(FBX_TAG_VISIBILITY) != 0);
		pFbxObject->FieldReadI(FBX_TAG_PACKETSPERFRAME);	// compatability
		SetNumberOfThreads(pFbxObject->FieldReadI(FBX_TAG_NUMBEROFTHREADS));

		mExportModels.Clear();
		if(pFbxObject->FieldReadBegin(FBX_TAG_MODELS))
		{
			int lModelCount = pFbxObject->FieldReadGetCount();
			for (int i=0; i<lModelCount; i++)
			{
				HIObject lObject = pFbxObject->FieldReadObjectReference();
				HFBModel lModel = FBCast<FBModel>(lObject, true);
				InternExportModel(lModel);
			}
			pFbxObject->FieldReadEnd();
		}
	}
	return true;
}
