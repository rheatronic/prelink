#include "lookat.h"

#include <fbsdk/fbsdk.h>

void Normalize(FBTVector &v)
{
	double m = FBLength(v);
	v[0] /= m;
	v[1] /= m;
	v[2] /= m;
}

void Compose(FBMatrix &matrix, FBTVector right, FBTVector up, FBTVector forward)
{
	matrix.Identity();

	matrix(0, 0) = right[0];
	matrix(0, 1) = right[1];
	matrix(0, 2) = right[2];

	matrix(1, 0) = up[0];
	matrix(1, 1) = up[1];
	matrix(1, 2) = up[2];

	matrix(2, 0) = forward[0];
	matrix(2, 1) = forward[1];
	matrix(2, 2) = forward[2];
}

void LookAt(double pRotation[3], double pUp[3], double pForward[3])
{
	FBTVector up(pUp), x(pForward);
	up[3] = 0.0; x[3] = 0.0;
	Normalize(x);

	FBTVector z;
	FBMult(z, up, x);
	Normalize(z);

	FBTVector y;
	FBMult(y, z, x);

	FBMatrix lMatrix;
	Compose(lMatrix, x, y, z);

	// XXX if above were corrected, following would not be needed
	FBMatrix lRotX180;
	FBRotationToMatrix(lRotX180, FBRVector(180, 0, 0));
	FBMatrixMult(lMatrix, lMatrix, lRotX180);
	//lMatrix = lMatrix * lRotX180;

	FBRVector lRotation;
	FBMatrixToRotation(lRotation, lMatrix);

	pRotation[0] = lRotation[0];
	pRotation[1] = lRotation[1];
	pRotation[2] = lRotation[2];
}
