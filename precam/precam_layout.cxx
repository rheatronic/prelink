//--- Class declarations
#include "precam_layout.h"

//--- Registration defines
#define	PRECAM__LAYOUT		Precam_Layout

//--- FiLMBOX implementation and registration
FBDeviceLayoutImplementation(	PRECAM__LAYOUT		);
FBRegisterDeviceLayout		(	PRECAM__LAYOUT,
								PRECAM__CLASSSTR,
								FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX constructor/destructor.
 ************************************************/
bool Precam_Layout::FBCreate()
{
	mCameraDevice		= ((Precam *)(FBDevice *)Device);
	SupportedCommTypes	= kFBCommTypeNetworkTCP;
	CustomLayout		= false;
	
	if( FBDeviceCameraLayout::FBCreate() )
	{
		return true;
	}
	return false;
}
void Precam_Layout::FBDestroy()
{
	FBDeviceCameraLayout::FBDestroy();
}

/************************************************
 *	UI configuration.
 ************************************************/
void Precam_Layout::UIConfigure()
{
	mLayoutCommunication.RemoveRegion("RegionSync");
	mLayoutCommunication.ClearControl("LabelSyncDelay");
	mLayoutCommunication.ClearControl("SliderSyncDelay");
	mLayoutCommunication.ClearControl("EditNumberSyncDelay");
	mLayoutCommunication.ClearControl("ButtonSyncInfo");

	mLayoutCommunication.AddRegion("RegionSync", "RegionSync",
		0,		kFBAttachLeft,		"",		0.00,
		0,		kFBAttachTop,		"",		0.00,
		0,		kFBAttachNone,		NULL,	0.00,
		0,		kFBAttachNone,		NULL,	0.00 );

	FBDeviceCameraLayout::UIConfigure();
}

void Precam_Layout::UICreateLayoutCustom()
{
	FBDeviceCameraLayout::UICreateLayoutCustom();
}

void Precam_Layout::UIConfigureLayoutCustom()
{
	FBDeviceCameraLayout::UIConfigureLayoutCustom();
}

void Precam_Layout::UIRefreshLayoutCustom()
{
	FBDeviceCameraLayout::UIRefreshLayoutCustom();
}

void Precam_Layout::UIResetLayoutCustom()
{
	FBDeviceCameraLayout::UIResetLayoutCustom();
}

void Precam_Layout::UIUpdateEnabled()
{
	FBDeviceCameraLayout::UIUpdateEnabled();
}

/************************************************
 *	Communications: Network
 ************************************************/
void Precam_Layout::OnNetworkChange( HISender pSender, HKEvent pEvent )
{
	FBDeviceCameraLayout::OnNetworkChange( pSender, pEvent );
	char lStr[255];
	sprintf( lStr, "%d", mCameraDevice->GetNetworkPort() );

	mEditNetworkAddress.Text		= mCameraDevice->GetNetworkAddress();
	mEditNetworkPort.Text			= lStr;
	mListNetworkProtocol.ItemIndex	= kFBCommTypeNetworkTCP;
}

void Precam_Layout::OnNetworkAddressChange( HISender pSender, HKEvent pEvent )
{
	FBDeviceCameraLayout::OnNetworkAddressChange( pSender, pEvent );
	mCameraDevice->SetNetworkAddress( mEditNetworkAddress.Text );
	OnNetworkChange(pSender,pEvent);
}

void Precam_Layout::OnNetworkPortChange( HISender pSender, HKEvent pEvent )
{
	FBDeviceCameraLayout::OnNetworkPortChange( pSender, pEvent );
	mCameraDevice->SetNetworkPort( atoi((const char*)mEditNetworkPort.Text) );
	OnNetworkChange(pSender,pEvent);
}
