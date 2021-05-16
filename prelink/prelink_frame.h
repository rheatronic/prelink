#ifndef __PRELINK_FRAME_H__
#define __PRELINK_FRAME_H__

#include "bitflix/bitflix_debug.h"

#include <string>
#include "prelink_container.h"

class Frame : public Container {
public:
	Frame() : mHasTime(false) {}
	Frame(char *timecode) { setTime(timecode); }
	void setTime(char *timecode) { mHasTime = true; mTimecode = timecode; }
	virtual void writeBegin(std::ostrstream &out) { out << "<frame"; if(mHasTime) out << " timecode=\"" << mTimecode << "\""; out << ">"; }
	virtual void writeEnd(std::ostrstream &out) { out << "</frame>"; }
private:
	bool mHasTime;
	std::string mTimecode;
};

#endif
