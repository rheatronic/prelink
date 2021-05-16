#ifndef _SERVER_H_
#define _SERVER_H_

// following causes include windows.h to not bring in winsock (old version)
//#ifndef WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN
//#endif

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "spslogger.h"

class WS2
{
public:
	static bool Initialize();
	static bool Shutdown();
private:
	WS2(const WS2&);
};

class Server
{
public:
	Server(int port);
	bool Open();
	void Close();
	virtual bool Poll(struct timeval *timeout);	// NULL would mean block
	virtual bool Accept(SOCKET s) = 0;			// specialized in derivations by type of connection
private:
	Server(const Server&) {}
	int _port;
	SOCKET _socket;
};

class Connection
{
public:
	Connection(SOCKET s);
	virtual ~Connection() { closesocket(_socket); }
	bool Poll();
	bool operator<(Connection &other) { return _socket < other._socket; }
	SPSLogger mReadSPS, mWriteSPS;
protected:
	virtual bool Read() = 0;
	virtual bool Write() = 0;
	SOCKET _socket;
private:
	Connection(const Connection&);
};

#include <set>

typedef std::set<Connection*> ConnectionSet;

class ConnectionServer : public Server 
{
public:
	ConnectionServer(int port) : Server(port) {}
	virtual bool Poll(struct timeval *timeout);
	ConnectionSet _connectionSet;
protected:
	void PollConnections();
private:
	ConnectionServer(const ConnectionServer&);
};

#endif
