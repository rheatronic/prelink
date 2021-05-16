#ifndef _BITFLIX_TCPIP_H_
#define _BITFLIX_TCPIP_H_

#include "bitflix/bitflix_debug.h"

#include <WinSock2.h>

class __declspec( dllexport ) BFTCPIP {
public:
	static bool Initialize();
	static bool Shutdown();

	BFTCPIP();
	~BFTCPIP();

	// after any call (except Read) which returns false GetLastError() can be used
	bool CreateSocket(SOCKET& networkSocket, int socketType );
	bool Connect(SOCKET& networkSocket, const char* networkAddr, unsigned short networkPort);
	bool CloseSocket(SOCKET networkSocket);
	bool Select(SOCKET&, bool testRead, bool testWrite, bool testError, unsigned long timeoutMilliseconds=0);
	bool Read(SOCKET&, char* buffer, int bytesToRead, int &bytesRead);
	bool Write(SOCKET networkSocket, const char* buffer, int bytesToWrite, int &bytesWritten);
	int GetLastError();
private:
	BFTCPIP(BFTCPIP&) {}
};

#endif
