#include "previzion_status_stream.h"
#include <sstream>
#include "lookat.h"
#define _USE_MATH_DEFINES
#include <math.h>

void StatusStream::SetMode(FBString &lBuffer, bool pMode, char* pItem)
{
	lBuffer = "<previzion><status_request value=\"";
	if( pMode ) lBuffer += "true\"><";
	else lBuffer += "false\"><";
	lBuffer += pItem;
	lBuffer += "></";
	lBuffer += pItem;
	lBuffer += "></status_request></previzion>";
}

const char* START_TOKEN = "<previzion>";
const char* END_TOKEN = "</previzion>";

#include "xml_parser.h"

StatusRecord* StatusStream::ParseBuffer(const char* pBuffer, int pNumberOfBytesRead)
{
	StatusRecord* lResult = 0;

	mBuffer.assign(pBuffer, pNumberOfBytesRead);

	// Find start token
	std::string::size_type lStart = mBuffer.find(START_TOKEN);;
	if( lStart != std::string::npos )
	{
		// clip off chars before delimiter
		mBuffer.erase(0, lStart);

		// Find end token (after starting token)
		std::string::size_type lStop = mBuffer.find(END_TOKEN);
		if( lStop != std::string::npos )
		{
			// parse the section
			lResult = ParseRecord( 0, lStop + strlen(END_TOKEN) );

			// leave remaining chars in buffer
			mBuffer.erase(0, lStop + strlen(END_TOKEN) );
		}
		else
		{
			// wait for next round of characters
		}
	}
	else
	{
		// wait for more characters
	}

	return lResult;
}

void Read(const std::string lContent, double pValue[3])
{
	std::stringstream iss(lContent, std::stringstream::in);
	iss >> pValue[0];
	iss >> pValue[1];
	iss >> pValue[2];
}

void Read(const std::string lContent, double &pValue)
{
	std::stringstream iss(lContent, std::stringstream::in);
	iss >> pValue;
}

void Read(const std::string lContent, unsigned char &pHour, unsigned char &pMinute, unsigned char &pSecond, unsigned char &pFrame)
{
	if( lContent.length() != 11 )
		throw BadSyntax();

	std::stringstream iss(lContent, std::stringstream::in);
	char delimiter;
	int number;
	
	iss >> number;
	pHour = number;
	iss >> delimiter;

	iss >> number;
	pMinute = number;
	iss >> delimiter;

	iss >> number;
	pSecond = number;
	iss >> delimiter;

	iss >> number;
	pFrame = number;
}

void Read(const std::string lContent, float &pValue)
{
	std::stringstream iss(lContent, std::stringstream::in);
	iss >> pValue;
}

StatusRecord* StatusStream::ParseRecord( std::string::size_type pStart, std::string::size_type pStop )
{
	// Attempt to parse an entire status record

	/*
<previzion>
 <status>
  <frame timecode="10:45:00:12" rate="29.97">
    <camera>
     <position>200.0 0.0 -1500.0</position>
     <forward>0.7071 0.0 0.7071</forward>
     <up>0.0 1.0 0.0</up>
     <xfov>48.3</xfov>
     <focus_distance>250.0</focus_distance>
     <iris>1.6</iris>
     <subject_distance>123.4</subject_distance>
     <distortion_r2>0.01</distortion_r2>
     <interocular>5.3</interocular>
     <convergence>250.0</convergence>
   </camera>
  </frame>
 <status>
</previzion>
	*/

	CameraStatus lCamera;
	lCamera.stereo = false;	// default, set to true if interocular and/or convergence are received

	std::string lName;
	AttrVec lAttrs;
	TagType lTagType;
	std::string::size_type lNext;

	try {
		// Should be previzion element
		lNext = parseTag(mBuffer, pStart, lName, lTagType, lAttrs);
		if( !(lName == "previzion" && lTagType == TAGTYPE_START) )
			throw BadSyntax();

		// Should be status element
		lNext = parseTag(mBuffer, lNext, lName, lTagType, lAttrs);
		if( !(lName == "status" && lTagType == TAGTYPE_START) )
			throw BadSyntax();

		// Should be frame element
		lNext = parseTag(mBuffer, lNext, lName, lTagType, lAttrs);
		if( !(lName == "frame" && lTagType == TAGTYPE_START) )
			throw BadSyntax();

		// Should be time and rate attrs
		for(std::size_t i=0; i<lAttrs.size(); i++)
			if( lAttrs[i].first == "timecode" )
			{
				Read(lAttrs[i].second, lCamera.hour, lCamera.minute, lCamera.second, lCamera.frame);
			}
			else if( lAttrs[i].first == "rate" )
			{
				Read(lAttrs[i].second, lCamera.rate);
			}
			else
				throw BadSyntax();

		// Should be camera element
		lNext = parseTag(mBuffer, lNext, lName, lTagType, lAttrs);
		if( !(lName == "camera" && lTagType == TAGTYPE_START) )
			throw BadSyntax();

		// 8 or 10 following elements containing the text blocks for the camera parameters
		for(int i=0; i < 10; i++)
		{
			lNext = parseTag(mBuffer, lNext, lName, lTagType, lAttrs);
			if( lTagType == TAGTYPE_END )
			{
				// if there's no stereo, we hit the end here
				break;
			} else if( lTagType != TAGTYPE_START )
				throw BadSyntax();

			std::string lStartName = lName;

			std::string lContent;
			lNext = parseContent(mBuffer, lNext, lContent);

			std::string lEndName;
			lNext = parseTag(mBuffer, lNext, lEndName, lTagType, lAttrs);
			if( lTagType != TAGTYPE_END || lStartName != lEndName )
				throw BadSyntax();

			lName = lStartName;

			if( lName == "position" )
			{
				Read(lContent, lCamera.position);
			} 
			else if( lName == "forward" )
			{
				Read(lContent, lCamera.forward);
			} 
			else if( lName == "up" )
			{
				Read(lContent, lCamera.up);
			} 
			else if( lName == "xfov" )
			{
				Read(lContent, lCamera.fov[0]);
			} 
			else if( lName == "focus_distance" )
			{
				Read(lContent, lCamera.focus);
			} 
			else if( lName == "iris" )
			{
				Read(lContent, lCamera.iris);
			} 
			else if( lName == "subject_distance" )
			{
				Read(lContent, lCamera.subject);
			} 
			else if( lName == "distortion_r2" )
			{
				Read(lContent, lCamera.distortion);
			} 
			else if( lName == "interocular" )
			{
				Read(lContent, lCamera.interocular);
				lCamera.stereo = true;
			} 
			else if( lName == "convergence" )
			{
				Read(lContent, lCamera.convergence);
				lCamera.stereo = true;
			}
		}

		if( lName == "frame" && lTagType == TAGTYPE_END )
		{
		} else
		{
			// frame
			lNext = parseTag(mBuffer, lNext, lName, lTagType, lAttrs);
			if( !(lName == "frame" && lTagType == TAGTYPE_END) )
				throw BadSyntax();
		}

		// status
		lNext = parseTag(mBuffer, lNext, lName, lTagType, lAttrs);
		if( !(lName == "status" && lTagType == TAGTYPE_END) )
			throw BadSyntax();

		// previzion
		lNext = parseTag(mBuffer, lNext, lName, lTagType, lAttrs);
		if( !(lName == "previzion" && lTagType == TAGTYPE_END) )
			throw BadSyntax();
		
	} 
	catch (BadSyntax bs) 
	{
		FBTrace("BadSyntax\n");
		return 0;
	} 
	catch (IncompleteParse ip) 
	{
		FBTrace("IncompleteParse\n");
		return 0;
	}

	// fixups

	lCamera.zoom = 0.0;	// focal length not provided by Previzion

	double lSensorWidth = 16.0, lSensorHeight = 9.0;	// Previzion only handles 16:9
	double xfov = lCamera.fov[0] / 180.0 * M_PI;
	double yfov = 2.0 * atan( ( lSensorHeight * tan( xfov / 2.0 ) ) / lSensorWidth );
	lCamera.fov[1] = yfov * 180.0 / M_PI;
	
	LookAt(lCamera.rotation, lCamera.up, lCamera.forward);

	// prep full status record
	StatusRecord *lRecord = new StatusRecord();
	lRecord->mStatus[0] = lCamera;
	return lRecord;
}
