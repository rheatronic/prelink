#ifndef _BITFLIX_TIME_H
#define _BITFLIX_TIME_H

#include <ostream>

#define DllExport   __declspec( dllexport )

struct DllExport BFTimecode
{
	BFTimecode(const BFTimecode& other)
		: hour(other.hour), minute(other.minute), second(other.second), frame(other.frame), rate(other.rate) {}
	BFTimecode(const char* tc, float r);
	BFTimecode(unsigned char h, unsigned char m, unsigned char s, unsigned char f, float r)
		: hour(h), minute(m), second(s), frame(f), rate(r) {}
	
	BFTimecode operator+(const BFTimecode& other);
	BFTimecode& operator+=(const BFTimecode& other);

	unsigned char hour, minute, second, frame;
	float rate;
};

DllExport std::ostream& operator<<(std::ostream &out, BFTimecode &tc);

#endif
