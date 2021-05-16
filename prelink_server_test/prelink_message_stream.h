#pragma once

#include "xml_stream.h"
#include <vector>
#include "prelink_parser.h"
class CameraSample;

/* 
XML token stream to PrelinkMessage interpretation
Buffers string of tokens from <previzion> to </previzion>
and then does a recursive descent parse.
*/ 
class PrelinkMessageStream : public IXMLSocketStream {
public:
	PrelinkMessageStream(SOCKET s);
	PrelinkMessage* Receive();		// caller takes ownership
	void Send(CameraSample* cam);	// caller retains ownership
private:
	StateStack _states;
};
