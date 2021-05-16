#ifndef _PRELINK_CONNECTOR_H_
#define _PRELINK_CONNECTOR_H_

#include "libprelink/libprelink.h"

#include <maya/MTime.h>
#include <maya/MString.h>
#include <maya/MMatrix.h>

double asRate(MTime::Unit unit);

struct MayaTransformState : public TransformState 
{
	MayaTransformState(MString &name, MMatrix &transform, bool &visibility);
	virtual void getName(std::string &name);
	virtual void getVisibility(bool &visibility);
	virtual void getMatrix(BFMatrix44 &matrix);

	MString mName;
	MMatrix mMatrix;
	bool mVisibility;
};

struct MayaFramedState : public FramedState
{
	MayaFramedState();
	MayaFramedState(MTime &time);
};

#endif
