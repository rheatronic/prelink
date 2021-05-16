#ifndef _PRELINK_SERVER_H
#define _PRELINK_SERVER_H

// TCP/IP server specialized for Prelink connections

#include "server.h"
#include "camera.h"
#include <iostream>
#include <fstream>
#include "prelink_message_stream.h"

class PrelinkConnection : public Connection
{
public:
	PrelinkConnection(SOCKET s, const char* path=0) : _pms(s), Connection(s), _sendCamera(false) { if(path) LogToFile(path); }
	virtual ~PrelinkConnection() { StopLogging(); }
	virtual bool Read();
	virtual bool Write();

	virtual void LogToFile(const char* path);
	virtual void StopLogging();
protected:
	virtual Camera* getCamera() = 0;
private:
	PrelinkConnection(const PrelinkConnection&);
	bool _sendCamera;
	std::ofstream _logStream;
	PrelinkMessageStream _pms;
};

#endif
