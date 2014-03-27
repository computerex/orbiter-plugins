#define STRCIT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include <windows.h>
#include "SWPhysicsMFD.h"
#include <vector>

std::vector<VOBJ*> g_data;
int mode;

VOBJ * GetFocusV()
{
	VESSEL * v = oapiGetFocusInterface();
	for (int i = 0; i < g_data.size(); i++)
	{
		if (v->GetHandle() == g_data[i]->hook)
			return g_data[i];
	}
	return NULL;
}
void Prune()
{
	if (g_data.empty())
		return;
	std::vector<VOBJ*>::iterator it = g_data.begin() + 1;
    for (int i = 0; i < g_data.size(); i++)
	{
		it = g_data.begin() + i;
		if (!oapiIsVessel(g_data[i]->hook))
		{
			delete g_data[i];
			g_data.erase(it);
		}
	}
}
bool GetDouble(void * id, char * str, void * data)
{
	*(double*)data=atof(str);
	return true;
}
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	Prune();
	for (int i = 0; i < g_data.size(); i++)
	{
		g_data[i]->Physics();
		if (g_data[i]->constant)
			g_data[i]->Propogate();
	}
}
DLLCLBK void opcDLLInit (HINSTANCE hDLL)
{
	static char *name = "Star Wars Physics MFD";
	MFDMODESPEC spec;
	spec.name    = name;
	spec.key     = OAPI_KEY_S;
	spec.msgproc = SWPMFD::MsgProc;
	mode = oapiRegisterMFDMode (spec);
}

DLLCLBK void opcDLLExit (HINSTANCE hDLL)
{
	oapiUnregisterMFDMode (mode);
	for (int i = 0; i < g_data.size(); i++)
		delete g_data[i];
}
SWPMFD::SWPMFD (DWORD w, DWORD h, VESSEL *vessel)
: MFD (w, h, vessel)
{
	bool found = false;
	for (int i = 0; i < g_data.size(); i++)
	{
		if (vessel->GetHandle()==g_data[i]->hook)
		{
			found=true;
			this->vbj=g_data[i];
			break;
		}
	}
	if (!found)
	{
		VOBJ * v = new VOBJ(vessel->GetHandle());
		g_data.push_back(v);
		this->vbj=v;
	}
	height=h/24;
	width=w/35;
}
SWPMFD::~SWPMFD ()
{
}
int SWPMFD::MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case OAPI_MSG_MFD_OPENED:
		return (int)(new SWPMFD (LOWORD(wparam), HIWORD(wparam), (VESSEL*)lparam));
	}
	return 0;
}
bool SWPMFD::ConsumeKeyBuffered (DWORD key)
{
	switch(key)
	{
	case OAPI_KEY_P:
		{
			vbj->Propogate();
			return true;
		}
	case OAPI_KEY_S:
		{
			oapiOpenInputBox("Enter vel. gain m/s : ",GetDouble, 0, 20, &vbj->wfac);
			return true;
		}
	case OAPI_KEY_T:
		{
			if (vbj->active)
				vbj->active=FALSE;
			else
				vbj->active=TRUE;
			return true;
		}
	case OAPI_KEY_C:
		{
			if (vbj->constant)
				vbj->constant=FALSE;
			else
				vbj->constant=TRUE;
			return true;
		}
	case OAPI_KEY_F:
		{
			VESSEL * v = oapiGetFocusInterface();
			VESSELSTATUS vs;
			v->GetStatus(vs);
			vs.status=0;
			VECTOR3 rofs;
			v->GlobalRot(_V(0,0,vbj->velocity),rofs);
			vs.rvel+=rofs;
			OBJHANDLE hVessel = oapiCreateVessel("blast","Blast", vs);
			VESSEL2 * blaster = (VESSEL2*)oapiGetVesselInterface(hVessel);
			double range = vbj->range;
			blaster->clbkConsumeBufferedKey(300, true, (char *)&range);
			return true;
		}
	case OAPI_KEY_R:
		{
			oapiOpenInputBox("Enter blaster range: ", GetDouble, 0, 20, &vbj->range);
			return true;
		}
	case OAPI_KEY_V:
		{
			oapiOpenInputBox("Enter blaster vel.: ", GetDouble, 0, 20, &vbj->velocity);
			return true;
		}
	case OAPI_KEY_D:
		{
			vbj->wfac*=-1;
			return 0;
		}
	case OAPI_KEY_G:
		{
			if (vbj->gravity)
				vbj->gravity=false;
			else
				vbj->gravity=true;
			return true;
		}
	}
	return false;
}

bool SWPMFD::ConsumeButton (int bt, int event)
{
	if (bt == 0)
		return ConsumeKeyBuffered(OAPI_KEY_P);

	if (!(event & PANEL_MOUSE_LBDOWN)) return false;

	switch(bt)
	{
	case 1:
		return ConsumeKeyBuffered(OAPI_KEY_S);
	case 2:
		return ConsumeKeyBuffered(OAPI_KEY_T);
	case 3:
		return ConsumeKeyBuffered(OAPI_KEY_C);
	case 4:
		return ConsumeKeyBuffered(OAPI_KEY_F);
	case 5:
		return ConsumeKeyBuffered(OAPI_KEY_R);
	case 6:
		return ConsumeKeyBuffered(OAPI_KEY_V);
	case 7:
		return ConsumeKeyBuffered(OAPI_KEY_D);
	case 8:
		return ConsumeKeyBuffered(OAPI_KEY_G);
	}
	return false;
}

char *SWPMFD::ButtonLabel (int bt)
{
	char *label[9] = {"GO", "SET", "TGL", "CON","FIRE", "RAN", "VEL", "DIR", "GRAV"};
	return (bt < 9 ? label[bt] : 0);
}
void SWPMFD::Update (HDC hDC)
{
	Title (hDC, "Starwars Physics MFD");

	double y = height*2;
	char cbuf[255];
	int len;

	len = sprintf(cbuf,"Vel. gain: %.2f meters",vbj->wfac);
	TextOut(hDC, width, y, cbuf, len); 
	y += height*3;

	if (vbj->active)
	{
		len = sprintf(cbuf,"SW Physics active");
		TextOut(hDC,width,y,cbuf,len);
		y+=height*1;
	}
	if (vbj->constant)
	{
		len = sprintf(cbuf,"Constant Propogation");
		TextOut(hDC,width, y, cbuf, len);
		y+=height*1;
	}
	if (!vbj->gravity)
	{
		len = sprintf(cbuf,"Gravity off");
		TextOut(hDC,width, y, cbuf, len);
		y+=height*1;
	}
	len = sprintf(cbuf,"Blaster range: %.2f",vbj->range);
	TextOut(hDC,width, y, cbuf,len);
	y+=height*1;
	len = sprintf(cbuf,"Blaster vel: %.2f",vbj->velocity);
	TextOut(hDC,width, y, cbuf,len);
	y+=height*1;

}
