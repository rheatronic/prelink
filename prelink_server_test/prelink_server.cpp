#include "prelink_server.h"
#include "prelink_messages.h"
#include <sstream>

void PrelinkConnection::LogToFile(const char* pPath)
{
	_logStream.open(pPath);
}

void PrelinkConnection::StopLogging()
{
	_logStream.close();
}

bool PrelinkConnection::Read()
{
	do {
		PrelinkMessage* msg = _pms.Receive();
	
		PrelinkStatusRequest* status = dynamic_cast<PrelinkStatusRequest*>(msg);
		if( status ) {
			_sendCamera = status->status;
			_logStream << "Camera status=" << _sendCamera << std::endl;
			delete status;
			continue;
		}

		static const char* visibility_names[] = {"Visible", "Invisible", "No Change"};

		PrelinkFrame* frame = dynamic_cast<PrelinkFrame*>(msg);
		if( frame ) {
			_logStream << "Frame " << frame->timecode << " " << frame->timecode.rate << std::endl;
			for(auto i = frame->nodes.cbegin(); i != frame->nodes.cend(); i++) {
				PrelinkNode* pln = *i;
				_logStream << "\tNode " << pln->name << " " << visibility_names[pln->visibility] << std::endl;
				for(int r=0;r<4;r++) {
					_logStream << "\t";
					for(int c=0;c<4;c++)
						_logStream << pln->matrix.matrix[r][c] << " ";
					_logStream << std::endl;
				}
				delete pln;
			}
			_logStream << std::endl;
			delete frame;
			continue;
		}

		if( msg ) {
			std::cerr << "Unknown msg type" << std::endl;
			delete msg;
		}
	} while ( !_pms.incomplete() && !_pms.empty() );

	return true;
}

bool PrelinkConnection::Write()
{
	// send an (abstract) camera status msg

	// This should decompose into routines for
	// - prepping the wrapper
	// - dispatching to routines specific to enables types of status to send

	// for now, do it inline

	if( !_sendCamera )
		return true;

	Camera* cam = getCamera();
	if( !cam )
		return true;

	std::ostringstream oss;

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

	int iResult = send(_socket, oss.str().c_str(), oss.str().length(), 0);
	if (iResult == SOCKET_ERROR) {
		//printf("send failed: %d\n", WSAGetLastError());
		closesocket(_socket);
		return false;
	}

	mWriteSPS.Sample();

	return true;
}
