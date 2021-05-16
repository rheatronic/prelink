#pragma once

#include <string>
#include <vector>
#include "camera.h"

// server-side incoming messages

class PrelinkMessage { virtual void dummy() {} };

class PrelinkStatusRequest : public PrelinkMessage {
public:
	bool status;
	std::vector<std::string> items;
};

struct Matrix44 {
	float matrix[4][4];
};

class PrelinkNode {
public:
	std::string name;
	enum { visible=0, invisible=1, nochange=2 } visibility;
	Matrix44 matrix;
};

class PrelinkFrame : public PrelinkMessage {
public:
	Timecode timecode;
	std::vector<PrelinkNode*> nodes;
};

// server-side outgoing message

// only one, CameraSample* defined in camera.h
