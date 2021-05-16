#ifndef _BITFLIX_MATH_H_
#define _BITFLIX_MATH_H_

#include <ostream>

#define DllExport   __declspec( dllexport )

struct DllExport BFVector3
{
	BFVector3(float* w) { v[0] = w[0]; v[1] = w[1]; v[2] = w[2]; }
	BFVector3(float x, float y, float z) { v[0] = x; v[1] = y; v[2] = z; }
	float v[3];
};

DllExport std::ostream& operator<<(std::ostream &out, BFVector3 &v3);

struct DllExport BFVector2
{
	BFVector2(float x, float y) { v[0] = x; v[1] = y; }
	float v[2];
};

DllExport std::ostream& operator<<(std::ostream& out, BFVector2& v2);

struct DllExport BFMatrix44
{
	float m[4][4];
};

#endif
