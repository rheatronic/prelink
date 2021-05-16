#include "spslogger.h"
#include <iostream>
#include <algorithm>
#include <vector>

void SPSLogger::Sample() {
	LARGE_INTEGER count;
	BOOL result = QueryPerformanceCounter(&count);
	if(result)
		mDeque.push_back(count.QuadPart);
	else
		std::cerr << "Could not query performance counter" << std::endl;

	while(mDeque.size() > mLength)
		mDeque.pop_front();
}

void SPSLogger::Desample() {
	if(mDeque.size() > 0 )
		mDeque.pop_front();
}

template <class fwdit> 
typename fwdit::value_type arithmetic_mean(fwdit begin, fwdit end) { 
    typedef typename fwdit::value_type res_type;

    res_type sum = res_type();
    size_t count = 0;

    for (fwdit pos = begin; pos!= end; ++pos) { 
        sum += *pos;
        ++count;
    }
    return sum/count;
}

void SPSLogger::GetStats(float &pSPS, float &pJitter) {
	if(mDeque.size() < 2) {
		pSPS = 0.0f;
		pJitter = 0.0f;
		return;
	}

	LARGE_INTEGER freq;
	BOOL result = QueryPerformanceFrequency(&freq);
	if(!result) {
		std::cerr << "Could not query performance counter frequency" << std::endl;
		return;
	}

	// get the sample times

	CountDeque::iterator first = mDeque.begin();
	CountDeque::iterator second = mDeque.begin() + 1;

	std::vector<long long> sampleCounts;

	while( second != mDeque.end() ) {
		sampleCounts.push_back(*second - *first);
		second++; first++;
	}

	long long meanCount = arithmetic_mean(sampleCounts.begin(), sampleCounts.end());
	pSPS = (float)freq.QuadPart / (float)meanCount;

	// calculate jitter as percentage of frame time

	// - get min/max of differences to mean count (sample) time

	// - distance max to min is jitter value over this set of samples

	// temp return jitter value as msec / frame
	pJitter = (float)(meanCount * 1000000 / freq.QuadPart);
}
