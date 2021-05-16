#include "prelinkConnector.h"

MayaTransformState::MayaTransformState(MString &name, MMatrix &transform, bool &visibility)
{
	mName = name;
	mMatrix = transform;
	mVisibility = visibility;
}

void MayaTransformState::getName(std::string &name)
{
	name = mName.asChar();
}

void MayaTransformState::getVisibility(bool &visibility) 
{
	visibility = mVisibility;
}

void MayaTransformState::getMatrix(BFMatrix44 &matrix) 
{
	for(int r=0; r<4; r++)
		for(int c=0; c<4; c++)
			matrix.m[r][c] = (float)mMatrix[r][c];
}

//

MayaFramedState::MayaFramedState()
	: FramedState() 
{
}

double asRate(MTime::Unit unit)
{
	double rates[MTime::kLast];
	rates[MTime::kInvalid] = 0.0;
	rates[MTime::kHours] = 0.0;                            //!< 3600 seconds
	rates[MTime::kMinutes] = 0.0;                          //!< 60 seconds
	rates[MTime::kSeconds] = 0.0;                          //!< 1 second
	rates[MTime::kMilliseconds] = 1000.0;                  //!< 1/1000 of a second
	rates[MTime::kGames] = 15.0;                           //!< 15 frames per second
	rates[MTime::kFilm] = 24.0;                            //!< 24 frames per second
	rates[MTime::kPALFrame] = 25.0;                        //!< 25 frames per second
	rates[MTime::kNTSCFrame] = 30.0 * 1000.0 / 1001.0;     //!< 30 frames per second
	rates[MTime::kShowScan] = 48.0;						//!< twice the speed of film (48 frames per second)
	rates[MTime::kPALField] = 50.0;						//!< twice the speed of PAL (50 frames per second)
	rates[MTime::kNTSCField] = 60.0 * 1000.0 / 1001.0;	//!< twice the speed of NTSC (60 frames per second)
	rates[MTime::k2FPS] = 2.0;								//!< 2 frames per second
	rates[MTime::k3FPS] = 3.0;								//!< 3 frames per second
	rates[MTime::k4FPS] = 4.0;								//!< 4 frames per second
	rates[MTime::k5FPS] = 5.0;								//!< 5 frames per second
	rates[MTime::k6FPS] = 6.0;								//!< 6 frames per second
	rates[MTime::k8FPS] = 8.0;								//!< 8 frames per second
	rates[MTime::k10FPS] = 10.0;							//!< 10 frames per second
	rates[MTime::k12FPS] = 12.0;							//!< 12 frames per second
	rates[MTime::k16FPS] = 16.0;							//!< 16 frames per second
	rates[MTime::k20FPS] = 20.0;							//!< 20 frames per second
	rates[MTime::k40FPS] = 40.0;							//!< 40 frames per second
	rates[MTime::k75FPS] = 75.0;							//!< 75 frames per second
	rates[MTime::k80FPS] = 80.0;							//!< 80 frames per second
	rates[MTime::k100FPS] = 100.0;							//!< 100 frames per second
	rates[MTime::k120FPS] = 120.0;							//!< 120 frames per second
	rates[MTime::k125FPS] = 125.0;							//!< 125 frames per second
	rates[MTime::k150FPS] = 150.0;							//!< 150 frames per second
	rates[MTime::k200FPS] = 200.0;							//!< 200 frames per second
	rates[MTime::k240FPS] = 240.0;							//!< 240 frames per second
	rates[MTime::k250FPS] = 250.0;							//!< 250 frames per second
	rates[MTime::k300FPS] = 300.0;							//!< 300 frames per second
	rates[MTime::k375FPS] = 375.0;							//!< 375 frames per second
	rates[MTime::k400FPS] = 400.0;							//!< 400 frames per second
	rates[MTime::k500FPS] = 500.0;							//!< 500 frames per second
	rates[MTime::k600FPS] = 600.0;							//!< 600 frames per second
	rates[MTime::k750FPS] = 750.0;							//!< 750 frames per second
	rates[MTime::k1200FPS] = 1200.0;						//!< 1200 frames per second
	rates[MTime::k1500FPS] = 1500.0;						//!< 1500 frames per second
	rates[MTime::k2000FPS] = 2000.0;						//!< 2000 frames per second
	rates[MTime::k3000FPS] = 3000.0;						//!< 3000 frames per second
	rates[MTime::k6000FPS] = 6000.0;						//!< 6000 frames per second
	return rates[unit];
}

BFTimecode asBFTimecode(const MTime& mtime)
{
	double rate = asRate(mtime.unit());
	unsigned char 
		hours = (unsigned char)floor(mtime.asUnits(MTime::kHours)), 
		minutes = (unsigned char)floor(mtime.asUnits(MTime::kMinutes)), 
		seconds = (unsigned char)floor(mtime.asUnits(MTime::kSeconds)),
		frames; //  = (unsigned char)floor(mtime.asUnits(mtime.unit()));
	double secondsfracd, secondsd;
	secondsfracd = modf( mtime.asUnits(MTime::kSeconds), &secondsd );
	//seconds = (unsigned char)floor( secondsd );
	frames = (unsigned char)floor( secondsfracd * rate + 0.5 );
	return BFTimecode(hours, minutes, seconds, frames, (float)rate);
}

MayaFramedState::MayaFramedState(MTime &time)
	: FramedState(asBFTimecode(time)) 
{
}
