#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>

DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
   OBJHANDLE hVes = NULL;
   VESSEL *  v    = NULL;
   for (int i = 0; i < oapiGetVesselCount(); i++)
   {
        hVes=oapiGetVesselByIndex(i);
        v=oapiGetVesselInterface(hVes);
        if (!v->GroundContact())
            continue;
            v->Undock(ALLDOCKS);
   } 
}
