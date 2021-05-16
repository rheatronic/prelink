#ifndef __PRELINK_DEVICE_H__
#define __PRELINK_DEVICE_H__
/*
 Prelink (C) 2010 by bitflix.  All rights reserved.
*/

/**	\file	prelink_device.h
*	Prelink, declarations for the top level plugin class.
*/

#include "bitflix/bitflix_debug.h"

//--- Class declaration
#include "prelink_hardware.h"
#ifdef HASP_CHECK
#include <hasp_api.h>
#endif

//--- Registration defines
#define PRELINK__CLASSNAME	Prelink
#define PRELINK__CLASSSTR	"Prelink"
#define PRELINK__ICON		"devices_netserver.tif"

//! Simple input device.
class Prelink : public FBDevice
{
	//--- FiLMBOX declaration
	FBDeviceDeclare( Prelink, FBDevice );

public:
	enum Clock
	{
		kNoClock,
		kLocalClock,
		kSystemClock
	};

	enum Space
	{
		kLocalSpace = 0,
		kGlobalSpace = 1
	};

	//--- FiLMBOX Construction/Destruction
	virtual bool FBCreate();		//!< FiLMBOX constructor.
	virtual void FBDestroy();		//!< FiLMBOX destructor.

	//--- Initialisation/Shutdown
	bool  Init();		//!< Initialize/create device.
	bool  Done();		//!< Remove device.
	bool  Reset();		//!< Reset device.
	bool  Stop();		//!< Stop device (offline).
	bool  Start();		//!< Start device (online).

	//--- The following will be called by the real-time engine.
	virtual bool AnimationNodeNotify	(	HFBAnimationNode pAnimationNode,	HFBEvaluateInfo pEvaluateInfo			);	//!< Real-time evaluation for node.
	virtual void DeviceIONotify			(	kDeviceIOs  pAction,				FBDeviceNotifyInfo &pDeviceNotifyInfo	);	//!< Notification of/for Device IO.
    virtual bool DeviceEvaluationNotify	(	kTransportMode pMode,				HFBEvaluateInfo pEvaluateInfo			);	//!< Evaluation the device (write to hardware).
	virtual bool DeviceOperation		(	kDeviceOperations pOperation												);	//!< Operate device.

	//--- Events
	void		 EventSceneChange		( HISender pSender, HKEvent pEvent );

	//--- FBX Load/Save.
	virtual bool FbxStore		( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat	);	//!< Store in FBX file.
	virtual bool FbxRetrieve	( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat	);	//!< Retrieve from FBX file.

	//--- Attribute management.
	double		GetExportRate		()						{ return mExportRate;						}
	void		SetExportRate		( double pRate )		{ mExportRate = pRate;						}

	bool		GetSendVisibility	()						{ return mSendVisibility;					}
	void		SetSendVisibility	( bool pSendVisibility ){ mSendVisibility = pSendVisibility;		}

	int			GetNumberOfThreads	()						{ return mNumberOfThreads;					}
	void		SetNumberOfThreads	( int pNumberOfThreads ){ mNumberOfThreads = pNumberOfThreads;		}

	Space		GetSpaceInfo		()						{ return mSpaceInfo;						}
	void		SetSpaceInfo		(Space pSpaceInfo)		{ mSpaceInfo = pSpaceInfo;					}

	Clock		GetClockInfo		()						{ return mClockInfo;						}
	void		SetClockInfo		(Clock pClockInfo)		{ mClockInfo = pClockInfo;					}
	
	void		SetNetworkAddress	( char *pIPAddress )	{ mHardware.SetNetworkAddress(pIPAddress);	}
	char *		GetNetworkAddress	()						{ return mHardware.GetNetworkAddress();		}
	void		SetNetworkPort		( int pPort )			{ mHardware.SetNetworkPort(pPort);			}
	int			GetNetworkPort		()						{ return mHardware.GetNetworkPort();		}

public:
	FBModelList	mExportModels;
	void		InternExportModel	(HFBModel pModel)		{if( pModel && mExportModels.Find( pModel ) == -1 ) mExportModels.Add( pModel );}

	bool		BadExportModels		();

	void EnqueueSelectionUpdate		();

private:
	void EnqueueListUpdate(FBModelList &pModelList, HFBEvaluateInfo pEvaluateInfo=0, bool pCheckModels=false);
	void MakePacket(Packet &pPacket, FBModelList &pModelList, HFBEvaluateInfo pEvaluateInfo, bool pCheckModels);

	FBSystem			mSystem;
	FBApplication		mApplication;
	double				mExportRate;		//!< FPS rate at which to stream.
	Space				mSpaceInfo;			//!< Send global or local space transforms?
	Clock				mClockInfo;			//!< Clock to send, None, Local or System.
	bool				mSendVisibility;	//!< Send visibility of nodes?
	int					mNumberOfThreads;	//!< Number of threads to use for XML processing
	PrelinkHardware		mHardware;			//!< Handle onto hardware.
#ifdef HASP_CHECK
	hasp_handle_t		mHaspHandle;		//!< License handle.
#endif
};

#endif /* __PRELINK_DEVICE_H__ */
