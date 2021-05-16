#include "prelink_badgroup.h"

char* BadGroup::GROUP_NAME = "Nonzero Pivot or Offset";

CRITICAL_SECTION BadGroup::GROUP_SECTION;

int BadGroup::SECTION_REFCOUNT = 0;

/************************************************
 *  Check models for pivots and offsets
 ************************************************/

bool BadModel(HFBModel pModel)
{
	char *lPropNames[] = { "Rotation Offset", "Rotation Pivot", "Scaling Offset", "Scaling Pivot", };
	for(int i=0;i<4;i++)
	{
		char* lPropName = lPropNames[i];
		HFBProperty lProp = pModel->PropertyList.Find(lPropName);
		if(lProp)
		{
			const double limit = 0.0000001;
			FBVector3d cVector;
			lProp->GetData(&cVector, sizeof(FBVector3d));
			if( cVector[0] > limit || cVector[0] < -limit ||
				cVector[1] > limit || cVector[1] < -limit ||
				cVector[2] > limit || cVector[2] < -limit )
			{
				return true;
			}
		}
	}
	return false;
}
