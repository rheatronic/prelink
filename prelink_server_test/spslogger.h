#ifndef _SPSLOGGER_H_
#define _SPSLOGGER_H_

#include <deque>
#include <windows.h>

typedef std::deque<long long> CountDeque;

class SPSLogger {
public:
	SPSLogger::SPSLogger(std::size_t pLength) : mLength(pLength) {}
	void Sample();
	void Desample();
	void GetStats(float &pSPS, float &pJitter);
private:
	std::size_t mLength;
	CountDeque mDeque;
};

#endif
