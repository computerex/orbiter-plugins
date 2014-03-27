#ifndef _H_ORB_HOVER_DUST_COMPUTEREX
#define _H_ORB_HOVER_DUST_COMPUTEREX

#include <orbitersdk.h>
#include <vector>

class VOBJ
{
public:
	VOBJ(OBJHANDLE v) : hook(v)
	{
		VESSEL * vessel = oapiGetVesselInterface(v);
		DWORD inx = vessel->GetGroupThrusterCount(THGROUP_HOVER);
		for (int i = 0; i < inx; i++)
		{
			pHandle.push_back(NULL);
			LP_Time.push_back(0);
		}
	}
	VOBJ()
	{
		hook      = 0;
	}
	std::vector<PSTREAM_HANDLE> pHandle;
	std::vector<double> LP_Time;
	OBJHANDLE hook;
};
#endif
