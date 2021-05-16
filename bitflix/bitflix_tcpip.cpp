#include "bitflix_tcpip.h"
#include <WS2tcpip.h>
#include <iostream>

BFTCPIP::BFTCPIP() {
}

BFTCPIP::~BFTCPIP() {
}

bool BFTCPIP::Initialize() {
	WSADATA wsaData;
	int error = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (error) {
        return false; // Couldn't start Winsock
    }

	// Wrong Winsock version?
    if (wsaData.wVersion != MAKEWORD(2,2)) {
        WSACleanup();
        return false;
    }

	return true;
}

bool BFTCPIP::Shutdown() {
	return WSACleanup() != SOCKET_ERROR;
}

bool BFTCPIP::CreateSocket(SOCKET& networkSocket, int socketType) {
	networkSocket = socket(AF_UNSPEC, socketType, IPPROTO_TCP);
	if (networkSocket == INVALID_SOCKET) {
		std::cerr << "socket create error " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

bool BFTCPIP::Connect(SOCKET& networkSocket, const char* networkAddr, unsigned short networkPort) {
	struct addrinfo *result = NULL,
					*ptr = NULL,
					hints;

	// Type of socket requested
	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	char portString[8];
	_itoa_s(networkPort, portString, 10);

	// Resolve the server address and port
	int iResult = getaddrinfo(networkAddr, portString, &hints, &result);
	if (iResult != 0) {
		std::cerr << "getaddrinfo error " << WSAGetLastError() << std::endl;
		return false;
	}

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	iResult = connect(networkSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(networkSocket);
		networkSocket = INVALID_SOCKET;
		std::cerr << "connect error " << WSAGetLastError() << std::endl;
	} else {
		BOOL optionValue = true;
		if(setsockopt(networkSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&optionValue, sizeof(BOOL)) == SOCKET_ERROR)
		{
			std::cerr << "could not set NODELAY error " << WSAGetLastError() << std::endl;
			// Note: we don't report a failure here
		}
	}

	freeaddrinfo(result);

	return networkSocket != INVALID_SOCKET;
}

bool BFTCPIP::CloseSocket(SOCKET networkSocket) {
	// shutdown the connection for sending since no more data will be sent
	// the client could still use the socket for receiving data
	if (shutdown(networkSocket, SD_SEND) == SOCKET_ERROR) {
		std::cerr << "shutdown SD_SEND failed: " << WSAGetLastError() << std::endl;
		if(closesocket(networkSocket) == SOCKET_ERROR) {
			std::cerr << "closesocket failed: " << WSAGetLastError() << std::endl;
		}
		return false;
	}

	// cleanup
	if( closesocket(networkSocket) == SOCKET_ERROR) {
		std::cerr << "closesocket FAILED: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

bool BFTCPIP::Select(SOCKET& socket, bool testRead, bool testWrite, bool testError, unsigned long timeoutMilliseconds) {
	fd_set _readS, _writeS, _errS;

	FD_ZERO(&_readS);
	FD_ZERO(&_writeS);
	FD_ZERO(&_errS);

	if(testRead)	FD_SET(socket, &_readS);
	if(testWrite)	FD_SET(socket, &_writeS);
	if(testError)	FD_SET(socket, &_errS);

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = timeoutMilliseconds * 1000L;

	int ready = select(0, &_readS, &_writeS, &_errS, &timeout);
	if( ready == SOCKET_ERROR )
	{
		std::cerr << "select socket error " << WSAGetLastError() << std::endl;
		return false;
	}

	return ready > 0;
}

bool BFTCPIP::Read(SOCKET& socket, char* buffer, int bufferLen, int &bytesRead) {
    bytesRead = recv(socket, buffer, bufferLen, 0);
    if (bytesRead > 0)
	{
		return true;
	}
	else if (bytesRead == 0)
	{
        std::cerr << "Connection closed" << std::endl;
		return false;
	}
    else
	{
        std::cerr << "recv failed: " <<  WSAGetLastError() << std::endl;
		return false;
	}
}

bool BFTCPIP::Write(SOCKET networkSocket, const char* buffer, int bytesToWrite, int &bytesWritten) {
	// Send a buffer
	bytesWritten = send(networkSocket, buffer, bytesToWrite, 0);
	if (bytesWritten == SOCKET_ERROR) {
		std::cerr << "send error " << WSAGetLastError() << std::endl;
		// If the send failed we can't do anything with the socket
		//closesocket(_socket);
		return false;
	}

	return true;
}

int BFTCPIP::GetLastError() {
	return WSAGetLastError(); 
}
