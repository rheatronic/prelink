// test_libprelink.cpp : Defines the entry point for the console application.
//

#include "bitflix/bitflix_debug.h"
#include "bitflix/bitflix_math.h"

#include "libprelink/prelink_client.h"

// simulated model node with name and transform matrix
struct MyModel {
	std::string mName;
	bool mVisibility;
	BFMatrix44 mMatrix;
};

// example wrapper for snapshot of MyModel
// throws TransformAccessorException on problems in conversion
class MyModelState : public TransformState {
public:
	MyModelState(const MyModel &model) : mModel(model) {}
	virtual void precheck() {}
	virtual void getName(std::string &name) { name = mModel.mName; }
	virtual void getVisibility(bool &visibility) { visibility = mModel.mVisibility; }
	virtual void getMatrix(BFMatrix44 &matrix) { matrix = mModel.mMatrix; }

protected:
	MyModel mModel;
};

// 

int main(int argc, char* argv[])
{
	// start a prelink server, local host, port 4444, in test mode (detects and checks frames 0 and 1 below)

	WSADATA wsaData;
	_ASSERT( !WSAStartup(MAKEWORD(2,2), &wsaData) );

	PrelinkClient mClient;

	mClient.SetNetworkAddress( "127.0.0.1" );
	mClient.SetNetworkPort( 4444 );

	_ASSERT(mClient.Open());

	FramedState *f = new FramedState(BFTimecode(0, 0, 0, 0, 24.0));

	MyModel a, b;
	
	a.mName = "A";
	a.mVisibility = true;

	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
			a.mMatrix.m[i][j] = (double)i+j;

	a.mName = "B";
	a.mVisibility = false;

	f->mStates.push_back(new MyModelState(a));
	f->mStates.push_back(new MyModelState(b));
	
	_ASSERT( mClient.Send(f) );

	f->mTime.frame = 1;

	_ASSERT( mClient.Send(f) );

	//
	
	_ASSERT( mClient.Close() );

	_ASSERT( !WSACleanup() );
}
