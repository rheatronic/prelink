#include "mobu_time.h"

FBTimeMode timeModeFromRate(double rate) {
	if(rate > 59.9 && rate < 60.0)
		return kFBTimeMode5994Frames;
	if(rate > 29.9 && rate < 30.0)
		return kFBTimeMode2997Frames;
	if(rate == 25.0)
		return kFBTimeMode25Frames;
	if(rate > 23.9 && rate < 24.0)
		return kFBTimeMode23976Frames;
	else
		return kFBTimeModeDefault;
}
