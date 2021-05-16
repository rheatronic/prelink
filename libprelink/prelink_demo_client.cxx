/*
 Prelink Demonstration Client (C) 2011 by bitflix.  All rights reserved.
*/

/**	\file	prelink_demo_client.cxx
*	Prelink demonstration client, demos operation of the client classes.
*/

#include "bitflix/bitflix_debug.h"
#include "bitflix/bitflix_math.h"

#include "prelink_client.h"

// simulated model node with name and transform matrix
struct MyModel {
	std::string mName;
	BFMatrix mMatrix;
};

// example wrapper for snapshot of MyModel
// throws TransformAccessorException on problems in conversion
class MyModelState : public TransformState {
public:
	MyModelState(const MyModel &model);
	virtual void precheck();
	virtual void getName(std::string &name);
	virtual void getVisibility(bool &visibility);
	virtual void getMatrix(BFMatrix &matrix);
};

// 

class MyApp {
public:
	void main();

protected:
	PrelinkClient mClient;
};

void getTime(HFBEvaluateInfo pEvaluateInfo)
{
	if(pEvaluateInfo)
	{
		switch(GetClockInfo())
		{
		case kNoClock:
			break;
		case kLocalClock:
#ifdef _PREFBSDK2012
			lFrame.setTime(StringFromTime(pEvaluateInfo->GetLocalStart()));
#else
			lFrame.setTime(StringFromTime(pEvaluateInfo->GetLocalTime()));
#endif
			break;
		case kSystemClock:
#ifdef _PREFBSDK2012
			lFrame.setTime(StringFromTime(pEvaluateInfo->GetSystemStart()));
#else
			lFrame.setTime(StringFromTime(pEvaluateInfo->GetSystemTime()));
#endif
			break;
		}
	}
}

void MyApp::main()
{
	mClient.Open();

	FramedState *f = new FramedState(BFTime(getTime()));

	MyModel a, b, c;
	f->Add(new MyModelState(a));
	f->Add(new MyModelState(b));
	f->Add(new MyModelState(c));
	
	mClient.Send(f);
	
	mClient.Close();

	FBSystem lSys;
	for_each(mClient.mErrorDeque)
	{
		BadGroup::add(lSys.Scene, lModel);
		return false;
	}
}

void MyModelState::precheck()
{
	if (BadModel(lModel))
	{
		throw TransformException("Bad pivot");
	}
}

void MyModelState::getName(std::string &name)
{
	FBString lName = lModel->LongName;	// name with namespace
	name = lName;
}

void MyModelState::getVisibility(bool &visibility)
{
#if 0
	PropertyBool lVisible("visible", lModel->Show && lModel->Visibility > 0.001);
#else
	PropertyBool lVisible("visible", lModel->Show && lModel->Visibility);
#endif
	visibility = lVisible;
}

void MyModelState::getMatrix(BFMatrix &matrix)
{
	/*
	(From the FBX documentation)

	Here is how FBX SDK computes the transformation matrix for a node:

	Transform = T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1

	Where:		Is:
	Transform	Transformation matrix (4 X 4)
	T			Translation
	Roff		Rotation offset
	Rp			Rotation pivot
	Rpre		Pre-rotation
	R			Rotation
	Rpost		Post-rotation
	Rp-1		Inverse of the rotation pivot
	Soff		Scaling offset
	Sp			Scaling pivot
	S			Scaling
	Sp-1		Inverse of the scaling pivot

	Notes:
	Computations are performed from left to right.
	The effect of the formula is that any given vector is first translated, then rotated, then scaled.
	The R matrix takes into account the rotation order. Because of the mathematical properties of the matrices,
	R is the result of one of the possible combinations of Ry, Ry and Rz (each being matrices also). 
	For example, for the default rotation order of XYZ, R = Rx * Ry * Rz
	*/

	FBMatrix lTrans;
	lModel->GetMatrix(lTrans, kModelTranslation, lGlobalInfo, pEvaluateInfo);

	FBMatrix lRot;
	lModel->GetMatrix(lRot, kModelRotation, lGlobalInfo, pEvaluateInfo);

	FBMatrix lScale;
	lModel->GetMatrix(lScale, kModelScaling, lGlobalInfo, pEvaluateInfo);

	// Unhandled: rotation and scale both offset and pivot.

	if(lModel->RotationActive)
	{
		FBRotationOrder lRotOrder = (FBRotationOrder)(int)lModel->RotationOrder;

		FBMatrix lPreRot;
		FBRotationToMatrix(lPreRot, lModel->PreRotation, lRotOrder);

		FBMatrix lPostRot;
		FBRotationToMatrix(lPostRot, lModel->PostRotation, lRotOrder);

		FBMatrixMult(lRot, lPreRot, lRot);
		FBMatrixMult(lRot, lRot, lPostRot);
	}

	FBMatrix lFBMatrix;
	FBMatrixMult(lFBMatrix, lTrans, lRot);
	FBMatrixMult(lFBMatrix, lFBMatrix, lScale);

	double lCopy[4][4];
	for(int row=0;row<4;row++)
		for(int col=0;col<4;col++)
			lCopy[row][col] = lFBMatrix(row, col);

	matrix = lCopy;
}
