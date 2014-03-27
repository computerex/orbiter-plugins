#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include "VOBJ.h"
#include <vector>
#include "Parser.h"

std::vector<VOBJ*> atList;
bool init = false;
double dThresh = 0.2, dAngThresh=0.5;
bool isAttended(OBJHANDLE hVessel)
{
	for (int i = 0; i < atList.size(); i++)
	{
		if (!atList[i])
			continue;
		if (!oapiIsVessel(atList[i]->hook))
			continue;
		if (atList[i]->hook == hVessel)
			return true;
	}
	return false;
}
void prune(std::vector<VOBJ*> &vc)
{
	if (vc.empty())
		return;
	std::vector<VOBJ*>::iterator it = vc.begin() + 1;
	for (int i = 0; i < vc.size(); i++)
	{
		it = vc.begin()+i;
		if (!oapiIsVessel(vc[i]->hook))
		{
			vc.erase(it);
			continue;
		}
	}
}
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	if (!init)
	{
		init=true;
		PARSER g_Parser("Modules/Plugin/VCollision.cfg");
		g_Parser.read();
		if (g_Parser.Double("DockingMaxVel"))
			dThresh=g_Parser.Double("DockingMaxVel");
		if (g_Parser.Double("AngularMaxVel"))
			dAngThresh=g_Parser.Double("AngularMaxVel");
	}
	prune(atList);
	OBJHANDLE hVessel = 0;
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		hVessel = oapiGetVesselByIndex(i);
		if (isAttended(hVessel))
			continue;
		atList.push_back(new VOBJ(hVessel, dThresh, dAngThresh));
	}
	for (int i = 0; i < atList.size(); i++)
	{
		atList[i]->TimeStep();
	}
}