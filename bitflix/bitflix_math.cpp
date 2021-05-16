#include "bitflix_math.h"

std::ostream& operator<<(std::ostream &out, BFVector3 &v3)
{
	out << v3.v[0] << ' ' << v3.v[1] << ' ' << v3.v[2];
	return out;
}

std::ostream& operator<<(std::ostream& out, BFVector2& v2)
{
	out << v2.v[0] << ' ' << v2.v[1];
	return out;
}
