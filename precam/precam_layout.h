#ifndef __PRECAM_LAYOUT_H__
#define __PRECAM_LAYOUT_H__

//--- Class declaration
#include "precam_device.h"

//! Simple device layout.
class Precam_Layout : public FBDeviceCameraLayout
{
	//--- FiLMBOX declaration.
	FBDeviceLayoutDeclare( Precam_Layout, FBDeviceCameraLayout );

public:
	//--- FiLMBOX Creation/Destruction.
	virtual bool FBCreate();			//!< FiLMBOX constructor.
	virtual void FBDestroy();			//!< FiLMBOX destructor.

	//--- Custom UI
	virtual void	UICreateLayoutCustom	();
	virtual void	UIConfigure				();
	virtual void		UIConfigureLayoutCustom	();
	virtual void	UIRefreshLayoutCustom	();
	virtual void	UIResetLayoutCustom		();
	virtual void	UIUpdateEnabled			();

	// Communications
	virtual void	OnNetworkChange				( HISender pSender, HKEvent pEvent );
	virtual void		OnNetworkAddressChange		( HISender pSender, HKEvent pEvent );
	virtual void		OnNetworkPortChange			( HISender pSender, HKEvent pEvent );

private:
	FBSystem		mSystem;		//!< System interface.
	Precam*			mCameraDevice;	//!< Handle onto device.
};

#endif /* _PRECAM_LAYOUT_H_ */
