#include "prelink_parser.h"
#include "xml_stream.h"
#include "prelink_message_stream.h"
#include "camera.h"

PrelinkMessageStream::PrelinkMessageStream(SOCKET s)
	: IXMLSocketStream(s) {
	ParseState::initialize(_states);
}

PrelinkMessage* PrelinkMessageStream::Receive()
{
	XMLElement* element = Read();
	ParseState* top = _states.top();
	return top->next(element, _states);
}

#include <iostream>
#include <sstream>

void PrelinkMessageStream::Send(CameraSample* cam)
{
	std::ostringstream oss(std::ostringstream::out);
	oss << "<previzion>" << std::endl;
		oss << "<status>" << std::endl;
			Timecode tc = cam->getTimecode();
			oss << "<frame timecode=\"" << tc << "\" rate=\"" << tc.rate << "\">" << std::endl;
				oss << "<camera>" << std::endl;
					oss << "<position>" << cam->getPosition() << "</position>" << std::endl;	
					oss << "<forward>" << cam->getForward() << "</forward>" << std::endl;
					oss << "<up>" << cam->getUp() << "</up>" << std::endl;
					oss << "<xfov>" << cam->getFOV() << "</xfov>" << std::endl;
					oss << "<focus_distance>" << cam->getFocusDistance() << "</focus_distance>" << std::endl;
					oss << "<iris>" << cam->getIris() << "</iris>" << std::endl;
					oss << "<subject_distance>" << cam->getSubjectDistance() << "</subject_distance>" << std::endl;
					oss << "<distortion_r2>" << cam->getDistortionR2() << "</distortion_r2>" << std::endl;
					// if camera is stereoscopic, also send:
					// <interocular>5.3</interocular>
					// <convergence>250.0</convergence>
				oss << "</camera>" << std::endl;
			oss << "</frame>" << std::endl;
		oss << "</status>" << std::endl;
	oss << "</previzion>" << std::endl;
	std::string buf(oss.str());
	Put(buf.c_str(), buf.size());
}
