#ifndef __PRELINK_NODE_H__
#define __PRELINK_NODE_H__

#include "bitflix/bitflix_debug.h"

#include <string>
#include "prelink_container.h"

class Node : public Container {
public:
	Node(char *name) { mName = name; }
	virtual void writeBegin(std::ostrstream &out) { out << "<node id=\"" << mName << "\">"; }
	virtual void writeEnd(std::ostrstream &out) { out << "</node>"; }
private:
	std::string mName;
};

#endif
