// Prelink server connection set node for Maya
// Copyright 2011 bitflix LLC All Rights Reserved

#include <maya/MPxObjectSet.h> 
#include <maya/MString.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnPlugin.h>
#include <api_macros.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnMatrixData.h>

#include <maya/MGlobal.h>
#include <maya/MEventMessage.h>
#include <maya/MNodeMessage.h>
#include <maya/MDGMessage.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>
#include <maya/MObjectHandle.h>

#include <map>

#include "bitflix/bitflix_debug.h"
#include "prelinkConnector.h"

//

#define	PLUGIN_VENDOR "bitflix"
#define PLUGIN_VERSION "0.2.4"

//      Check if the given object is visible
bool isObjectVisible( const MFnDagNode & fnDN )
{
    // Check the visibility attribute of the node
    //
    MPlug vPlug = fnDN.findPlug( "visibility" );
    // Also check to see if the node is an intermediate object in
    // a computation.  For example, it could be in the middle of a
    // chain of deformations.  Intermediate objects are not visible.
    //
    MPlug iPlug = fnDN.findPlug( "intermediateObject" );

    bool visible, intermediate;
    vPlug.getValue( visible );
    iPlug.getValue( intermediate );

    return  visible && !intermediate;
}

// map for MObjectHandle hashCode to MCallbackId

typedef std::map<unsigned int,MCallbackId> MObjHashCBIDMap;

class prelinkNode : public MPxObjectSet
{
public:
							prelinkNode();
	virtual					~prelinkNode(); 
	virtual void			postConstructor();

	virtual MStatus			legalConnection( const MPlug& plug,
											 const MPlug& otherPlug,
											 bool asSrc,
											 bool& isLegal ) const;

	virtual MStatus			compute( const MPlug &  plug, 
									MDataBlock &  block );

	//
	void					addAttrChangeCallback( MObject& obj );
	void					removeAttrChangeCallback( MObject& obj );
	void					clearAttrChangeCallbacks();

	virtual bool			canBeDeleted( bool isSrcNode ) { return false; }	// Don't delete node when neighbors go

	//

	static  void *          creator();
	static  MStatus         initialize();

	static	MObject			serverAddress;
	static	MObject			serverPort;
	static  MObject         serverOpen;
	static  MObject         serverConnected;

protected:
	// Callback functions for scene and node messages
	static void				prelinkNodeAttrChangeCB( MNodeMessage::AttributeMessage msg, MPlug & plug,
										MPlug & otherPlug, void* );
	static void				dgRemoveCB( MObject& node, void *clientData );
	static void				timeChangeCB( MTime &time, void* );
	static void				attrChangeCB( MNodeMessage::AttributeMessage msg, MPlug & plug,
											MPlug & otherPlug, void* );

	PrelinkClient			prelinkClient;
	MCallbackId				dgRemoveCallbackId;
	MCallbackId				timeChangeCallbackId;
	MCallbackId				prelinkNodeAttrChangeCallbackId;
	MObjHashCBIDMap			objectAttrChangeCallbackIds;

	bool					printTimes;

public: 
	static	MTypeId		id;
};

//

MTypeId prelinkNode::id( 0x80AF7 );
MObject prelinkNode::serverAddress;
MObject prelinkNode::serverPort;
MObject prelinkNode::serverOpen;
MObject prelinkNode::serverConnected;

prelinkNode::prelinkNode() : prelinkNodeAttrChangeCallbackId( NULL ), printTimes(true) {}

prelinkNode::~prelinkNode() 
{
	MStatus status;

	status = MDGMessage::removeCallback( dgRemoveCallbackId );
	if ( status == MS::kSuccess ) {
		dgRemoveCallbackId = 0;
	} else
		MGlobal::displayError("prelinkNode::~prelinkNode - could not remove dg remove callback");
	
	status = MDGMessage::removeCallback( timeChangeCallbackId );
	if ( status == MS::kSuccess ) {
		timeChangeCallbackId = 0;
	} else
		MGlobal::displayError("prelinkNode::~prelinkNode - could not remove time change callback");

	status = MEventMessage::removeCallback( prelinkNodeAttrChangeCallbackId );
	if ( status == MS::kSuccess ) {
		prelinkNodeAttrChangeCallbackId = 0;
	} else
		MGlobal::displayError("prelinkNode::~prelinkNode - could not remove prelinkNode attr change callback");

	clearAttrChangeCallbacks();
}

void prelinkNode::postConstructor()
{
	MStatus status;
	MObject thisObj = thisMObject();
	
	prelinkNodeAttrChangeCallbackId = MNodeMessage::addAttributeChangedCallback(
		thisObj,
		prelinkNode::prelinkNodeAttrChangeCB,
		(void *)(size_t)this,
		&status );
	if ( status != MS::kSuccess )
		MGlobal::displayError("prelinkNode::postConstructor - could not add prelinkNodeAttr change callback");

	timeChangeCallbackId = MDGMessage::addTimeChangeCallback( 
		prelinkNode::timeChangeCB, 
		(void *)(size_t)this,
		&status );
	if( status != MS::kSuccess )
		MGlobal::displayError("prelinkNode::postConstructor - could not add time change callback");

	dgRemoveCallbackId = MDGMessage::addNodeRemovedCallback( 
		prelinkNode::dgRemoveCB, 
		"prelinkNode",
		(void *)(size_t)this,
		&status );
	if( status != MS::kSuccess )
		MGlobal::displayError("prelinkNode::postConstructor - could not add node remove callback");
}

MStatus prelinkNode::legalConnection( const MPlug& plug,
									  const MPlug& otherPlug,
									  bool asSrc,
									  bool& isLegal ) const
{
	// if asSrc, pass
	// else it's an incoming connection
	// disallow connection to dagSetMembers
	// disallow connection to instObjGroups
	// only allow matrix attrs to be connected to dnSetMembers
	// (but allow connections to other attrs)
	return MStatus::kUnknownParameter;
}

MStatus prelinkNode::compute( const MPlug& plug, 
							  MDataBlock& data )
{
	if( plug.partialName() == "sc" )
	{
		MDataHandle outputHandle = data.outputValue( plug );
		bool connected = prelinkClient.IsConnected();
		outputHandle.set(connected);
        data.setClean(plug);
		return MStatus::kSuccess;
	}
	else
	{
		return MStatus::kUnknownParameter;
	}
}

void prelinkNode::addAttrChangeCallback( MObject &obj )
{
	MStatus status;
	
	MCallbackId cbID = MNodeMessage::addAttributeChangedCallback(
		obj, prelinkNode::attrChangeCB, (void *)(size_t)this, &status );
	
	MFnDagNode dgFn( obj );
	MString info = "prelinkNode::addAttrChangeCallback - ";
	info += dgFn.name();
	if ( status == MS::kSuccess )
	{
		MObjectHandle objH(obj);
		objectAttrChangeCallbackIds[objH.hashCode()] = cbID;
	} else {
		 info += " could not add matrix node attr change callback";
		MGlobal::displayError( info );
	}
}

void prelinkNode::removeAttrChangeCallback( MObject &obj )
{
	MFnDagNode dgFn( obj );
	MString info = "prelinkNode::removeAttrChangeCallback - ";
	info += dgFn.name();

	MObjectHandle objH(obj);
	MCallbackId id = objectAttrChangeCallbackIds[objH.hashCode()];
	MStatus status = MMessage::removeCallback( id );
	
	if ( status == MS::kSuccess )
	{
		MObjectHandle objH(obj);
		objectAttrChangeCallbackIds.erase(objH.hashCode());
	} else {
		 info += " could not remove matrix node attr change callback";
		MGlobal::displayError( info );
	}
}

void prelinkNode::clearAttrChangeCallbacks()
{
	for( MObjHashCBIDMap::iterator it = objectAttrChangeCallbackIds.begin(); 
		it != objectAttrChangeCallbackIds.end(); 
		it++ )
		if( MMessage::removeCallback( (*it).second ) != MS::kSuccess )
			MGlobal::displayError( "prelinkNode::clearAttrChangeCallbacks - could not remove attr change callback" );
	objectAttrChangeCallbackIds.clear();
}

//
// Callback function
// We're only called on (any) attr change for prelinkNode
// Test for dnSetMembers (to add nodes we're monitoring)
// ...or changes to the serverAddress, serverPort or serverConnect attrs
//

void prelinkNode::prelinkNodeAttrChangeCB( 
	MNodeMessage::AttributeMessage msg, 
	MPlug &plug,
	MPlug &otherPlug,
	void* data )
{
	prelinkNode *i = (prelinkNode *)(size_t)data;
	MString plugName = plug.partialName();
	MString otherPlugName = otherPlug.partialName();

	if( plugName == "dsm" &&
		msg & MNodeMessage::kOtherPlugSet &&
		msg & MNodeMessage::kIncomingDirection &&
		otherPlugName == "iog" )
	{
		// dagSetMembers
		MObject obj = otherPlug.node();
		if ( msg & MNodeMessage::kConnectionMade ) {
			// if camera
			//   ?
			// or...
			i->addAttrChangeCallback( obj );
		} else if ( msg & MNodeMessage::kConnectionBroken ) {
			// if camera
			//   ?
			// or...
			i->removeAttrChangeCallback( obj );
		} else
			MGlobal::displayError( "prelinkNode::prelinkNodeAttrChangeCB - Other msg to dagSetMembers <- instObjGroups ignored" );
	}
	else if( plugName == "so" &&
		     msg & MNodeMessage::kAttributeSet )
	{
		bool succeed;
		// server_open
		if( plug.asBool() )
		{
			succeed = i->prelinkClient.Open();
		}
		else
		{
			succeed = i->prelinkClient.Close();
		}
	}
	else if( plugName == "sa" &&
		     msg & MNodeMessage::kAttributeSet )
	{
		// server_address
		i->prelinkClient.Close();
		i->prelinkClient.SetNetworkAddress( plug.asString().asChar() );
	}
	else if( plugName == "sp" &&
		     msg & MNodeMessage::kAttributeSet )
	{
		// server_port
		i->prelinkClient.Close();
		i->prelinkClient.SetNetworkPort( plug.asShort() );
	}
}

void prelinkNode::timeChangeCB( MTime &time, void *data )
{
	MDGContext ctx(time);

	// get the array plug
	prelinkNode *i = (prelinkNode *)(size_t)data;
	MObject node = i->thisMObject();
	MPlug arrayPlug(node, i->dagSetMembers);

#ifdef _LEAK_DEBUG
	_CrtMemState s1;
	_CrtMemCheckpoint( &s1 );
#endif

	MayaFramedState *mfs = new MayaFramedState(time);

	if(true) {
		MString msg;
		msg = "prelinkNode::timeChangeCB ";
		msg += time.asUnits(time.unit());
		msg += "\t";
		msg += mfs->mTime.hour;
		msg += ":";
		msg += mfs->mTime.minute;
		msg += ":";
		msg += mfs->mTime.second;
		msg += ":";
		msg += mfs->mTime.frame;
		msg += "\n";
		MGlobal::displayInfo( msg );
	}

	// iterate plugs and their connections
	for ( unsigned int i = 0; i < arrayPlug.numElements(); i++ ) 
	{
        MPlug childPlug = arrayPlug[i];

		MPlugArray connectedPlugs;
		childPlug.connectedTo( connectedPlugs, true, false );
		for ( unsigned int j = 0; j < connectedPlugs.length(); j++ )
		{
			MPlug connectedPlug = connectedPlugs[j];

			MObject childObj = connectedPlug.node();
			if( childObj != MObject::kNullObj ) {
				MFnDagNode dagNodeFn( childObj );
				MString nodeName = dagNodeFn.name();
				MMatrix nodeMatrix = dagNodeFn.transformationMatrix();
				bool visibility = isObjectVisible( dagNodeFn );

				MayaTransformState *mts = new MayaTransformState(nodeName, nodeMatrix, visibility);
				mfs->mStates.push_back(mts);
			}
		}
    }

	i->prelinkClient.Send(mfs);

	for( unsigned int i=0; i < mfs->mStates.size(); i++ )
		delete mfs->mStates[i];
	delete mfs; mfs = 0;

#ifdef _LEAK_DEBUG
	_CrtMemState s2;
	_CrtMemCheckpoint( &s2 );

	_CrtMemState s3;
	if ( _CrtMemDifference( &s3, &s1, &s2) )
		_CrtMemDumpStatistics( &s3 );
#endif
}

void prelinkNode::attrChangeCB( 
	MNodeMessage::AttributeMessage msg, 
	MPlug &plug,
	MPlug &otherPlug,
	void* data )
{
	// Attr change on connected node

	// If attribute affects the node...
	if( msg & MNodeMessage::kAttributeSet &&
		msg & MNodeMessage::kIncomingDirection )
	{
		// get the node
		MObject nodeObject = plug.node();
		MFnDagNode dagFn( nodeObject );

		MString nodeName = dagFn.name();

		// change is reported on translateX, etc. not the matrix
		// XXX could filter based on only those things that alter the matrix

		//

		MPlug matrixPlug(nodeObject, dagFn.attribute("m"));
		MObject matrixObject = matrixPlug.asMObject();
		MFnMatrixData fnMatrixData(matrixObject);
		MMatrix nodeMatrix = fnMatrixData.matrix();

		//
		bool visibility = isObjectVisible( dagFn );

#ifdef _LEAK_DEBUG
		_CrtMemState s1;
		_CrtMemCheckpoint( &s1 );
#endif

		//
		MayaFramedState *mfs = new MayaFramedState();
		MayaTransformState *mts = new MayaTransformState(nodeName, nodeMatrix, visibility);
		mfs->mStates.push_back(mts);

		//
		prelinkNode *i = (prelinkNode *)(size_t)data;
		i->prelinkClient.Send(mfs);

		delete mts; mts = 0;
		delete mfs; mfs = 0;

#ifdef _LEAK_DEBUG
	_CrtMemState s2;
	_CrtMemCheckpoint( &s2 );

	_CrtMemState s3;
	if ( _CrtMemDifference( &s3, &s1, &s2) )
		_CrtMemDumpStatistics( &s3 );
#endif
	}
}

void prelinkNode::dgRemoveCB(
	MObject& node,
	void* data )
{
	// cast node to MFn prelinkNode
	MStatus mStatus;
	MFnDependencyNode dgFn( node, &mStatus );
	if( mStatus.error() )
	{
		MGlobal::displayError( "prelinkNode::dgRemoveCB - got non-dependency node (not a prelinkNode)" );
		return;
	}

	// get serverOpen attr
	MPlug plug = dgFn.findPlug( prelinkNode::serverOpen, &mStatus );
	if( mStatus.error() )
	{
		MGlobal::displayError( "prelinkNode::dgRemoveCB - could not find serverOpen plug (attribute)" );
		return;
	}

	//
	mStatus = plug.setBool(false);
	if( mStatus.error() )
	{
		MGlobal::displayError( "prelinkNode::dgRemoveCB - could not set serverOpen plug to false" );
		return;
	}
}

void* prelinkNode::creator()
{
	return new prelinkNode();
}

MStatus prelinkNode::initialize()
{ 
	// this is called once at plugin load time

	// Inputs
	MString defaultAddr(DEFAULT_PRELINK_ADDRESS);
	MFnStringData defaultAddrFn;
	MObject defaultAddrObj = defaultAddrFn.create(defaultAddr);
	MAKE_TYPED_ATTR(	serverAddress, "serverAddress", "sa", MFnData::kString, defaultAddrObj );
	//serverAddress_fn.setChannelBox(true);	// this has no effect, likely because this is a string attribute
	serverAddress_fn.setKeyable(false);
	serverAddress_fn.setStorable(true);
	MAKE_NUMERIC_ATTR(	serverPort, "serverPort", "sp", MFnNumericData::kShort, DEFAULT_PRELINK_PORT, false, false, false );
	//serverPort_fn.setChannelBox(true);
	serverPort_fn.setKeyable(false);
	serverPort_fn.setStorable(true);
	MAKE_NUMERIC_ATTR(	serverOpen, "serverOpen", "so", MFnNumericData::kBoolean, 0, false, false, false );
	serverOpen_fn.setChannelBox(true);
	serverOpen_fn.setKeyable(true);
	serverOpen_fn.setStorable(false);

	// Outputs
	MStatus status;
	MFnNumericAttribute serverConnected_fn;
	serverConnected = serverConnected_fn.create( "serverConnected", "sc", MFnNumericData::kBoolean, 0 );
	MCHECKERROR(status,"create serverConnected attr");
	serverConnected_fn.setHidden(false);
	serverConnected_fn.setKeyable(false);
	serverConnected_fn.setReadable(true);
	status = addAttribute(serverConnected);
	MCHECKERROR(status,"add attribute error");
	serverConnected_fn.setChannelBox(true);

	// affects
	attributeAffects( serverAddress, serverConnected );
	attributeAffects( serverPort, serverConnected );
	attributeAffects( serverOpen, serverConnected );

	return MS::kSuccess;
}

// -----
// Plugin support
// -----

MStatus initializePlugin( MObject obj )
{ 
#ifdef _LEAK_DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_MAP_ALLOC | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_CRT_DF );
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDERR );
#endif

	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_VENDOR, PLUGIN_VERSION, "Any" );

	status = plugin.registerNode( "prelinkNode", prelinkNode::id, 
						 &prelinkNode::creator, &prelinkNode::initialize,
						 MPxNode::kObjectSet );
	if (!status) {
		status.perror( "prelinkNode initializePlugin registerNode" );
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterNode( prelinkNode::id );
	if (!status) {
		status.perror( "prelinkNode uninitializePlugin deregisterNode" );
		return status;
	}

	return status;
}
