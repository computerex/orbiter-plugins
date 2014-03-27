#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include "Parachute.h"

Parachute::Parachute(OBJHANDLE hVessel, int flightmodel) : VESSEL2(hVessel, flightmodel)
{
}
void Parachute::clbkSetClassCaps (FILEHANDLE cfg)
{
	SetSize (3.5);
	SetEmptyMass (500.0);
}
void Parachute::clbkPreStep(double simt, double simdt, double mjd)
{
	if (GroundContact())
		oapiDeleteVessel(GetHandle());
}
void Parachute::clbkDrawHUD(int mode, const HUDPAINTSPEC * hps, HDC hDC)
{
	VESSEL2::clbkDrawHUD(mode, hps, hDC);
}
DLLCLBK VESSEL *ovcInit (OBJHANDLE hvessel, int flightmodel)
{
	return new Parachute (hvessel, flightmodel);
}
DLLCLBK void ovcExit (VESSEL *vessel)
{
	if (vessel) delete (Parachute*)vessel;
}
