#ifndef __PRELINK_LAYOUT_H__
#define __PRELINK_LAYOUT_H__
/*
 Prelink (C) 2010 by bitflix.  All rights reserved.
*/

/**	\file	prelink_layout.h
*	PrelinkLayout, declarations for the GUI panel.
*/

#include "bitflix/bitflix_debug.h"

//--- Class declaration
#include "prelink_device.h"

//! Simple device layout.
class PrelinkLayout : public FBDeviceLayout
{
	//--- FiLMBOX declaration.
	FBDeviceLayoutDeclare( PrelinkLayout, FBDeviceLayout );

public:
	//--- FiLMBOX Creation/Destruction.
	virtual bool FBCreate();		//!< FiLMBOX Constructor.
	virtual void FBDestroy();		//!< FiLMBOX Destructor.

	// UI Management
	void	UICreate				();
	void		UICreateLayout0		();
	void		UICreateLayout1		();
	void		UICreateLayout2		();
	void	UIConfigure				();
	void		UIConfigureLayout0	();
	void		UIConfigureLayout1	();
	void		UIConfigureLayout2	();
	void	UIReset					();
	void	UIRefresh				();

	// Main Layout: Events
	void	EventDeviceStatusChange				( HISender pSender, HKEvent pEvent );
	void	EventUIIdle							( HISender pSender, HKEvent pEvent );
	void	EventTabPanelChange					( HISender pSender, HKEvent pEvent );

	// Layout 0: Events
	void	EventEditNumberExportRateChange		( HISender pSender, HKEvent pEvent );
	void	EventListTransformsChange			( HISender pSender, HKEvent pEvent );
	void	EventListSendTimeChange				( HISender pSender, HKEvent pEvent );
	void	EventButtonClearAllModelsClick		( HISender pSender, HKEvent pEvent );
	void	EventButtonClearStaticModelsClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonAddModelsClick			( HISender pSender, HKEvent pEvent );
	void	EventButtonSubModelsClick			( HISender pSender, HKEvent pEvent );
	void	EventButtonSelectHilightedClick		( HISender pSender, HKEvent pEvent );
	void	EventButtonSendModelsClick			( HISender pSender, HKEvent pEvent );
	void	EventContainerExportNamesDragAndDrop( HISender pSender, HKEvent pEvent );

	// Layout 1: Events
	void	EventEditNetworkAddressChange		( HISender pSender, HKEvent pEvent );
	void	EventEditNetworkPortChange			( HISender pSender, HKEvent pEvent );

	// Layout 2: Events
	void	EventButtonSendVisibilityClick		( HISender pSender, HKEvent pEvent );
	void	EventListNumberOfThreadsChange		( HISender pSender, HKEvent pEvent );

public:
	FBTabPanel			mTabPanel;

	FBLayout			mLayoutStreams;
		FBLabel				mLabelExportRate;
		FBEditNumber		mEditNumberExportRate;
		FBLabel				mLabelTransforms;
		FBList				mListTransforms;
		FBLabel				mLabelSendTime;
		FBList				mListSendTime;
		FBButton			mButtonClearAllModels;
		FBButton			mButtonClearStaticModels;
		FBButton			mButtonAddModels;
		FBButton			mButtonSubModels;
		FBButton			mButtonSelectHilighted;
		FBButton			mButtonSendModels;
		FBVisualContainer	mContainerExportNames;

	FBLayout			mLayoutEvents;
		FBLabel				mLabelSendVisibility;
		FBButton			mButtonSendVisibility;
		FBLabel				mLabelNumberOfThreads;
		FBList				mListNumberOfThreads;

	FBLayout			mLayoutCommunication;
		FBButton			mButtonChannelType;
			FBLabel				mLabelNetworkAddress;
			FBEdit				mEditNetworkAddress;
			FBLabel				mLabelNetworkPort;
			FBEditNumber		mEditNetworkPort;

private:
	void			CheckExportModels();
	void			UpdateExportModels();

	FBSystem			mSystem;		//!< System interface.
	Prelink*			mDevice;		//!< Handle onto device.
};

#endif /* __PRELINK_LAYOUT_H__ */
