#ifndef _H_COMPUTEREX_SPOTLIGHTS_VOBJ
#define _H_COMPUTEREX_SPOTLIGHTS_VOBJ

#include "orbitersdk.h"
#include "thruster.h"
#include <vector>


class VOBJ
{
public:
	VOBJ() : hook(0)
	{
		lights.clear();
		beacons.clear();
	}
	VOBJ(OBJHANDLE v, std::vector<SPOTLIGHT*> lv, std::vector<BEACON*> bv) 
	{
		hook    = v;
		lights  = lv;
		beacons = bv;
	}
	double dot(VECTOR3 a, VECTOR3 b)
	{
		return a.x*b.x+a.y*b.y+a.z*b.z;
	}
	OBJHANDLE star()
	{
		DWORD inx = oapiGetGbodyCount();
		OBJHANDLE hBody;
		for (int i = 0; i < inx; i++)
		{
			hBody = oapiGetGbodyByIndex(i);
			if (oapiGetObjectType(hBody)==OBJTP_STAR)
				return hBody;
		}
		return NULL;
	}
	void status(bool power)
	{
		if (!oapiIsVessel(hook))
			return;
		VESSEL * v = oapiGetVesselInterface(hook);
		for (int i = 0; i < lights.size(); i++)
		{
			if (power)
			{
				v->SetThrusterLevel(lights[i]->hHandle, lights[i]->lvl);
			}else
				v->SetThrusterLevel(lights[i]->hHandle, 0);
		}
		for (int i = 0; i < beacons.size(); i++)
		{
				beacons[i]->beacon.active=power;
		}
	}
	bool night(void)
	{
		VECTOR3 rppos, rspos;
		VESSEL * v = oapiGetVesselInterface(hook);
		v->GetRelativePos(v->GetSurfaceRef(), rppos);
		v->GetRelativePos(star(), rspos);
		if (dot(rppos, rspos) > 0)
			return true;
		return false;
	}
	bool active()
	{
		VESSEL * v = oapiGetVesselInterface(hook);
		if (lights.size()<1 && beacons.size()<1)
			return false;
		if (lights.size() > 1)
			return v->GetThrusterLevel(lights[0]->hHandle);
		else
			return beacons[0]->beacon.active;
		return false;
	}
	std::vector<SPOTLIGHT*> lights;
	std::vector<BEACON*> beacons;
	OBJHANDLE hook;
};
#endif