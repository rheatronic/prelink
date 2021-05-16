#ifndef __PREVIZION_STATUS_STREAM_H__
#define __PREVIZION_STATUS_STREAM_H__

#include "previzion_status.h"
#include <string>

// Processes TCP stream into status updates.

class StatusStream
{
public:
	StatusStream::StatusStream() { Reset(); }
	void SetMode(std::string &lBuffer, bool pMode, char* pItem);
	void Reset() { mBuffer.clear(); }
	void ParseBuffer(PrevizionUpdate& update, const char* pBuffer, int pNumberOfBytesRead);
private:
	void ParseRecord(PrevizionUpdate& update, std::string::size_type pStart, std::string::size_type pStop);
	std::string mBuffer;
};

#endif
