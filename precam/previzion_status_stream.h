#ifndef __PREVIZION_STATUS_STREAM_H__
#define __PREVIZION_STATUS_STREAM_H__

#include "previzion_camera_frame.h"
#include <fbsdk/fbsdk.h>
#include <string>

// Processes TCP stream into status records.

class StatusStream
{
public:
	StatusStream::StatusStream() { Reset(); }
	void SetMode(FBString &lBuffer, bool pMode, char* pItem);
	void Reset() { mBuffer.clear(); }
	StatusRecord *ParseBuffer(const char* pBuffer, int pNumberOfBytesRead);
private:
	StatusRecord* ParseRecord(std::string::size_type pStart, std::string::size_type pStop);
	std::string mBuffer;
};

#endif
