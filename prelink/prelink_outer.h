#ifndef __PRELINK_OUTER_H__
#define __PRELINK_OUTER_H__

#include "bitflix/bitflix_debug.h"

#include "prelink_container.h"

class Outer : public Container {
	virtual void writeBegin(std::ostrstream &out) { out << "<previzion><animation>"; }
	virtual void writeEnd(std::ostrstream &out) { out << "</animation></previzion>\0\0"; }
};

#endif
