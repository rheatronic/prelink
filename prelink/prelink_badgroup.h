#ifndef _PRELINK_BADGROUP_H_
#define _PRELINK_BADGROUP_H_

/************************************************
 * Manage Group of discovered bad models
 ************************************************/

#include "bitflix/bitflix_debug.h"

#include <fbsdk/fbsdk.h>
#include <windows.h>

bool BadModel(HFBModel pModel);

class BadGroup {
public:
	static char* GROUP_NAME;
	static CRITICAL_SECTION GROUP_SECTION;
	static int SECTION_REFCOUNT;

	static void CriticalSectionInitialize() 
	{
		if( !SECTION_REFCOUNT )
			InitializeCriticalSection(&GROUP_SECTION);
		SECTION_REFCOUNT++;
	}

	static void CriticalSectionFinalize()
	{
		SECTION_REFCOUNT--;
		if( !SECTION_REFCOUNT )
			DeleteCriticalSection(&GROUP_SECTION);
	}

	static HFBGroup find(HFBScene pScene)
	{
		for(int i=0;i<pScene->Groups.GetCount();i++)
		{
			FBString lName = pScene->Groups[i]->Name;
			if(lName == GROUP_NAME)
				return pScene->Groups[i];
		}
		return 0;
	}

	static HFBGroup intern(HFBScene pScene)
	{ 
		HFBGroup lGroup = find(pScene);
		if (lGroup)
			return lGroup;

		HFBGroup lNewGroup = new FBGroup(GROUP_NAME);
		return lNewGroup;
	}

	static bool add(HFBScene pScene, HFBModel pModel)
	{
		EnterCriticalSection(&BadGroup::GROUP_SECTION);
		HFBGroup lGroup = intern(pScene);
		bool retval;
		if(lGroup && !lGroup->Contains(pModel)) 
		{
			lGroup->Items.Add(pModel);
			FBTrace("Prelink: Model %s has w/non-zero pivot or offset, transform not sent, added to Group '%s'.\n", pModel->Name.AsString(), BadGroup::GROUP_NAME);
			retval = true;
		} else
			retval = false;
		LeaveCriticalSection(&BadGroup::GROUP_SECTION);
		return retval;
	}

	static void remove(HFBScene pScene, HFBModel pModel)
	{
		EnterCriticalSection(&BadGroup::GROUP_SECTION);
		HFBGroup lGroup = intern(pScene);
		if(lGroup && lGroup->Contains(pModel)) 
		{
			lGroup->Items.Remove(pModel);
		}
		LeaveCriticalSection(&BadGroup::GROUP_SECTION);
	}

	static void update(HFBScene pScene)
	{
		HFBGroup lGroup = intern(pScene);
		if(lGroup) 
		{
			for(int i=0;i<lGroup->Items.GetCount();i++)
			{
				HFBModel lModel = dynamic_cast<HFBModel>(lGroup->Items.GetAt(i));
				if( !BadModel(lModel) )
					lGroup->Items.Remove(lModel);
			}
		}
	}
};

#endif
