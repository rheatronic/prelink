/*
 Prelink (C) 2010 by bitflix.  All rights reserved.
*/

/**	\file	prelink_layout.cxx
*	PrelinkLayout class, a GUI panel for output to Previzion.
*/

//--- Class declarations
#include "prelink_layout.h"


//--- Registration define
#define PRELINK__LAYOUT	PrelinkLayout

//--- FiLMBOX implementation and registration
FBDeviceLayoutImplementation(	PRELINK__LAYOUT		);
FBRegisterDeviceLayout		(	PRELINK__LAYOUT,
								PRELINK__CLASSSTR,
								PRELINK__ICON		);


/************************************************
 *	FiLMBOX constructor.
 ************************************************/
bool PrelinkLayout::FBCreate()
{
	// Get a handle on the device.
	mDevice = ((Prelink *)(FBDevice *)Device);

	UICreate	();
	UIConfigure	();
	UIReset		();

	mDevice->OnStatusChange.Add	( this,(FBCallback)&PrelinkLayout::EventDeviceStatusChange   );
	//mSystem.OnUIIdle.Add		( this,(FBCallback)&PrelinkLayout::EventUIIdle               );

	return true;
}


/************************************************
 *	FiLMBOX destructor.
 ************************************************/
void PrelinkLayout::FBDestroy()
{
	mDevice->OnStatusChange.Remove	( this,(FBCallback)&PrelinkLayout::EventDeviceStatusChange   );
	//mSystem.OnUIIdle.Remove			( this,(FBCallback)&PrelinkLayout::EventUIIdle               );
}


/************************************************
 *	UI Creation.
 ************************************************/
void PrelinkLayout::UICreate()
{
	int lS, lH;		// space, height
	lS = 5;
	lH = 25;

	// Create regions
	AddRegion	( "TabPanel",	"TabPanel",
					0,		kFBAttachLeft,		"",			1.00,
					0,		kFBAttachTop,		"",			1.00,
					0,		kFBAttachRight,		"",			1.00,
					lH,		kFBAttachNone,		NULL,		1.00 );
	AddRegion	( "MainLayout",	"MainLayout",
					lS,		kFBAttachLeft,		"TabPanel",	1.00,
					lS,		kFBAttachBottom,	"TabPanel",	1.00,
					-lS,	kFBAttachRight,		"TabPanel",	1.00,
					-lS,	kFBAttachBottom,	"",			1.00 );

	// Assign regions
	SetControl	( "TabPanel",	mTabPanel		);
	SetControl	( "MainLayout",	mLayoutStreams	);

	// Create sub layouts
	UICreateLayout0();
	UICreateLayout1();
	UICreateLayout2();
}


/************************************************
 *	Create the sub-layout 0.
 ************************************************/
void PrelinkLayout::UICreateLayout0()
{
	int lS, lW, lH;		// space, width, height.
	lS = 4;
	lW = 100;
	lH = 18;

	// Add regions
	mLayoutStreams.AddRegion("LabelExportRate",	"LabelExportRate",
							 lS,		kFBAttachLeft,		"",						1.0,
							 lS,		kFBAttachTop,		"",						1.0,
							 lW,		kFBAttachNone,		NULL,					1.0,
							 lH,		kFBAttachNone,		NULL,					1.0 );
	mLayoutStreams.AddRegion("EditNumberExportRate",	"EditNumberExportRate",
							 lS,		kFBAttachRight,		"LabelExportRate",		1.0,
							 0,			kFBAttachTop,		"LabelExportRate",		1.0,
							 (3*lW)/2,	kFBAttachNone,		NULL,					1.0,
							 0,			kFBAttachHeight,	"LabelExportRate",		1.0 );

	mLayoutStreams.AddRegion("LabelTransforms",	"LabelTransforms",
							 0,			kFBAttachLeft,		"LabelExportRate",		1.0,
							 lS,		kFBAttachBottom,	"LabelExportRate",		1.0,
							 0,			kFBAttachWidth,		"LabelExportRate",		1.0,
							 0,			kFBAttachHeight,	"LabelExportRate",		1.0 );
	mLayoutStreams.AddRegion("ListTransforms",	"ListTransforms",
							 lS,		kFBAttachRight,		"LabelExportRate",		1.0,
							 0,			kFBAttachTop,		"LabelTransforms",		1.0,
							 0,			kFBAttachWidth,		"EditNumberExportRate",	1.0,
							 0,			kFBAttachHeight,	"LabelExportRate",		1.0 );

	mLayoutStreams.AddRegion("LabelSendTime",	"LabelSendTime",
							 0,			kFBAttachLeft,		"LabelExportRate",		1.0,
							 lS,		kFBAttachBottom,	"LabelTransforms",		1.0,
							 0,			kFBAttachWidth,		"LabelExportRate",		1.0,
							 0,			kFBAttachHeight,	"LabelExportRate",		1.0 );
	mLayoutStreams.AddRegion("ListSendTime",	"ListSendTime",
							 lS,		kFBAttachRight,		"LabelExportRate",		1.0,
							 0,			kFBAttachTop,		"LabelSendTime",		1.0,
							 0,			kFBAttachWidth,		"EditNumberExportRate",	1.0,
							 0,			kFBAttachHeight,	"LabelExportRate",		1.0 );

	mLayoutStreams.AddRegion("ButtonClearAllModels",	"ButtonClearAllModels",
							 0,			kFBAttachLeft,		"LabelExportRate",		1.0,
							 lS,		kFBAttachBottom,	"LabelSendTime",		1.0,
							 0,			kFBAttachWidth,		"LabelExportRate",		1.0,
							 0,			kFBAttachHeight,	"LabelExportRate",		1.0 );
	mLayoutStreams.AddRegion("ButtonClearStaticModels",	"ButtonClearStaticModels",
							 0,			kFBAttachLeft,		"LabelExportRate",		1.0,
							 2*lS,		kFBAttachBottom,	"ButtonClearAllModels",		1.0,
							 0,			kFBAttachWidth,		"LabelExportRate",		1.0,
							 0,			kFBAttachHeight,	"LabelExportRate",		1.0 );

	mLayoutStreams.AddRegion("ButtonAddModels",			"ButtonAddModels",
							 0,			kFBAttachLeft,		"LabelExportRate",		1.0,
							 2*lS,		kFBAttachBottom,	"ButtonClearStaticModels",	1.0,
							 0,			kFBAttachWidth,		"LabelExportRate",		1.0,
							 0,			kFBAttachHeight,	"LabelExportRate",		1.0 );
	mLayoutStreams.AddRegion("ButtonSubModels",			"ButtonSubModels",
						     0,			kFBAttachLeft,		"LabelExportRate",		1.0,
							 2*lS,		kFBAttachBottom,	"ButtonAddModels",		1.0,
							 0,			kFBAttachWidth,		"LabelExportRate",		1.0,
							 0,			kFBAttachHeight,	"LabelExportRate",		1.0 );
	mLayoutStreams.AddRegion("ButtonSelectHilighted",		"ButtonSelectHilighted",
						     0,			kFBAttachLeft,		"LabelExportRate",		1.0,
							 2*lS,		kFBAttachBottom,	"ButtonSubModels",		1.0,
							 0,			kFBAttachWidth,		"LabelExportRate",		1.0,
							 0,			kFBAttachHeight,	"LabelExportRate",		1.0 );

	mLayoutStreams.AddRegion("ContainerExportNames",	"ContainerExportNames",
							 lS,		kFBAttachRight,		"LabelExportRate",		1.0,
							 0,			kFBAttachTop,		"ButtonClearAllModels",	1.0,
							 150,		kFBAttachNone,		NULL,					1.0,
							 200,		kFBAttachNone,		NULL,					1.0 );

	mLayoutStreams.AddRegion("ButtonSendModels",		"ButtonSendModels",
							 lS,		kFBAttachRight,		"ContainerExportNames",	1.0,
							 0,			kFBAttachTop,		"ButtonClearAllModels",	1.0,
							 0,			kFBAttachWidth,		"LabelExportRate",		1.0,
							 0,			kFBAttachHeight,	"LabelExportRate",		1.0 );
	// Assign regions
	mLayoutStreams.SetControl("LabelExportRate",		mLabelExportRate		);
	mLayoutStreams.SetControl("EditNumberExportRate",	mEditNumberExportRate	);

	mLayoutStreams.SetControl("LabelTransforms",		mLabelTransforms		);
	mLayoutStreams.SetControl("ListTransforms",			mListTransforms			);

	mLayoutStreams.SetControl("LabelSendTime",			mLabelSendTime			);
	mLayoutStreams.SetControl("ListSendTime",			mListSendTime			);

	mLayoutStreams.SetControl("ButtonClearAllModels",	mButtonClearAllModels	);
	mLayoutStreams.SetControl("ButtonClearStaticModels",	mButtonClearStaticModels	);
	mLayoutStreams.SetControl("ButtonAddModels",		mButtonAddModels		);
	mLayoutStreams.SetControl("ButtonSubModels",		mButtonSubModels		);
	mLayoutStreams.SetControl("ButtonSelectHilighted",	mButtonSelectHilighted	);
	mLayoutStreams.SetControl("ButtonSendModels",		mButtonSendModels		);
	mLayoutStreams.SetControl("ContainerExportNames",	mContainerExportNames	);
}


void PrelinkLayout::UICreateLayout1()
{
	int lS, lW, lH;		// space, width, height.
	lS = 4;
	lW = 120;
	lH = 18;

	// Add regions
	mLayoutEvents.AddRegion("LabelSendVisibility",	"LabelSendVisibility",
							 lS,		kFBAttachLeft,		"",						1.0,
							 lS,		kFBAttachTop,		"",						1.0,
							 lW,		kFBAttachNone,		NULL,					1.0,
							 lH,		kFBAttachNone,		NULL,					1.0 );
	mLayoutEvents.AddRegion("ButtonSendVisibility",	"ButtonSendVisibility",
							 lS,		kFBAttachRight,		"LabelSendVisibility",	1.0,
							 0,			kFBAttachTop,		"LabelSendVisibility",	1.0,
							 lW/2,		kFBAttachNone,		NULL,					1.0,
							 0,			kFBAttachHeight,	"LabelSendVisibility",	1.0 );

	mLayoutEvents.AddRegion("LabelNumberOfThreads", "LabelNumberOfThreads",
							 0,			kFBAttachLeft,		"LabelSendVisibility",	1.0,
							 lS,		kFBAttachBottom,	"LabelSendVisibility",	1.0,
							 0,			kFBAttachWidth,		"LabelSendVisibility",	1.0,
							 0,			kFBAttachHeight,	"LabelSendVisibility",	1.0 );
	mLayoutEvents.AddRegion("ListNumberOfThreads",	"ListNumberOfThreads",
							 lS,		kFBAttachRight,		"LabelSendVisibility",	1.0,
							 0,			kFBAttachTop,		"LabelNumberOfThreads",	1.0,
							 lW/2,		kFBAttachNone,		NULL,					1.0,
							 0,			kFBAttachHeight,	"LabelSendVisibility",	1.0 );


	// Assign regions
	mLayoutEvents.SetControl("LabelSendVisibility",			mLabelSendVisibility		);
	mLayoutEvents.SetControl("ButtonSendVisibility",		mButtonSendVisibility		);

	mLayoutEvents.SetControl("LabelNumberOfThreads",		mLabelNumberOfThreads		);
	mLayoutEvents.SetControl("ListNumberOfThreads",			mListNumberOfThreads		);
}


/************************************************
 *	Create the communications layout.
 ************************************************/
void PrelinkLayout::UICreateLayout2()
{
	int lS		= 5;
	int lSx		= 15;
	int lSy		= 20;
	int lS_y	= -15;
	int lW		= 100;
	int lH		= 18;
	int lHlr	= 75;
	int lWlr	= 200;
	int lWrb	= 80;
	int lSlbx	= 15;
	int lSlby	= 10;
	int lWlb	= 80;

	// Add regions (network)
	mLayoutCommunication.AddRegion( "LayoutRegionNetwork",	"LayoutRegionNetwork",
									lSx,	kFBAttachLeft,		"",						1.00,
									lSy,	kFBAttachTop,		"",						1.00,
									lWlr,	kFBAttachNone,		NULL,					1.00,
									lHlr,	kFBAttachNone,		NULL,					1.00 );
	mLayoutCommunication.AddRegion( "ButtonChannelType",	"ButtonChannelType",
									lS,		kFBAttachLeft,		"LayoutRegionNetwork",	1.00,
									lS_y,	kFBAttachTop,		"LayoutRegionNetwork",	1.00,
									lWrb,	kFBAttachNone,		NULL,					1.00,
									lH,		kFBAttachNone,		NULL,					1.00 );
	mLayoutCommunication.AddRegion( "LabelNetworkAddress",		"LabelNetworkAddress",
									lSlbx,	kFBAttachLeft,		"LayoutRegionNetwork",	1.00,
									lSlby,	kFBAttachTop,		"LayoutRegionNetwork",	1.00,
									lWlb,	kFBAttachNone,		NULL,					1.00,
									lH,		kFBAttachNone,		NULL,					1.00 );
	mLayoutCommunication.AddRegion( "EditNetworkAddress",		"EditNetworkAddress",
									lS,		kFBAttachRight,		"LabelNetworkAddress",		1.00,
									0,		kFBAttachTop,		"LabelNetworkAddress",		1.00,
									lW,		kFBAttachNone,		NULL,					1.00,
									lH,		kFBAttachNone,		NULL,					1.00 );
	mLayoutCommunication.AddRegion( "LabelNetworkPort",	"LabelNetworkPort",
									0,		kFBAttachLeft,		"LabelNetworkAddress",		1.00,
									lS,		kFBAttachBottom,	"LabelNetworkAddress",		1.00,
									0,		kFBAttachWidth,		"LabelNetworkAddress",		1.00,
									0,		kFBAttachHeight,	"LabelNetworkAddress",		1.00 );
	mLayoutCommunication.AddRegion( "EditNetworkPort",	"EditNetworkPort",
									lS,		kFBAttachRight,		"LabelNetworkPort",	1.00,
									0,		kFBAttachTop,		"LabelNetworkPort",	1.00,
									0,		kFBAttachWidth,		"EditNetworkAddress",		1.00,
									0,		kFBAttachHeight,	"LabelNetworkPort",	1.00 );

	// Assign regions (network)
	mLayoutCommunication.SetControl( "ButtonChannelType",	mButtonChannelType		);
	mLayoutCommunication.SetControl( "LabelNetworkAddress",	mLabelNetworkAddress	);
	mLayoutCommunication.SetControl( "EditNetworkAddress",	mEditNetworkAddress		);
	mLayoutCommunication.SetControl( "LabelNetworkPort",	mLabelNetworkPort		);
	mLayoutCommunication.SetControl( "EditNetworkPort",		mEditNetworkPort		);
}


/************************************************
 *	Configure the UI elements.
 ************************************************/
void PrelinkLayout::UIConfigure()
{
	SetBorder ("MainLayout", kFBStandardBorder, false,true, 1, 0,90,0);

	mTabPanel.Items.SetString("Stream~Options~Network");
	mTabPanel.OnChange.Add( this, (FBCallback)&PrelinkLayout::EventTabPanelChange );

	UIConfigureLayout0();
	UIConfigureLayout1();
	UIConfigureLayout2();
}


/************************************************
 *	Configure the test button layout.
 ************************************************/
void PrelinkLayout::UIConfigureLayout0()
{
	// Configure elements
	mLabelExportRate.Caption		= "Export FPS:";

	mEditNumberExportRate.Min		= 1.0;
	mEditNumberExportRate.Max		= 120.0;
	mEditNumberExportRate.Precision	= 3.3;
	mEditNumberExportRate.SmallStep	= 0.001;
	mEditNumberExportRate.LargeStep	= 1.0;

	mLabelTransforms.Caption		= "Transforms:";
	
	mListTransforms.Items.InsertAt(Prelink::kLocalSpace, "Local");
	mListTransforms.Items.InsertAt(Prelink::kGlobalSpace, "Global");

	mLabelSendTime.Caption			= "Send time:";

	mListSendTime.Items.InsertAt(Prelink::kNoClock, "None");
	mListSendTime.Items.InsertAt(Prelink::kLocalClock, "Local");
	mListSendTime.Items.InsertAt(Prelink::kSystemClock, "System");

	mButtonClearAllModels.Caption	= "Clear All";
	mButtonClearStaticModels.Caption	= "Clear Static";
	mButtonAddModels.Caption		= "Add Selected";
	mButtonSubModels.Caption		= "Sub Selected";
	mButtonSelectHilighted.Caption  = "Select Hilighted";
	mButtonSendModels.Caption		= "Send Selected";

    mContainerExportNames.IconPosition	= kFBIconLeft;

	// Add callbacks
	mEditNumberExportRate.OnChange.Add		( this, (FBCallback) &PrelinkLayout::EventEditNumberExportRateChange );
	mListTransforms.OnChange.Add			( this, (FBCallback) &PrelinkLayout::EventListTransformsChange );
	mListSendTime.OnChange.Add				( this, (FBCallback) &PrelinkLayout::EventListSendTimeChange );
	mButtonClearAllModels.OnClick.Add		( this, (FBCallback) &PrelinkLayout::EventButtonClearAllModelsClick );
	mButtonClearStaticModels.OnClick.Add	( this, (FBCallback) &PrelinkLayout::EventButtonClearStaticModelsClick );
	mButtonAddModels.OnClick.Add			( this, (FBCallback) &PrelinkLayout::EventButtonAddModelsClick );
	mButtonSubModels.OnClick.Add			( this, (FBCallback) &PrelinkLayout::EventButtonSubModelsClick );
	mButtonSelectHilighted.OnClick.Add		( this, (FBCallback) &PrelinkLayout::EventButtonSelectHilightedClick );
	mButtonSendModels.OnClick.Add			( this, (FBCallback) &PrelinkLayout::EventButtonSendModelsClick );
    mContainerExportNames.OnDragAndDrop.Add	( this, (FBCallback) &PrelinkLayout::EventContainerExportNamesDragAndDrop );
}


void PrelinkLayout::UIConfigureLayout1()
{
	// Configure elements
	mLabelSendVisibility.Caption		= "Send Visibility changes:";

	mButtonSendVisibility.Caption		= "Send";
	mButtonSendVisibility.Style			= kFBCheckbox;
	mButtonSendVisibility.State			= 1;

	mLabelNumberOfThreads.Caption		= "Use Threads";

	mListNumberOfThreads.Items.InsertAt(0, "No");
	mListNumberOfThreads.Items.InsertAt(1, "Yes");

	// Add callbacks
	mButtonSendVisibility.OnClick.Add			( this, (FBCallback) &PrelinkLayout::EventButtonSendVisibilityClick );
	mListNumberOfThreads.OnChange.Add			( this, (FBCallback) &PrelinkLayout::EventListNumberOfThreadsChange );
}


/************************************************
 *	Configure the communiciations layout.
 ************************************************/
void PrelinkLayout::UIConfigureLayout2()
{
	// Configure elements
    mLayoutCommunication.SetBorder( "LayoutRegionNetwork",	kFBEmbossBorder,false,true,2,1,90.0,0 );

	mButtonChannelType.Caption		= "TCP/IP";
	mButtonChannelType.Style		= kFBRadioButton;
	mButtonChannelType.State		= 1;

	mLabelNetworkAddress.Caption	= "Address:";
	mEditNetworkAddress.OnChange.Add( this, (FBCallback)&PrelinkLayout::EventEditNetworkAddressChange );

	mLabelNetworkPort.Caption		= "Port:";
	mEditNetworkPort.Min			= 1.0;
	mEditNetworkPort.Precision		= 1.0;
	mEditNetworkPort.SmallStep		= 1.0;
	mEditNetworkPort.LargeStep		= 1.0;

	// Add callbacks
	mEditNetworkPort.OnChange.Add( this, (FBCallback)&PrelinkLayout::EventEditNetworkPortChange );
}


/************************************************
 *	Refresh the UI.
 ************************************************/
void PrelinkLayout::UIRefresh()
{
}


/************************************************
 *	Reset the UI components from the device.
 ************************************************/
void PrelinkLayout::UIReset()
{
	mEditNumberExportRate.Value		= mDevice->GetExportRate();
	mListTransforms.ItemIndex		= mDevice->GetSpaceInfo();
	mListSendTime.ItemIndex			= mDevice->GetClockInfo();
	UpdateExportModels();

	mButtonSendVisibility.State		= mDevice->GetSendVisibility() ? 1 : 0;
	mListNumberOfThreads.ItemIndex	= mDevice->GetNumberOfThreads();

	mEditNetworkAddress.Text		= mDevice->GetNetworkAddress();
	mEditNetworkPort.Value			= mDevice->GetNetworkPort();
}


/************************************************
 *	Export names drag/drop callback.
 ************************************************/
void PrelinkLayout::EventContainerExportNamesDragAndDrop( HISender pSender, HKEvent pEvent )
{
    FBEventDragAndDrop	lDragAndDrop( pEvent );

	switch( lDragAndDrop.State )
	{
		case kFBDragAndDropDrag:
		{
			lDragAndDrop.Accept();
		}
		break;
		case kFBDragAndDropDrop:
		{
			if( mDevice->Online )
			{
				mDevice->DeviceSendCommand( FBDevice::kOpStop );
			}

			for(int i=0; i < lDragAndDrop.GetCount(); i++) 
			{
				HFBComponent lComponent = lDragAndDrop.Get(i);
	            if( lComponent && lComponent->Is( FBModel::TypeInfo ) )
				{
					HFBModel lModel = (HFBModel)lComponent;
					mDevice->InternExportModel( lModel );
				} 
				else if( lComponent && lComponent->Is( FBCharacter::TypeInfo ) )
				{
					HFBCharacter lCharacter = (HFBCharacter)lComponent;

					for(int nodeId=0; nodeId < (int)kFBLastNodeId; nodeId++)
					{
						HFBModel lModel = lCharacter->GetModel((FBBodyNodeId)nodeId);
						mDevice->InternExportModel( lModel );
					}
				}
			}

			CheckExportModels();

			UIReset();
		}
		break;
	}
}


void PrelinkLayout::UpdateExportModels()
{
	mContainerExportNames.Items.Clear();

	if( mDevice->mExportModels.GetCount() == 0 )
	{
		mContainerExportNames.Items.Add( "--- No Models ---" );
	}

	int i;
	for(i=0;i<mDevice->mExportModels.GetCount();i++)
	{
		mContainerExportNames.Items.Add( mDevice->mExportModels[i]->Name, (unsigned long) mDevice->mExportModels[i] );
	}
}


void PrelinkLayout::CheckExportModels()
{
	if(mDevice->BadExportModels())
	{
		// pop warning dialog
		FBMessageBox(
			"Bad models for prelink export...", 
			"Some models had Rotation or Scaling offsets or pivots.\n"
			"These would have caused bad transforms to be applied in Previzion.\n"
			"They were removed and added to the group 'Nonzero Pivot or Offset'\n"
			"for your reference.",
			"Ok");
	}
}


/************************************************
 *	Export rate change callback.
 ************************************************/
void PrelinkLayout::EventEditNumberExportRateChange( HISender pSender, HKEvent pEvent )
{
	double rate = mEditNumberExportRate.Value;

	if( mDevice->Online )
	{
		mDevice->DeviceSendCommand( FBDevice::kOpStop );
	}

	mDevice->SetExportRate( rate );
	mEditNumberExportRate.Value = mDevice->GetExportRate();
}


void PrelinkLayout::EventListTransformsChange( HISender pSender, HKEvent pEvent )
{
	mDevice->SetSpaceInfo( (Prelink::Space)(int)mListTransforms.ItemIndex );
	mListTransforms.ItemIndex = mDevice->GetSpaceInfo();
}


void PrelinkLayout::EventListSendTimeChange( HISender pSender, HKEvent pEvent )
{
	mDevice->SetClockInfo( (Prelink::Clock)(int)mListSendTime.ItemIndex );
	mListSendTime.ItemIndex = mDevice->GetClockInfo();
}


void PrelinkLayout::EventButtonClearAllModelsClick( HISender pSender, HKEvent pEvent )
{
	if( mDevice->Online )
	{
		mDevice->DeviceSendCommand( FBDevice::kOpStop );
	}

	mDevice->mExportModels.Clear();

	UIReset();
}


#define ANIMATED(ANIMNODE) ( ANIMNODE.IsAnimated() && ANIMNODE.GetAnimationNode()->KeyCount > 2 )

// DRIVEN?

void PrelinkLayout::EventButtonClearStaticModelsClick( HISender pSender, HKEvent pEvent )
{
	if( mDevice->Online )
	{
		mDevice->DeviceSendCommand( FBDevice::kOpStop );
	}

	int i;
	for(i=mDevice->mExportModels.GetCount()-1;i>=0;i--)
	{
		HFBModel lModel = mDevice->mExportModels[i];
		if( !( ANIMATED(lModel->Translation) ||
			   ANIMATED(lModel->Rotation) ||
			   ANIMATED(lModel->Scaling) ||
			   ANIMATED(lModel->Visibility) ) )
		{
			mDevice->mExportModels.Remove(lModel);
		}
	}

	UIReset();
}


void PrelinkLayout::EventButtonAddModelsClick( HISender pSender, HKEvent pEvent )
{
	if( mDevice->Online )
	{
		mDevice->DeviceSendCommand( FBDevice::kOpStop );
	}

	FBModelList lSelection;
	FBGetSelectedModels(lSelection);
	for(int i=0; i < lSelection.GetCount(); i++) 
	{
		HFBModel lModel = (HFBModel)lSelection[i];
		if( mDevice->mExportModels.Find( lModel ) == -1 )
		{
			mDevice->mExportModels.Add( lModel );
		}
	}

	CheckExportModels();

	UIReset();
}


void PrelinkLayout::EventButtonSubModelsClick( HISender pSender, HKEvent pEvent )
{
	if( mDevice->Online )
	{
		mDevice->DeviceSendCommand( FBDevice::kOpStop );
	}

	FBModelList lSelection;
	FBGetSelectedModels(lSelection);
	for(int i=0; i < lSelection.GetCount(); i++) 
	{
		HFBModel lModel = (HFBModel)lSelection[i];
		if( mDevice->mExportModels.Find( lModel ) != -1 )
		{
			mDevice->mExportModels.Remove( lModel );
		}
	}

	UIReset();
}


void PrelinkLayout::EventButtonSelectHilightedClick( HISender pSender, HKEvent pEvent )
{
	// clear selection
	FBModelList lSelection;
	FBGetSelectedModels(lSelection);
	for(int i=0; i < lSelection.GetCount(); i++)
	{
		HFBModel lModel = (HFBModel)lSelection[i];
		lModel->Selected = false;
	}

	// select highlighted item
	int i = mContainerExportNames.ItemIndex;
	kReference r = mContainerExportNames.Items.GetReferenceAt(i);
	if(!r) return;
	HFBModel lModel = (HFBModel)r;
	if(!lModel) return;
	lModel->Selected = true;
}


void PrelinkLayout::EventButtonSendModelsClick( HISender pSender, HKEvent pEvent )
{
	mDevice->EnqueueSelectionUpdate();
}


void PrelinkLayout::EventButtonSendVisibilityClick( HISender pSender, HKEvent pEvent )
{
	mDevice->SetSendVisibility(mButtonSendVisibility.State != 0);
	UIReset();
}


void PrelinkLayout::EventListNumberOfThreadsChange( HISender pSender, HKEvent pEvent )
{
	mDevice->SetNumberOfThreads((int)mListNumberOfThreads.ItemIndex);
	UIReset();
}


/************************************************
 *	Tab panel change callback.
 ************************************************/
void PrelinkLayout::EventTabPanelChange( HISender pSender, HKEvent pEvent )
{
	switch( mTabPanel.ItemIndex )
	{
		case 0:	SetControl("MainLayout", mLayoutStreams			);	break;
		case 1: SetControl("MainLayout", mLayoutEvents			);	break;
		case 2:	SetControl("MainLayout", mLayoutCommunication	);	break;
	}
}


/************************************************
 *	Device status change callback.
 ************************************************/
void PrelinkLayout::EventDeviceStatusChange( HISender pSender, HKEvent pEvent )
{
	UIReset();
}


/************************************************
 *	UI Idle callback.
 ************************************************/
void PrelinkLayout::EventUIIdle( HISender pSender, HKEvent pEvent )
{
	if( mDevice->Online )
	{
		UIRefresh();
	}
}


/************************************************
 *	When the serial port list changes.
 ************************************************/
void PrelinkLayout::EventEditNetworkAddressChange( HISender pSender, HKEvent pEvent )
{
	if( mDevice->Online )
	{
		mDevice->DeviceSendCommand( FBDevice::kOpStop );
	}

	mDevice->SetNetworkAddress((char*)(mEditNetworkAddress.Text.AsString()));
	mEditNetworkAddress.Text = mDevice->GetNetworkAddress();
}


/************************************************
 *	Server port change callback.
 ************************************************/
void PrelinkLayout::EventEditNetworkPortChange( HISender pSender, HKEvent pEvent )
{
	if( mDevice->Online )
	{
		mDevice->DeviceSendCommand( FBDevice::kOpStop );
	}

	mDevice->SetNetworkPort( mEditNetworkPort.Value );
	mEditNetworkPort.Value = mDevice->GetNetworkPort();
}

