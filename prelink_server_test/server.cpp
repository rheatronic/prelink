// polling version
// call Initialize, then Poll at intervals

#include "server.h"
#include <iostream>

bool WS2::Initialize()
{
	WSADATA wsaData;
	int error = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (error)
    {
		std::cerr << "could not start winsock " << WSAGetLastError() << std::endl;
        return false;

    }

    if (wsaData.wVersion != MAKEWORD(2,2)) // Wrong Winsock version?
    {
        WSACleanup();
        return false;
    }

	return true;
}

bool WS2::Shutdown()
{
	return WSACleanup() != SOCKET_ERROR;
}


// -----


Server::Server(int port)
	: _port(port), _socket(INVALID_SOCKET)
{
}

bool Server::Open()
{
    SOCKADDR_IN addr; // The address structure for a TCP socket
    addr.sin_family = AF_INET;      // Address family
    addr.sin_port = htons (_port);   // Assign port to this socket

    //Accept a connection from any IP using INADDR_ANY
    //You could pass inet_addr("0.0.0.0") instead to accomplish the 
    //same thing. If you want only to watch for a connection from a 
    //specific IP, specify that //instead.
    addr.sin_addr.s_addr = htonl (INADDR_ANY);  
    _socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
    if (_socket == INVALID_SOCKET)
    {
		std::cerr << "error trying to create socket " << WSAGetLastError() << std::endl;
        return false;
    }

    if (bind(_socket, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
       //We couldn't bind (this will happen if you try to bind to the same  
       //socket more than once)
		std::cerr << "error trying to bind " << WSAGetLastError() << std::endl;
        return false;
    }

	if( listen(_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "error trying to listen " << WSAGetLastError() << std::endl;
		return false;
	}

	BOOL falseValue = true;
	if(setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&falseValue, sizeof(BOOL)) == SOCKET_ERROR)
		std::cerr << "could not set NODELAY error " << WSAGetLastError() << std::endl;

	return true;
}

void Server::Close()
{
	if(_socket != INVALID_SOCKET)
		closesocket(_socket);
}

bool Server::Poll(struct timeval *timeout)
{
	fd_set _readS, _errS;
	FD_ZERO(&_readS);
	FD_SET(_socket, &_readS);
	FD_ZERO(&_errS);
	FD_SET(_socket, &_errS);

	int result = select(0, &_readS, 0, &_errS, timeout);
	if( result < 0 )
	{
		// err 
		std::cerr << "select error" << std::endl;
		return false;
	} else if( result == 0 )
	{
		return true;
	}

	if( FD_ISSET(_socket, &_errS) )
	{
		// pending error
		std::cerr << "error on socket" << std::endl;
		return false;
	}

	if( FD_ISSET(_socket, &_readS) )
	{
		// pending connection
		SOCKET c = accept(_socket, NULL, NULL);
		if (c == INVALID_SOCKET)
		{
			// report
			std::cerr << "could not accept connection" << std::endl;
		}
		else
			Accept(c);
	}
	return true;
}


// -----


Connection::Connection(SOCKET s)
	: _socket(s), mReadSPS(60), mWriteSPS(60)
{
}


bool Connection::Poll()
{
	fd_set _readS, _writeS, _errS;
	FD_ZERO(&_readS);
	FD_SET(_socket, &_readS);
	FD_ZERO(&_writeS);
	FD_SET(_socket, &_writeS);
	FD_ZERO(&_errS);
	FD_SET(_socket, &_errS);

	// connections must poll fast as possible
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	
	int result = select(0, &_readS, &_writeS, &_errS, &timeout);
	if( result < 0 )
	{
		// err 
		return false;
	} else if( result == 0 )
	{
		// nobody ready
		return true;
	}

	if( FD_ISSET(_socket, &_errS) )
	{
		// pending error
		return false;
	}

	bool keepOpen = true;

	if(  FD_ISSET(_socket, &_writeS) )
	{
		// ready to write (if needed)
		keepOpen = Write();
	}

	if( FD_ISSET(_socket, &_readS) )
	{
		// data ready to read
		keepOpen &= Read();
	}

	return keepOpen;
}


// -----

bool ConnectionServer::Poll(struct timeval *timeout)
{
	PollConnections();
	return Server::Poll(timeout);
}

void ConnectionServer::PollConnections()
{
	ConnectionSet revised;
	ConnectionSet closed;

	// poll & note condition of connection
	for(ConnectionSet::iterator i = _connectionSet.begin(); i != _connectionSet.end(); ++i)
	{
		Connection *c = *i;
		if( c->Poll() )
			revised.insert(c);
		else
			closed.insert(c);
	}

	// reduce to remaining active connections
	_connectionSet = revised;

	// close and delete connections from last round
	for(ConnectionSet::iterator j = closed.begin(); j != closed.end(); j++)
	{
		Connection *c = *j;
		delete c;
	}
}
