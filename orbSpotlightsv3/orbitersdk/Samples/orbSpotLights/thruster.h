#ifndef _H_COMPUTEREX_ORBITER_SPOTLIGHTS_
#define _H_COMPUTEREX_ORBITER_SPOTLIGHTS_

#include <orbitersdk.h>

class SPOTLIGHT
{
public:
	SPOTLIGHT() : w(0), l(0) , hHandle(0), lvl(0){};
	SPOTLIGHT(double dw, double dl, double lv, VECTOR3 vpos, VECTOR3 vdir, char * tex)
	{
		w = dw, l = dl, lvl = lv;
		pos = vpos, dir = vdir;
		hHandle=0;
		sprintf(texture,"%s",tex);
	}
	double w, l, lvl;
	VECTOR3 pos, dir;
	THRUSTER_HANDLE hHandle;
	char texture[255];
};
class BEACON 
{
public:
	BEACON(BEACONLIGHTSPEC beac)
	{
		beacon=beac;
	}
	BEACONLIGHTSPEC beacon;
};
#endif
