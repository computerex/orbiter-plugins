#define STRICT
#define ORBITER_MODULE
#include <orbitersdk.h>
#include "PARSER.h"
#include "EHUD.h"

HUDOPTIONS hops;
EHUD       HUD;
bool       init;
int        iTarget=0;
double     lInputTime=0;
OBJHANDLE  hTarget=NULL;
void LoadOptions()
{
	PARSER gpr("Modules/Plugin/kUndock.cfg");
	gpr.read();
	hops.color=gpr.Vector3("color");
	hops.size=gpr.Double("Size");
	hops.startx=gpr.Double("StartX");
	hops.starty=gpr.Double("StartY");
	hops.xofs=gpr.Double("XOFS");
	hops.yofs=gpr.Double("YOFS");
	init=true;
}
void nextTarget()
{
	if (iTarget+1 < oapiGetVesselCount())
		iTarget++;
	else
		iTarget=0;
}
void prevTarget()
{
	if (iTarget-1 < 0)
		iTarget=oapiGetVesselCount()-1;
	else
		iTarget--;
}
void undock()
{
	OBJHANDLE hVes = oapiGetVesselByIndex(iTarget);
	if (!oapiIsVessel(hVes))
		return;
	VESSEL(hVes).Undock(ALLDOCKS);
}
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	if (!init)
	{
		LoadOptions();
	}
	hTarget=oapiGetVesselByIndex(iTarget);
	if (!oapiIsVessel(hTarget))
	{
		nextTarget();
		return;
	}
	if (lInputTime<oapiGetSimTime())
	{
		if (GetKeyState(VK_F11) & 0x800)
		{
			nextTarget();
			lInputTime=oapiGetSimTime()+0.2;
		}
		if (GetKeyState(VK_F12) & 0x800)
		{
			prevTarget();
			lInputTime=oapiGetSimTime()+0.2;
		}
		if (GetKeyState(VK_ESCAPE) & 0x800)
		{
			undock();
			lInputTime=oapiGetSimTime()+0.2;
		}
	}
	HUD.clear();
	HUD.Add(0.001, NULL, HUD._D(hops.startx, 1, hops.starty, 1, hops.size, hops.color, VESSEL(hTarget).GetName()));
	HUD.TimeStep();
}