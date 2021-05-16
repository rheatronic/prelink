#ifndef __PREVIZION_CAMERA_FRAME_H__
#define __PREVIZION_CAMERA_FRAME_H__

// The format specified by Phil is capable of greater generality
// In particular, every status record sent from Previzion could potentially contain:
// 1) a list of non-time related statii
// 2) a list of frame times, each containing:
//   2a) a list of unordered time related statii

// In future a StatusRecord will be an unordered collection of status items
// Some of these items will have frame times associated, and some not.

// In the meantime we will assume a status record contains a single CameraStatus

// A status item, not associated with a particular moment in time
class Status {
};

// A status record associated with a frame time
class FramedStatus : public Status {
public:
	unsigned char hour, minute, second, frame;
	float rate;
};

// A frame of ingested camera data from previzion's status stream
class CameraStatus : public FramedStatus {
public:
	CameraStatus() : stereo(false) {}
	double position[3], forward[3], up[3], rotation[3];
	double zoom, fov[2], focus, iris, subject, distortion, interocular, convergence;
	bool stereo;
};

class StatusRecord {
public:
	StatusRecord() {}
	CameraStatus mStatus[1];
};

#endif
