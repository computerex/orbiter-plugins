#define  STRICT
#define  ORBITER_MODULE
#include <orbitersdk.h>
#include "ParticleManager.h"

ParticleManager P_MNG;
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	P_MNG.Prune();
	P_MNG.TimeStep();
}
