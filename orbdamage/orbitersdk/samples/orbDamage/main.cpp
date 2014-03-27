#define  STRICT
#define  ORBITER_MODULE
#include <orbitersdk.h>
#include "VOBJ.h"
#include "DMMANAGER.h"

DMMANAGER DM_G;
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	DM_G.PreStep();
}
DLLCLBK void opcPostStep(double simt, double simdt, double mjd)
{
	DM_G.PostStep();
}