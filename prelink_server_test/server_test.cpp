// server_test.cpp : test the server classes

//#define FAST_AS_POSSIBLE 1

#include "prelink_server.h"
#include <iostream>

class TestPrelinkServer : public ConnectionServer
{
public:
	TestPrelinkServer(int port, const char* logPath) : ConnectionServer(port), _camera(0), _logPath(logPath) {}
	void setCamera(Camera* camera) { _camera = camera; }
	Camera* getCamera() { return _camera; }
	virtual bool Accept(SOCKET s);
private:
	TestPrelinkServer(const TestPrelinkServer&);
	Camera* _camera;
	const char* _logPath;
};

class TestPrelinkConnection : public PrelinkConnection
{
public:
	TestPrelinkConnection(TestPrelinkServer* server, SOCKET s, const char* path=0) : PrelinkConnection(s, path), _server(server) {}
	virtual ~TestPrelinkConnection();
protected:
	virtual Camera* getCamera() { return _server->getCamera(); }
	TestPrelinkServer* _server;
};

TestPrelinkConnection::~TestPrelinkConnection()
{
	std::cout << "Client connection closed on socket " << _socket << std::endl;
}

bool TestPrelinkServer::Accept(SOCKET s)
{
	std::cout << "Accepting client connection on socket " << s << std::endl;
	_connectionSet.insert(new TestPrelinkConnection(this, s, _logPath));
	return true;
}

#define PORT 4444
#define FRAMERATE 29.97f

class TestCamera : public Camera
{
public:
	TestCamera();
	virtual bool validTimecode() { return true; }
	virtual Timecode getTimecode() { return _tc; }
	virtual float getFramerate() { return FRAMERATE; }
	virtual Vector3 getPosition() { return Vector3(0.1234f, 1.1234f, 2.1234f); }
	virtual Vector3 getUp()  { return Vector3(0.0f, 1.0f, 0.0f); }
	virtual Vector3 getForward();
	virtual Vector2 getFOV()  { return Vector2(80.0f, 45.0f); }	// 16:9
	virtual float getFocusDistance() { return 200.3f; }
	virtual float getIris() { return 4.5f; }
	virtual float getSubjectDistance() { return 250.0f; }
	virtual float getDistortionR2() { return 0.0f; }
	virtual float getInteraxial() { return 2.0f; }
	virtual float getConvergence() { return 0.0f; }

	void frameFWD();

protected:
	Timecode _tc;
	unsigned int _spin;
};

TestCamera::TestCamera()
	: _tc(10, 59, 59, 0, FRAMERATE), _spin(0)
{
}

#define M_PI 3.141592654

 Vector3 TestCamera::getForward()
{
	float radians = (float)((_spin % (360 * 5)) / 5.0 * M_PI / 180.0);
	return Vector3(cos(radians), 0.0f, sin(radians));
}

void TestCamera::frameFWD()
{
	_tc = _tc + Timecode(0, 0, 0, 1, FRAMERATE);
	_spin++;
}

// -----

int main(int argc, char* argv[])
{
#ifdef FAST_AS_POSSIBLE
	std::cout << "FAST AS POSSIBLE" << std::endl;
#endif

	bool stats = true;
	int port = PORT;
	float rate = FRAMERATE;
	const char* logPath = 0;

	if( argc == 1 )
		;
	else if( argc == 4 )
	{
		port = atoi(argv[1]);
		rate = (float)atof(argv[2]);
		logPath = argv[3];
	}
	else
	{
		std::cerr << "Usage: " << argv[0] << " [<port> <framerate> <incoming packet log path>]" << std::endl;
		exit(-1);
	}

	std::cout << "server_test starting on port " << port << " at rate " << rate << std::endl;

	if(!WS2::Initialize())
		return -1;
	std::cout << "WS2 initialized\n";

	TestPrelinkServer server(port, logPath);
	TestCamera* cam = new TestCamera();
	server.setCamera(cam);
	std::cout << "server made\n";
	
	if(!server.Open())
		return -2;
	std::cout << "server opened\n";

	// what we need
	struct timeval frame;
	frame.tv_sec = 0;
	frame.tv_usec = (long)(1000000.0 / (double)rate);
	std::cout << "frame " << frame.tv_usec << " usec" << std::endl;

	// adjusted to latency
	struct timeval timeout = frame;

	// loop duration to correct clock
	SPSLogger duration((int)ceil(FRAMERATE));
	long delta = 0;
	while(server.Poll(&timeout))
	{
		if (stats) {
			std::cout << cam->getTimecode() << " ";
			for(ConnectionSet::iterator i = server._connectionSet.begin(); i != server._connectionSet.end(); ++i)	{
				Connection *c = *i;
				float sps, usec;
				c->mReadSPS.GetStats(sps, usec);
				std::cout << sps << " " << usec << " " << delta;
			}
			std::cout << "     \r";
		}

		cam->frameFWD();

		//
		duration.Sample();
		float ignore, usec;
		duration.GetStats(ignore, usec);

		if(usec > 1.0f) 
		{
			// reduce timeout to dynamically correct the clock
			long diff = (long)usec - frame.tv_usec;
			delta += diff/8;	// adjust the clock slowly
			timeout.tv_usec = frame.tv_usec - delta;
		}

#ifdef FAST_AS_POSSIBLE
		timeout.tv_usec = 0L;
#endif
	}

	return 0;
}
