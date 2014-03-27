#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include "BOOMMANAGER.h"

BOOMMANAGER g_BM;
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
	g_BM.TimeStep();
	prune(g_BM.atList);
}