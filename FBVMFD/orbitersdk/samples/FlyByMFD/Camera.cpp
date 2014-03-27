#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include "Camera.h"
#include <math.h>

double sqr(double x)
{
    return x * x;
}
VECTOR3 nvec(VECTOR3 v)
{
    double l = sqrt(sqr(v.x)+sqr(v.y)+sqr(v.z));
    return _V(v.x/l,v.y/l,v.z/l);
}
VECTOR3 vrec2sphv(VECTOR3 v)
{
   VECTOR3 t;
   double a1,a2,h,m,n,r;
   r=1;
   a1=v.y/sqrt(sqr(v.x)+sqr(v.z));
   a2=v.z/v.x;
   h=sqrt(sqr(v.x)+sqr(v.y)+sqr(v.z))-r;
   m=atan(a1);
   n=atan(a2);
   if((v.x>0)&&(v.z>0))n=n;
   if((v.x<0)&&(v.z>0))n=n+PI;
   if((v.x<0)&&(v.z<0))n=n-PI;
   if((v.x>0)&&(v.z<0))n=n;
   t.x=m;
   t.y=n;
   t.z=h;
   return t;
}
VECTOR3 calc(CAMERA * cm)
{
    if (!cm)
       return _V(0,0,0);
    if (!cm->hTarget)
       return _V(0,0,0);
    
    VECTOR3 tgtPos, tgtrPos, tgtrVel, tgtrDir;
    OBJHANDLE thetgt = cm->hTarget;
    
    oapiGetGlobalPos(thetgt,&tgtPos);
    cm->GetRelativePos(thetgt,tgtrPos);
    cm->GetRelativeVel(thetgt,tgtrVel);
    cm->Global2Local(tgtPos,tgtrDir);
    tgtrDir=nvec(tgtrDir);
    
    double ya,xa;
    VECTOR3 vv;
    vv=vrec2sphv(tgtrDir);
    ya=vv.x;
    xa=vv.y-PI/2;
    return _V(xa,ya,0);
}
bool DoubleInput (void *id, char *str, void *data)
{
    *(double*)data=atof(str);
    return true;
}
CAMERA::CAMERA(OBJHANDLE hVessel, int flightmodel) 
: VESSEL2(hVessel, flightmodel)
{
    hTarget = 0;
}
void CAMERA::clbkPreStep(double simt, double simdt, double mjd)
{
    if (!hTarget)
        return;
    VECTOR3 v = calc(this);
    oapiCameraSetCockpitDir(v.x,v.y,false);
}
int CAMERA::clbkConsumeDirectKey (char *kstate)
{
     if (KEYDOWN(kstate,OAPI_KEY_NUMPAD1))
     {
          oapiCameraSetAperture(oapiCameraAperture()-0.001);
          RESETKEY(kstate,OAPI_KEY_NUMPAD1);
          return 1;
     }
     if (KEYDOWN(kstate,OAPI_KEY_NUMPAD2))
     {
          oapiCameraSetAperture(oapiCameraAperture()+0.001);
          RESETKEY(kstate,OAPI_KEY_NUMPAD2);
          return 1;
     }
     return 0;
}
int CAMERA::clbkConsumeBufferedKey(DWORD key, bool down, char * kstate)
{
     if (!down) return 0;
     
     switch(key)
     {
     }
     return 0;
}
void CAMERA::clbkDrawHUD (int mode, const HUDPAINTSPEC *hps, HDC hDC)
{
    VESSEL2::clbkDrawHUD ( mode, hps, hDC);
    char cbuf[255];
}
    
DLLCLBK VESSEL *ovcInit (OBJHANDLE hvessel, int flightmodel)
{
	return new CAMERA (hvessel, flightmodel);
}
DLLCLBK void ovcExit (VESSEL *vessel)
{
	if (vessel) delete (CAMERA*)vessel;
}
