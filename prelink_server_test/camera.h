#ifndef _CAMERA_H
#define _CAMERA_H

// An abstract camera class, with data members and stream formatting

#include <ostream>

const float DEFAULT_TIMECODE_RATE = 23.976f;

struct Timecode
{
	Timecode()
		: hour(0), minute(0), second(0), frame(0), rate(0.0f) { initialize_rate(); }
	Timecode(const Timecode& other)
		: hour(other.hour), minute(other.minute), second(other.second), frame(other.frame), rate(other.rate) { initialize_rate(); }
	Timecode(const char* tc, float r=0.0f);
	Timecode(unsigned char h, unsigned char m, unsigned char s, unsigned char f, float r=0.0f)
		: hour(h), minute(m), second(s), frame(f), rate(r) { initialize_rate(); }

	void initialize_rate() { if( rate == 0.0f ) rate = DEFAULT_TIMECODE_RATE; }
	
	Timecode operator+(const Timecode& other);
	Timecode& operator+=(const Timecode& other);

	bool operator==(const Timecode& other) { return hour == other.hour && minute == other.minute && second == other.second && frame == other.frame && rate == other.rate; }

	unsigned char hour, minute, second, frame;
	float rate;
};

std::ostream& operator<<(std::ostream &out, Timecode &tc);

struct Vector3
{
	Vector3(float* w) { v[0] = w[0]; v[1] = w[1]; v[2] = w[2]; }
	Vector3(float x, float y, float z) { v[0] = x; v[1] = y; v[2] = z; }
	float v[3];
};

std::ostream& operator<<(std::ostream &out, Vector3 &v3);

struct Vector2
{
	Vector2(float x, float y) { v[0] = x; v[1] = y; }
	float v[2];
};

std::ostream& operator<<(std::ostream& out, Vector2& v2);

class Camera
{
public:
	virtual bool validTimecode() = 0;
	virtual Timecode getTimecode() = 0;
	virtual float getFramerate() = 0;
	virtual Vector3 getPosition() = 0;
	virtual Vector3 getUp() = 0;
	virtual Vector3 getForward() = 0;
	virtual Vector2 getFOV() = 0;
	virtual float getFocusDistance() = 0;
	virtual float getIris() = 0;
	virtual float getSubjectDistance() = 0;
	virtual float getDistortionR2() = 0;
	virtual bool isStereoscopic() { return false; }
	virtual float getInteraxial() = 0;
	virtual float getConvergence() = 0;
};

class CameraSample : public Camera
{
public:
	CameraSample(Camera* camera) :
		_timecode(camera->getTimecode()),
		_position(camera->getPosition()), 
		_up(camera->getUp()),
		_forward(camera->getForward()), 
		_FOV(camera->getFOV()),
		_focusDistance(camera->getFocusDistance()),
		_iris(camera->getIris()),
		_subjectDistance(camera->getSubjectDistance()),
		_distortionR2(camera->getDistortionR2()),
		_isStereoscopic(camera->isStereoscopic()),
		_interaxial(camera->getInteraxial()),
		_convergence(camera->getConvergence())
		{}

	virtual bool validTimecode() { return true; }
	virtual Timecode getTimecode() { return _timecode; }
	virtual float getFramerate() { return _framerate; }
	virtual Vector3 getPosition() { return _position; }
	virtual Vector3 getUp() { return _up; }
	virtual Vector3 getForward() { return _forward; }
	virtual Vector2 getFOV() { return _FOV; }
	virtual float getFocusDistance() { return _focusDistance; }
	virtual float getIris() { return _iris; }
	virtual float getSubjectDistance() { return _subjectDistance; }
	virtual float getDistortionR2() { return _distortionR2; }
	virtual bool isStereoscopic() { return _isStereoscopic; }
	virtual float getInteraxial() { return _interaxial; }
	virtual float getConvergence() { return _convergence; }

protected:
	Timecode _timecode;
	float _framerate;
	Vector3 _position;
	Vector3 _up;
	Vector3 _forward;
	Vector2 _FOV;
	float _focusDistance;
	float _iris;
	float _subjectDistance;
	float _distortionR2;
	bool _isStereoscopic;
	float _interaxial;
	float _convergence;
};

#endif
