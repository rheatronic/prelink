#ifndef __PREVIZION_CAMERA_FRAME_H__
#define __PREVIZION_CAMERA_FRAME_H__

#include <vector>
#include "bitflix\bitflix_time.h"

// The format specified by Phil is capable of great generality
// In particular, every status record sent from Previzion could potentially contain:
// 1) a list of non-time related statii
// 2) a list of frame times, each containing:
//   2a) a list of unordered time related statii

#ifdef _USRDLL
        // .dll
        #define DECLSPECIFIER __declspec(dllexport)
        #define EXPIMP_TEMPLATE
#else
        // .exe
        #define DECLSPECIFIER __declspec(dllimport)
        #define EXPIMP_TEMPLATE extern
#endif

class DECLSPECIFIER PrevizionStatus {
public:
	virtual void dummy() {}
};

EXPIMP_TEMPLATE template class DECLSPECIFIER std::vector<PrevizionStatus*>;

class DECLSPECIFIER CameraStatus : public PrevizionStatus {
public:
	CameraStatus() : stereo(false) {}
	double position[3], forward[3], up[3], rotation[3];
	double zoom, fov[2], focus, iris, subject, distortion, interocular, convergence;
	bool stereo;

	CameraStatus& operator=(CameraStatus const &other) {
		for(int i = 0; i < 3; i++) {
			position[i] = other.position[i];
			forward[i] = other.forward[i];
			up[i] = other.up[i];
			rotation[i] = other.rotation[i];
		}
		zoom = other.zoom;
		fov[0] = other.fov[0] ; fov[1] = other.fov[1];
		focus = other.focus;
		iris = other.iris;
		subject = other.subject;
		distortion = other.distortion;
		interocular = other.interocular;
		convergence = other.convergence;
		stereo = other.stereo;
		return *this;
	}
};

class DECLSPECIFIER PrevizionFrame {
public:
	PrevizionFrame() : mHasTime(false), mTime(0, 0, 0, 0, 0.0) {}
	PrevizionFrame(BFTimecode &tc) : mHasTime(true), mTime(tc) {}
	bool mHasTime;
	BFTimecode mTime;
	
	std::vector<PrevizionStatus*> mStates;
};

typedef std::vector<PrevizionFrame*> PrevizionUpdate;

#endif
