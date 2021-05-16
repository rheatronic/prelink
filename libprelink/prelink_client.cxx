/*
 Prelink (C) 2011 by bitflix.  All rights reserved.
*/

/**	\file	prelink_client.cxx
*	PrelinkClient class, network output to Previzion.
*/

#include <strstream>

//--- Class declaration
#include "prelink_client.h"

#if defined( KARCH_ENV_WIN )
#include <winsock2.h>
#endif

#if defined( KARCH_ENV_UNIX )
#include <netinet/in.h>
#endif

//

PrelinkClient::PrelinkClient()
{
	mState				= kUninitialized;
	mNetworkAddress		= DEFAULT_PRELINK_ADDRESS;
	mNetworkPort		= DEFAULT_PRELINK_PORT;
}


PrelinkClient::~PrelinkClient()
{
}


bool PrelinkClient::Open()
{
	if( mState == kOpen )
	{
		BFTrace("Already open\n");
		return true;
	}

	BFTrace("Open %s %d\n", mNetworkAddress.c_str(), mNetworkPort);

	if( !mTCPIP.CreateSocket( mNetworkSocket, SOCK_STREAM ))
	{
		mState = kError;
		return false;
	}

	if( !mTCPIP.Connect( mNetworkSocket, mNetworkAddress.c_str(), mNetworkPort ) )
	{
		// What's the best way to clear the error?
		mTCPIP.CloseSocket( mNetworkSocket );

		mState = kError;
		return false;
	}

	mState = kOpen;
	return true;
}


bool PrelinkClient::IsConnected()
{
	return mState == kOpen;
}


bool PrelinkClient::Close()
{
	if( mState != kOpen )
	{
		BFTrace("Not open\n");
		return true;
	}

	if( mTCPIP.CloseSocket(mNetworkSocket) )
	{
		mState = kClosed;
		return true;
	}
	else
	{
		mState = kError;
		return false;
	}
}


// process one tfs in a worker thread
void Process( std::ostrstream &out, TransformState *tfs)
{
	try 
	{
		tfs->precheck();
	}
	catch (TransformException& )
	{
		// tag tfs with error
		// put in error deque
		return;
	}

	std::string lName;
	tfs->getName(lName);
	out << "<node id=\"" << lName << "\">";
	{
		//
		bool lVisible;
		tfs->getVisibility(lVisible);
		out << "<property id=\"visible\">" << ( lVisible ? "true" : "false" ) << "</property>";

		BFMatrix44 lMatrix;
		tfs->getMatrix(lMatrix);
		{
			out << "<matrix>";
			for(int i=0; i<4; i++)
				for(int j=0; j<4; j++)
					out << lMatrix.m[j][i] << " ";
			out << "</matrix>";
		}
	}
	out << "</node>";
}


bool PrelinkClient::Send(FramedState *pState)
{
	std::ostrstream out;

	// Packet & Animation contexts
	out << "<previzion><animation>";
	
	// Frame context
	out << "<frame"; if(pState->mHasTime) out << " timecode=\"" << pState->mTime << "\""; out << ">";

	// States
	for(std::size_t i = 0; i < pState->mStates.size(); i++)
		Process( out, pState->mStates[i] );

	/*
	{
		// compute the transforms in parallel
		Concurrency::concurrent_vector<Packet*> results(nModels);
		Concurrency::parallel_for(0, nModels, 
			[=, &pModelList, &results](int i)
			{
				Packet *p = new Packet;
				if( Process( *p, pModelList.GetAt( i ) ) )
					results[i] = p;
				else
				{
					delete p;
					results[i] = 0;
				}
			}
		);

		// assemble thread elements into pPacket
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
	} */

	out << "</frame>";

	out << "</animation></previzion>\0\0";

	out << std::ends; std::string lXML = out.str(); out.freeze(false);

	int lBytesWritten;
	mTCPIP.Write(mNetworkSocket, lXML.c_str(), (int)lXML.size(), lBytesWritten);
	if ((int)lXML.size() != lBytesWritten) 
	{
		BFTrace("Bad send: Packet size %d but wrote %d\n", (int)lXML.size(), lBytesWritten);
		return false;
	}

	return true;
}

bool PrelinkClient::Receive()
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

	PrevizionUpdate lRecord;
	mStatusStream.ParseBuffer( lRecord, lpBuffer, lpNumberOfBytesRead );
	for(auto i = lRecord.begin(); i != lRecord.end(); i++)
	{
		PrevizionFrame* lFrame = *i;
		for(auto j = lFrame->mStates.begin(); j != lFrame->mStates.end(); j++)
		{
			PrevizionStatus* lStatus = *j;

			// one clause for each possible type
			CameraStatus* lCameraStatus = dynamic_cast<CameraStatus*>(lStatus);
			if( lCameraStatus )
			{
				mStatus = *lCameraStatus;
				mCounter++;
				lSuccess = true;
			}
			
			delete lStatus;
		}
		delete lFrame;
	}

	return lSuccess;
}
