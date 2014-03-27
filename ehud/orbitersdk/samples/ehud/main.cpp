#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include "EHUD.h"
#include "Parser.h"

HUDOPTIONS hoptions;
BOOL       Init=FALSE;
EHUD       HUD;
VESSEL *   vessel=NULL;
VECTOR3    A, W, F;
double     ApA, PeA;
OBJHANDLE  hBody=NULL;
char cbuf[255];
void LoadHudOptions(void)
{
	PARSER gpr("Modules/Plugin/ehud.cfg");
	gpr.read();
	hoptions.color=gpr.Vector3("HUDCOLOR");
	hoptions.size=gpr.Double("TEXTSIZE");
	hoptions.startx=gpr.Double("StartX");
	hoptions.starty=gpr.Double("StartY");
	hoptions.xofs=gpr.Double("X_OFS");
	hoptions.yofs=gpr.Double("Y_OFS");
}
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
}
DLLCLBK void opcPostStep(double simt, double simdt, double mjd)
{
	if (Init==FALSE)
	{
		Init=TRUE;
		LoadHudOptions();
	}
	HUD.clear();
	vessel=oapiGetFocusInterface();
	vessel->GetForceVector(F);
	vessel->GetWeightVector(W);
	A = (F-W)/vessel->GetMass();
	hBody = vessel->GetSurfaceRef();
	vessel->GetApDist(ApA);
	vessel->GetPeDist(PeA);
	ApA -=oapiGetSize(hBody);
	PeA -=oapiGetSize(hBody);

	double y = hoptions.starty, x = hoptions.startx;
	sprintf(cbuf, "Accel: %.2f m/s^2", A.z);
	HUD.Add(0.0000001, vessel->GetHandle(), HUD._D(x, 1, y, 1, hoptions.size, hoptions.color, cbuf));
	y+=hoptions.yofs;
	x+=hoptions.xofs;
	sprintf(cbuf, "G: %.2f", A.y/9.8);
	HUD.Add(0.0000001, vessel->GetHandle(), HUD._D(x, 1, y, 1, hoptions.size, hoptions.color, cbuf));
	y+=hoptions.yofs;
	x+=hoptions.xofs;
	sprintf(cbuf, "ApA: %.4f km", ApA/1000);
	HUD.Add(0.0000001, vessel->GetHandle(), HUD._D(x, 1, y, 1, hoptions.size, hoptions.color, cbuf));
	y+=hoptions.yofs;
	x+=hoptions.xofs;
	sprintf(cbuf, "PeA %.4f km", PeA/1000);
	HUD.Add(0.0000001, vessel->GetHandle(), HUD._D(x, 1, y, 1, hoptions.size, hoptions.color, cbuf));
	y+=hoptions.yofs;
	x+=hoptions.xofs;
	HUD.TimeStep();
}