#define STRICT
#define ORBITER_MODULE

#include <stdio.h>
#include <windows.h>
#include <orbitersdk.h>
#include "FlyByMFD.h"
#include <vector>
#include "Camera.h"

#define YELLOW RGB(255, 255, 0)
#define WHITE RGB(255, 255, 255)
#define BLUE RGB(0, 0, 255)
#define GREY RGB(160, 160, 160)

int g_MFDmode;
std::vector<MFDData*> g_MFDData;
FlyByMFD *g_MFD     = NULL;
OBJHANDLE LVessel   = NULL;
int CAM_MODE        = CAM_NORMAL;
DLLCLBK void opcDLLInit(HINSTANCE hDLL)
{
	static char *name = "Fly By MFD";
	MFDMODESPEC spec;
	spec.name   = name;
	spec.key    = OAPI_KEY_F;
	spec.msgproc = FlyByMFD::MsgProc;
	g_MFDmode = oapiRegisterMFDMode(spec);
}
DLLCLBK void opcDLLExit(HINSTANCE hDLL)
{ 
	oapiUnregisterMFDMode(g_MFDmode);
}
DLLCLBK void opcPreStep(double simt, double simdt)
{
	if (!g_MFD)
		return;

	OBJHANDLE hCamRelv = oapiGetVesselByIndex(g_MFD->data->CamRelV);
	if (!hCamRelv)
		g_MFD->ConsumeKeyBuffered(OAPI_KEY_LBRACKET);
}
FlyByMFD::FlyByMFD(DWORD w, DWORD h, VESSEL *vessel): MFD(w,h,vessel)
{
	bool found = false;
	OBJHANDLE vHandle = vessel->GetHandle();
	for (int i = 0; i < g_MFDData.size(); i++)
	{
		if (vHandle == g_MFDData[i]->hVessel)
		{
			this->data=g_MFDData[i];
			found = true;
			break;
		}
	}
	if (!found)
	{
		MFDData * tdata = new MFDData(vHandle,0,_V(0,0,0),_V(0,0,0),FALSE);
		g_MFDData.push_back(tdata);
		this->data=tdata;
	}
    g_MFD=this;
}
FlyByMFD::~FlyByMFD()
{
	g_MFD=NULL;
}
bool VectorInput (void *id, char *str, void *user_data)
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
bool FlyByMFD::ConsumeKeyBuffered(DWORD Key)
{
	switch(Key)
	{
	case OAPI_KEY_LBRACKET:
		{
			if (data->CamRelV+1 >= oapiGetVesselCount())
				data->CamRelV = 0;
			else
				data->CamRelV++;
			return true;
		}
	case OAPI_KEY_P:
		{
			oapiOpenInputBox ("Camera Pos: [x y z]", VectorInput, 0, 20, &(data->CamPos));
			return true;
		}
	case OAPI_KEY_V:
		{
			oapiOpenInputBox ("Camera Vel: [x y z]", VectorInput, 0, 20, &(data->CamSpeed));
			return true;
		}
	case OAPI_KEY_F:
		{
			ActivateFlyBy();
			return true;
		}
	case OAPI_KEY_R:
		{
			Revert();
			return true;
		}
	case OAPI_KEY_RBRACKET:
		{
			if (data->Tracking==TRUE)
				data->Tracking=FALSE;
			else
				data->Tracking=TRUE;
			return true;
		}
	}
	return false;
}
bool FlyByMFD::ConsumeButton (int bt, int event)
{
	if (!(event & PANEL_MOUSE_LBDOWN)) return false;
	static const DWORD btkey[6] = { OAPI_KEY_P, OAPI_KEY_V, OAPI_KEY_F, OAPI_KEY_R, OAPI_KEY_LBRACKET, OAPI_KEY_RBRACKET };
	if (bt < 6) return ConsumeKeyBuffered (btkey[bt]);
	else return false;
}
char *FlyByMFD::ButtonLabel (int bt)
{
	char *label[6] = {"POS", "VEL", "FLY", "REV", "TGT", "TRK"};
	return (bt < 6 ? label[bt] : 0);
}
void FlyByMFD::Update(HDC hDC)
{
	Title(hDC,"Fly By View MFD");
	char cbuf[255];

	SetTextColor(hDC,YELLOW);
	OBJHANDLE hTarget = oapiGetVesselByIndex(data->CamRelV);
	sprintf_s(cbuf,"Camera Rel: %s",VESSEL(hTarget).GetName());
	TextOut(hDC,10,20,cbuf,strlen(cbuf));
	SetTextColor(hDC,WHITE);
	sprintf_s(cbuf,"Camera Pos: %.2f %.2f %.2f",data->CamPos.x, data->CamPos.y, data->CamPos.z);
	TextOut(hDC,10,40,cbuf,strlen(cbuf));
	SetTextColor(hDC,YELLOW);
	sprintf_s(cbuf,"Camera Vel: %.2f %.2f %.2f",data->CamSpeed.x, data->CamSpeed.y, data->CamSpeed.z);
	TextOut(hDC,10,60,cbuf,strlen(cbuf));
	sprintf_s(cbuf,"Err");
	if (data->Tracking==TRUE)
	    sprintf_s(cbuf,"Tracking  : Yes");
	else if (data->Tracking==FALSE)
	{
		SetTextColor(hDC,GREY);
		sprintf_s(cbuf,"Tracking  : No");
	}
	TextOut(hDC,10,80,cbuf,strlen(cbuf));
}
void FlyByMFD::ActivateFlyBy(void)
{
	CAM_MODE = CAM_FLYBY;
	LVessel  = data->hVessel;
	VESSEL * v = oapiGetVesselInterface(oapiGetVesselByIndex(data->CamRelV));
	VESSELSTATUS vs; v->GetStatus(vs);

    VECTOR3 Pos;
    oapiGetGlobalPos(v->GetHandle(), &Pos);
    v->Global2Local(Pos,Pos);
	Pos.x = Pos.x+data->CamPos.x;
	Pos.y = Pos.y+data->CamPos.y;
    Pos.z = Pos.z+data->CamPos.z;
    v->Local2Rel(Pos,Pos);

	VECTOR3 rofs;
	v->GlobalRot(data->CamSpeed,rofs);
	vs.rpos = Pos;
    vs.rvel.x+=rofs.x;
	vs.rvel.y+=rofs.y;
	vs.rvel.z+=rofs.z;
	vs.status = 0;
    OBJHANDLE cam = oapiCreateVessel("camera_CMFD", "camera", vs);
	CAMERA * cm = (CAMERA*)oapiGetVesselInterface(cam);
	if (data->Tracking==TRUE)
		cm->hTarget=oapiGetVesselByIndex(data->CamRelV);
	else
		cm->hTarget=NULL;
	oapiSetFocusObject(cam);
}
void FlyByMFD::Revert(void)
{
	CAM_MODE = CAM_NORMAL;
	if (LVessel)
		oapiSetFocusObject(LVessel);
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		if (!strcmp("camera_CMFD",VESSEL(oapiGetVesselByIndex(i)).GetName()))
		{
			oapiDeleteVessel(oapiGetVesselByIndex(i));
		}
	}
}
int FlyByMFD::MsgProc(UINT msg, UINT mfd, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case OAPI_MSG_MFD_OPENED:
		return (int)(new FlyByMFD(LOWORD(wParam),HIWORD(wParam),(VESSEL*)lParam));
	}
	return 0;
}