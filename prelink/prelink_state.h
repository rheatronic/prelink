#ifndef __PRELINK_STATE_H__
#define __PRELINK_STATE_H__

#include "bitflix/bitflix_debug.h"

#include <strstream>

class State {
public:
	virtual void write(std::ostrstream &out) = 0;
};

#endif
