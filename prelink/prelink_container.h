#ifndef __PRELINK_CONTAINER_H__
#define __PRELINK_CONTAINER_H__

#include "bitflix/bitflix_debug.h"

#include <strstream>

class Container {
public:
	virtual void writeBegin(std::ostrstream &out) = 0;
	virtual void writeEnd(std::ostrstream &out) = 0;
};

#endif
