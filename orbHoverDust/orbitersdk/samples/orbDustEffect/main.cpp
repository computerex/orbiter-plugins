#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include "DUSTMANAGER.h"

DUSTMANAGER g_DM;
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
		if (!g_DM.hasHovers(vc[i]->hook))
		{
			vc.erase(it);
		}
	}
}
DLLCLBK void opcPreStep(double mjd, double simdt, double simt)
{
	g_DM.PreStep();
	prune(g_DM.atList);
}
DLLCLBK void opcPostStep(double mjd, double simdt, double simt)
{
}
