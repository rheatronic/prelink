#include "bitflix_time.h"

#include <iostream>
#include <iomanip>
#include <regex>

typedef std::match_results<const char*> cmatch;

BFTimecode::BFTimecode(const char* tc, float r)
	: hour(0), minute(0), second(0), frame(0), rate(r)
{
	if(!tc || !tc[0])
		throw std::invalid_argument("Timecode string empty");

	std::regex expression("([0-9][0-9]):([0-9][0-9]):([0-9][0-9]):([0-9][0-9])");
	cmatch results;
	std::regex_search(tc, results, expression);
	// note std::match_results size is 5 (first is whole area matched, then groups)
	// but length is the length of the whole matched area matched (in this case 11)
	if(results.size() != 5)
		throw std::invalid_argument("Timecde invalid string format");

	hour = atoi(results[1].str().c_str());

	minute = atoi(results[2].str().c_str());
	if(minute >= 60)
		throw std::invalid_argument("Timecode minute count >= 60");

	second = atoi(results[3].str().c_str());
	if(second >= 60)
		throw std::invalid_argument("Timecode second count >= 60");

	frame = atoi(results[4].str().c_str());
	if( frame >= (unsigned char)std::ceil(rate))
		throw std::invalid_argument("Timecode frame count bad for rate");
}

BFTimecode BFTimecode::operator+(const BFTimecode& other)
{
	if( rate != other.rate)
		throw std::invalid_argument("Mismatched rates on timecodes being added");

	BFTimecode result(*this);
		
	unsigned char fps = (unsigned char)std::ceil(rate);
	result.frame += other.frame;
	result.second += result.frame / fps;
	result.frame %= fps;
		
	unsigned char spm = 60;
	result.second += other.second;
	result.minute += result.second / spm;
	result.second %= spm;

	unsigned char mph = 60;
	result.minute += other.minute;
	result.hour += result.minute / mph;
	result.minute %= mph;

	result.hour += other.hour;
		
	return (result);
}

BFTimecode& BFTimecode::operator+=(const BFTimecode& other)
{
	*this = *this + other;
	return *this;
}


std::ostream& operator<<(std::ostream &out, BFTimecode &tc)
{
	out << std::setw(2) << std::setfill('0') << (unsigned int)tc.hour
		<< ":"
		<< std::setw(2) << std::setfill('0') << (unsigned int)tc.minute
		<< ":"
		<< std::setw(2) << std::setfill('0') << (unsigned int)tc.second
		<< ":"
		<< std::setw(2) << std::setfill('0') << (unsigned int)tc.frame;
	return out;
}
