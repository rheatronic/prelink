#ifndef __PRELINK_PROPERTIES_H__
#define __PRELINK_PROPERTIES_H__

#include "bitflix/bitflix_debug.h"

#include "prelink_state.h"

class PropertyBool : public State {
public:
	PropertyBool(char *name, bool value) { mName = name; mValue = value; }
	virtual void write(std::ostrstream &out) { out << "<property id=\"" << mName << "\">" << ( mValue ? "true" : "false" ) << "</property>"; }
private:
	std::string mName;
	bool mValue;
};

#endif
