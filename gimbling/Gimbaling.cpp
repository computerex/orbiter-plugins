#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>

VECTOR3 nrmvec3(VECTOR3 v)
{
    double l = length(v);
    return _V(v.x/l,v.y/l,v.z/l);
}
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
     static double       PrintTime = 0;
     static NOTEHANDLE   hNote;
     static bool         init      = false;
     static bool         active    = false;
     static double       linpt     = 0;
     if (!init)
     {
         init=true;
         hNote=oapiCreateAnnotation(true,1,_V(1,0,0));
         oapiAnnotationSetPos(hNote,0.4,0.2,1,1);
     }
     if (GetKeyState(VK_LCONTROL) & 0x800)
     {
              // key 'G'
          if (GetKeyState(0x47) & 0x800 && (oapiGetSimTime()-linpt) > 1 )
          {
              if (active)
                  active=false;
              else
                  active=true;
              PrintTime = oapiGetSimTime()+3;
              linpt     = oapiGetSimTime();
          }
     }
     if (PrintTime > oapiGetSimTime())
         if (active)
             oapiAnnotationSetText(hNote, "Gimbling On");
         else
             oapiAnnotationSetText(hNote, "Gimbling Off");
     else
         oapiAnnotationSetText(hNote, "");
     if (!active)
          return;
     VESSEL *   vessel = oapiGetFocusInterface();
     VECTOR3    dir;
     OBJHANDLE  thruster;
     double     cLevel = vessel->GetControlSurfaceLevel(AIRCTRL_ELEVATOR);
     double     dLevel = vessel->GetControlSurfaceLevel(AIRCTRL_RUDDER);
     for (int i = 0; i < vessel->GetGroupThrusterCount(THGROUP_MAIN); i++)
     {
         thruster = vessel->GetGroupThruster(THGROUP_MAIN, i);
         dir.x=-dLevel;
         dir.y=-cLevel;
         dir.z=1-cLevel;
         dir  = nrmvec3(dir);
         vessel->SetThrusterDir(thruster,dir);
     }
}