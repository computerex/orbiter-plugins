#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include "Camera.h"
#include <math.h>
#include <string>
#include <vector>

void Tokenize(const std::string& str,
			  std::vector<std::string>& tokens,
			  const std::string& delimiters = " ")
{
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
    {
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}
inline double sqr(double x)
{
	return x*x;
}
inline VECTOR3 nvec(VECTOR3 v)
{
    double l = sqrt(sqr(v.x)+sqr(v.y)+sqr(v.z));
    return _V(v.x/l,v.y/l,v.z/l);
}
bool InputVector (void *id, char *str, void *user_data)
{
	char cbuf[255];
	VECTOR3 *vTemp = (VECTOR3*)user_data;
	sprintf_s(cbuf,"%s",str);
	std::string svector = cbuf;
	std::vector<std::string> tokens;
	Tokenize(svector,tokens);
	if (tokens.size() < 3)
		return false;
	vTemp->x=atof(tokens[0].c_str());
	vTemp->y=atof(tokens[1].c_str());
	vTemp->z=atof(tokens[2].c_str());
	return true;
}
bool InputDouble (void *id, char *str, void *data)
{
    *(double*)data=atof(str);
    return true;
}
bool InputTarget (void * id, char * str, void * data)
{
	if (strlen(str)==0)
		return false;
	CAMERA * cam = (CAMERA*)data;
	cam->setTarget(NULL);
	OBJHANDLE hTarget = oapiGetObjectByName(str);
	if (hTarget==NULL)
		return false;
	cam->setTarget(hTarget);
	return true;
}
VECTOR3 GetRelativeDir(OBJHANDLE obj, OBJHANDLE ref)
{
	if (obj == NULL || ref == NULL)
		return _V(0,0,0);
	VECTOR3 pos, dir;
	oapiGetGlobalPos(ref, &pos);
	VESSEL(obj).Global2Local(pos, dir);
	return nvec(dir);
}
void SetStateRelative(OBJHANDLE obj, OBJHANDLE ref, VECTOR3 rpos, VECTOR3 rvel)
{
	if (obj == NULL || ref == NULL)
		return;

	VECTOR3 pos;
	oapiGetGlobalPos(ref, &pos);
	VESSEL * v = oapiGetVesselInterface(ref);
	v->Global2Local(pos, pos);
	pos+=rpos;
	v->Local2Rel(pos, pos);

	VECTOR3 vel;
	v->GlobalRot(rvel, vel);
	VESSELSTATUS vs; v->GetStatus(vs);

	vs.status=0;
	vs.rpos=pos;
	vs.rvel+=vel;
	vs.vrot=_V(0,0,0);

	v = oapiGetVesselInterface(obj);
	v->DefSetState(&vs);
}
CAMERA::CAMERA(OBJHANDLE hVessel, int flightmodel) 
: VESSEL2(hVessel, flightmodel)
{
    hTarget = NULL;
	track=false;
	ofs=0.1;
	panofs = 0.01;
	selectedView = 0;
	pan = _V(0, 0, 0);

	/*views.push_back(CM_CFG(_V(0, -12, 110), _V(0, 3, -5)));
	views.push_back(CM_CFG(_V(30, 0, 150), _V(0, 0, -5)));
	views.push_back(CM_CFG(_V(-30, 0, 150), _V(0, 0, -5)));
	views.push_back(CM_CFG(_V(0, 0, 100), _V(0, 0, -5)));
	views.push_back(CM_CFG(_V(0, 0, -100), _V(0, 0, 5)));
	views.push_back(CM_CFG(_V(0, 20, 70), _V(0, -3, -5)));*/

	pos = vel = _V(0, 0, 0);
}
void CAMERA::clbkSetClassCaps(FILEHANDLE cfg)
{
	VESSEL2::clbkSetClassCaps(cfg);

	int viewCount = 0;
	if (!oapiReadItem_int(cfg, "ViewCount", viewCount))
		return;
	CM_CFG view;
	char buffer[255];
	for (int i = 0; i < viewCount; i++)
	{
		sprintf(buffer, "View_%d_Pos", i+1);
		oapiReadItem_vec(cfg, buffer, view.pos);
		sprintf(buffer, "View_%d_Vel", i+1);
		oapiReadItem_vec(cfg, buffer, view.vel);
		views.push_back(view);
	}
	if (viewCount>0)
	{
		pos = views[selectedView].pos;
		vel = views[selectedView].vel;
	}
}
VECTOR3 tdir, tvofs, tpos; // global vars are faster
double tdist, tsize;
void CAMERA::clbkPreStep(double simt, double simdt, double mjd)
{
	if (hTarget!=NULL)
	{
		GetRelativePos(hTarget, tpos);
		tvofs = GetRelativeDir(this->GetHandle(), hTarget);
		if (track)
		{
			tdist=length(tpos);
			tsize=oapiGetSize(hTarget);
			if (tdist-tsize<camDist)
				camDist=tdist-tsize;
			tvofs = nvec(tvofs);
			SetCameraDefaultDirection(tvofs);
		}
		else
			tvofs = nvec(tvofs+pan);
		SetCameraOffset(_V(tvofs.x*camDist, tvofs.y*camDist, tvofs.z*camDist));
	}
}
int CAMERA::clbkConsumeDirectKey (char *kstate)
{
	if (hTarget==NULL)
		return 0;
     if (KEYDOWN(kstate, OAPI_KEY_1))
	 {
		 camDist+=ofs;
		 RESETKEY(kstate, OAPI_KEY_1);
		 return 1;
	 }
	 if (KEYDOWN(kstate, OAPI_KEY_2))
	 {
		 camDist-=ofs;
		 RESETKEY(kstate, OAPI_KEY_2);
		 return 1;
	 }
	 if (KEYDOWN(kstate, OAPI_KEY_LEFT))
	 {
		 pan = _V(pan.x-panofs/1000, pan.y, pan.z);
		 RESETKEY(kstate, OAPI_KEY_LEFT);
		 return 1;
	 }
	 if (KEYDOWN(kstate, OAPI_KEY_RIGHT))
	 {
		 pan = _V(pan.x+panofs/1000, pan.y, pan.z);
		 RESETKEY(kstate, OAPI_KEY_RIGHT);
		 return 1;
	 }
	 if (KEYDOWN(kstate, OAPI_KEY_UP))
	 {
		 pan = _V(pan.x, pan.y+panofs/1000, pan.z);
		 RESETKEY(kstate, OAPI_KEY_UP);
		 return 1;
	 }
	 if (KEYDOWN(kstate, OAPI_KEY_DOWN))
	 {
		 pan = _V(pan.x, pan.y-panofs/1000, pan.z);
		 RESETKEY(kstate, OAPI_KEY_DOWN);
	     return 1;
	 }
     return 0;
}
int CAMERA::clbkConsumeBufferedKey(DWORD key, bool down, char * kstate)
{
     if (!down) return 0;
     
     switch(key)
     {
	 case OAPI_KEY_D:
		 {
			 if (track)
				 track=false;
			 else
				 track=true;
			 pan = _V(0, 0, 0);
			 return true;
		 }
	 case OAPI_KEY_S:
		 {
			 SetCameraOffset(_V(0, 0, 0));
			 camDist=0;
			 oapiOpenInputBox("Enter target name", InputTarget, 0, 20, this);
			 return true;
		 }
	 case OAPI_KEY_P:
		 {
			 oapiOpenInputBox("Enter pos [x y z] [meters]", InputVector, 0, 20, &pos);
			 return true;
		 }
	 case OAPI_KEY_V:
		 {
			 oapiOpenInputBox("Enter vel [x y z] [meters]", InputVector, 0, 20, &vel);
			 return true;
		 }
	 case OAPI_KEY_J:
		 {
			 oapiOpenInputBox("Enter zoom ofs: [meters]", InputDouble, 0, 20, &ofs);
			 return true;
		 }
	 case OAPI_KEY_N:
		 {
			 oapiOpenInputBox("Enter pan offset: ", InputDouble, 0, 20, &panofs);
			 return true;
		 }
	 case OAPI_KEY_F:
		 {
			 if (hTarget==NULL)
				 return false;
			 SetCameraOffset(_V(0, 0, 0));
			 camDist=0;
			 SetStateRelative(this->GetHandle(), hTarget, pos, vel);
			 return true;
		 }
	 case OAPI_KEY_O:
		 {
			 views.push_back(CM_CFG(pos, vel));
			 selectedView = views.size()-1;
			 return true;
		 }
	 case OAPI_KEY_E:
		 {
			 selectedView++;
			 if (selectedView>=views.size())
				 selectedView=0;
			 pos = views[selectedView].pos;
			 vel = views[selectedView].vel;
			 return true;
		 }
     }
     return 0;
}
void CAMERA::clbkDrawHUD (int mode, const HUDPAINTSPEC *hps, HDC hDC)
{
    char cbuf[255];

	int width = hps->W/50;
	int height = hps->H/30;
	int y = height*6;
	int x = width/2;

	int len;

	if (track)
		len = sprintf(cbuf, "Tracking: Active");
	else
		len = sprintf(cbuf, "Tracking: Inactive");
	TextOut(hDC, x, y, cbuf, len);

	if (hTarget!=NULL)
	{
		char name[255];
		oapiGetObjectName(hTarget, name, 255);
		len = sprintf(cbuf, "Target  : %s", name);
	}
	else
		len = sprintf(cbuf, "Target  : None");

	y += height*0.8;
	TextOut(hDC, x, y, cbuf, len);

	y += height*0.8;
	len = sprintf(cbuf, "Pos     : %.2f %.2f %.2f", pos.x, pos.y, pos.z);
	TextOut(hDC, x, y, cbuf, len);

	y += height*0.8;
	len = sprintf(cbuf, "Vel     : %.2f %.2f %.2f", vel.x, vel.y, vel.z);
	TextOut(hDC, x, y, cbuf, len);

	y+= height*0.8;
	len = sprintf(cbuf, "Zoom    : %.2f", ofs);
	TextOut(hDC, x, y, cbuf, len);

	y += height*0.8; 
	len = sprintf(cbuf, "Panning : %.4f", panofs);
	TextOut(hDC, x, y, cbuf, len);

	y += height*0.8;
	len = sprintf(cbuf, "View    : (%d of %d)", selectedView+1, views.size());
	TextOut(hDC, x, y, cbuf, len);

	x = width/2;
	y += height*3;

	len = sprintf(cbuf, "Keyboard commands: " );
	TextOut(hDC, x, y, cbuf, len);

	y += height*1;

	len = sprintf(cbuf, "D       : toggle track");
	TextOut(hDC, x, y, cbuf, len);
	y+=height*0.8;
	len = sprintf(cbuf, "S       : target name");
	TextOut(hDC, x, y, cbuf, len);
	y+=height*0.8;
	len = sprintf(cbuf, "P       : rel. position");
	TextOut(hDC, x, y, cbuf, len);
	y+=height*0.8;
	len = sprintf(cbuf, "V       : rel. velocity");
	TextOut(hDC, x, y, cbuf, len);
	y+=height*0.8;
	len = sprintf(cbuf, "J       : set zoom");
	TextOut(hDC, x, y, cbuf, len);
	y+=height*0.8;
	len = sprintf(cbuf, "N       : set panning");
	TextOut(hDC, x, y, cbuf, len);
	y+=height*0.8;
	len = sprintf(cbuf, "F       : fly-by");
	TextOut(hDC, x, y, cbuf, len);
	y+=height*0.8;
	len = sprintf(cbuf, "O       : save view");
	TextOut(hDC, x, y, cbuf, len);
	y+=height*0.8;
	len = sprintf(cbuf, "E       : switch view");
	TextOut(hDC, x, y, cbuf, len);
	y+=height*0.8;
	len = sprintf(cbuf, "1       : zoom in");
	TextOut(hDC, x, y, cbuf, len);
	y+=height*0.8;
	len = sprintf(cbuf, "2       : zoom out");
	TextOut(hDC, x, y, cbuf, len);
	y+=height*0.8;
	len = sprintf(cbuf, "CTRL + H: Toggle HUD");
	TextOut(hDC, x, y, cbuf, len);
	y+=height*0.8;
	len = sprintf(cbuf, "Cursor  : panning");
	TextOut(hDC, x, y, cbuf, len);
}
    
DLLCLBK VESSEL *ovcInit (OBJHANDLE hvessel, int flightmodel)
{
	return new CAMERA (hvessel, flightmodel);
}
DLLCLBK void ovcExit (VESSEL *vessel)
{
	if (vessel) delete (CAMERA*)vessel;
}
