#ifndef _H_SONIC_BOOM_ORBITER_COMPUTEREX_VOBJ_POWER
#define _H_SONIC_BOOM_ORBITER_COMPUTEREX_VOBJ_POWER

#include <orbitersdk.h>
#include <vector>
#include "PVISUAL.h"

class VOBJ
{
public:
	VOBJ(OBJHANDLE v, std::vector<PVISUAL*> ps, char * s)
	{
		hook     = v;
		streams  = ps;
		sprintf(sound,"%s",s);
		th       = 0;
		bk_mach  = false;
		LP_Time  = 0;
		duration = 5;
		shiver   = false;
	}
	VOBJ ()
	{
		th       = 0;
		hook     = 0;
		sprintf(sound,"Sound/Vessel/sonicbang.wav");
		bk_mach  = false;
		LP_Time  = 0;
		duration = 5;
		shiver   = false;
	}
	OBJHANDLE hook;
	std::vector<PVISUAL*> streams;
	char sound[255];
	THRUSTER_HANDLE th;
	bool bk_mach;
	double LP_Time;
	double duration;
	bool shiver;
};
#endif
