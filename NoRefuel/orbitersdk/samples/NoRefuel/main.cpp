#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include "NoRefuel.h"
#include "EHUD.h"

std::vector<VOBJ*> vov;
BOOL Enabled = FALSE;
EHUD hud;

bool isAttended(OBJHANDLE h)
{
	for (int i = 0; i < vov.size(); i++)
	{
		if (vov[i]->hook == h )
			return true;
	}
	return false;
}
void Prune()
{
	if (vov.empty())
		return;
	std::vector<VOBJ*>::iterator it = vov.begin() + 1;
	for (int i = 0; i < vov.size(); i++)
	{
		it = vov.begin()+i;
		if (!oapiIsVessel(vov[i]->hook))
		{
			delete vov[i];
			vov.erase(it);
		}
	}
}
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	hud.TimeStep();
	Prune();
	static double LastInputTime=oapiGetSimTime();
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		OBJHANDLE hVes = oapiGetVesselByIndex(i);
		if (!isAttended(hVes))
			vov.push_back(new VOBJ(hVes));
	}
	if (LastInputTime < oapiGetSimTime())
	{
		if (GetKeyState(VK_TAB) & 0x800)
		{
			if (GetKeyState(0x46) &0x800)
			{
				if (Enabled==TRUE)
				{
					Enabled=FALSE;
					hud.Add(1,NULL,hud._D(0.1,1,0.3,1,1,_V(1,0,0),"Auto-Refuel: ON"));
				}
				else
				{
					Enabled=TRUE;
					hud.Add(1,NULL,hud._D(0.1,1,0.3,1,1,_V(1,0,0),"Auto-Refuel: OFF"));
				}
				LastInputTime=oapiGetSimTime()+1;
			}
		}
	}
	if (Enabled==FALSE)
		return;
	for (int i = 0; i < vov.size(); i++)
	{
		vov[i]->TimeStep();
	}
}
