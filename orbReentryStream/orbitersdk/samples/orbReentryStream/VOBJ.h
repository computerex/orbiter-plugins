#ifndef _H_COMPUTEREX_VOBJ_JUST_AS_ALWAYS
#define _H_COMPUTEREX_VOBJ_JUST_AS_ALWAYS

#define DEFAULT_FLUX 79588461.422761

#include "PVISUAL.h"

class VOBJ
{
public:
	VOBJ() : hook(0), flux(DEFAULT_FLUX), dir(_V(0,1,0)) {}
	VOBJ(OBJHANDLE h, double f, std::vector<PVISUAL*> vi, VECTOR3 d)
	{
		hook    = h;
		flux    = f;
		streams = vi;
		dir     = d;
	}
	OBJHANDLE hook;
	double flux;
	VECTOR3 dir;
	THRUSTER_HANDLE th;
	std::vector<PVISUAL*> streams;
};
#endif