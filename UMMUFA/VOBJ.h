#ifndef _H_VOBJ_AGAIN_ELYSA_COMPUTEREX_4EVR_
#define _H_VOBJ_AGAIN_ELYSA_COMPUTEREX_4EVR_

#include <orbitersdk.h>
#include <vector>
#include <UMmuSDK.h>
#include "HudScroll.h"
#include "LOGGER.h"
#include <stdarg.h>

typedef std::vector<CREWMEMBERDEF> CrewMemberDef;

class VOBJ
{
public:
	VOBJ(CrewMemberDef b, OBJHANDLE h)
	{
		hook           = h;
		init(b);
		hudprint       = HUDSCROLL(4);
		dock           = 0;
		UmmuSlot       = 0;
	}
	VOBJ()
	{
		hook           = 0;
		hudprint       = HUDSCROLL(4);
		dock           = 0;
		UmmuSlot       = 0;
	}
protected:
	void init(CrewMemberDef def);
public:
	void init(CrewMemberDef b, OBJHANDLE h);
	int  TimeStep(void);
	int  nextlock();
	char *nextslot();
	int  eva();
public:
	UMMUCREWMANAGMENT    crew;
	OBJHANDLE            hook;
	UINT                 dock;
	DWORD                UmmuSlot;
	HUDSCROLL            hudprint;
};
void logwrite(const char * fmt, ...);
#endif
