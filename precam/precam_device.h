#ifndef __PRECAM_DEVICE_H__
#define __PRECAM_DEVICE_H__

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <fbsdk/fbtracker.h>

//--- Class declaration
#include "precam_hardware.h"

//--- Registration defines
#define PRECAM__CLASSNAME		Precam
#define PRECAM__CLASSSTR		"PrevizionCamera"

//! Simple input device.
class Precam : public FBDeviceCamera
{
	//--- FiLMBOX declaration
	FBDeviceDeclare( Precam, FBDeviceCamera );

public:
	//--- FiLMBOX Construction/Destruction
	virtual bool FBCreate();		//!< FiLMBOX Constructor.
	virtual void FBDestroy();		//!< FiLMBOX Destructor.

	//--- FiLMBOX Device functions
	//virtual bool DeviceSyncAnimationNodeNotify	( HFBAnimationNode pAnimationNode, HFBEvaluateInfo pEvaluateInfo, int pCorrectedSync	);
	virtual bool DeviceOperation				( kDeviceOperations pOperation															);
	virtual bool DeviceCameraAnimationNodeNotify( HFBEvaluateInfo pEvaluateInfo, HFBDeviceCameraPacket pDstPacket, HFBDeviceSyncPacket pSrcPacket );
	virtual void DeviceCameraEvalData			( FBDeviceNotifyInfo &pDeviceNotifyInfo													);
	virtual void RecordingDoneAnimation			( HFBAnimationNode pAnimationNode														);
	virtual bool ModelTemplateBindNotify		( HFBModel pModel, int pIndex, HFBModelTemplate pModelTemplate );

	//--- Load/Save.
	virtual bool FbxStore	( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool FbxRetrieve( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat );

	//--- Initialisation/Shutdown
	bool  Init();					//!< Initialization routine.
	bool  Done();					//!< Device removal.
	bool  Stop();					//!< Device online routine.
	bool  Start();					//!< Device offline routine.

	//--- Network
	void		SetNetworkAddress	(const char *pIPAddress){ mHardware.SetNetworkAddress(pIPAddress);		}
	char*		GetNetworkAddress	()						{ return mHardware.GetNetworkAddress();			}
	void		SetNetworkPort		(int pPort)				{ mHardware.SetNetworkPort(pPort);				}
	int			GetNetworkPort		()						{ return mHardware.GetNetworkPort();			}

	//--- Recording
	virtual void DeviceCameraRecordFrame( FBTime &pRecordTime, FBDeviceNotifyInfo& pNotifyInfo, PrecamPacket* pPacket );

	void BuildQueue();
	void DeleteQueue();
	void ResetQueue();

	PrecamPacket RecordPacket;

private:
	FBSystem	    mSystem;
	FBPlayerControl mPlayerControl;

    Precam_Hardware		mHardware;			//!< Handle onto hardware.

	HFBAnimationNode	mNodeCamera1_SubjectDistance;	//!< Raw channels from stream
	HFBAnimationNode	mNodeCamera1_DistortionR2;
	HFBAnimationNode	mNodeCamera1_Iris;
	HFBAnimationNode	mNodeCamera1_TC_Hour;
	HFBAnimationNode	mNodeCamera1_TC_Minute;
	HFBAnimationNode	mNodeCamera1_TC_Second;
	HFBAnimationNode	mNodeCamera1_TC_Frame;
	HFBAnimationNode	mNodeCamera1_TC_Rate;

	// Packet recording
	bool		mFirstPacket;
	FBTime		mRecordStartTime;
	FBTime		mFirstPacketTime;

};

#endif /* __PRECAM_DEVICE_H__ */
