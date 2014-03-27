#define STRCIT
#define ORBITER_MODULE
#include <orbitersdk.h>
#include <windows.h>
#include "UNVParachuteMFD.h"

#define WHITE RGB(255, 255, 255)
#define BLACK RGB(0, 0, 0)
#define RED   RGB(255, 0, 0)
#define GREEN RGB(0,255,0)
#define BLUE  RGB(0,0,255)
#define DTRQ  RGB(0,206, 209)
#define LYEL  RGB(255, 255, 224)
#define OVLG  RGB(0, 128, 0)

int mode;
UniversalParachuteManager UPM;
std::vector<MFDData*>     g_data;
bool GetDouble(void * id, char * str, void * data)
{
	double * value = (double*)data;
	if (strlen(str)==0)
		return false;
	*value=atof(str);
	return true;
}
bool SetChuteArea(void * id, char * str, void * data)
{
	if (strlen(str)==0)
		return false;
	PARACHUTE * chute = (PARACHUTE*)data;
	chute->setArea(atof(str));
	return true;
}
void Prune()
{
    if (g_data.empty())
        return;
    std::vector<MFDData*>::iterator it = g_data.begin() + 1;
    for (int i = 0; i < g_data.size(); i++)
    {
        it = g_data.begin() + i;
        if (!oapiIsVessel(g_data[i]->v->hook)) 
        {
            delete g_data[i];
            g_data.erase(it);
        }
    }
}
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	Prune();
	UPM.TimeStep();
}
DLLCLBK void opcDLLInit (HINSTANCE hDLL)
{
	static char *name = "Universal Parachute MFD";
	MFDMODESPEC spec;
	spec.name    = name;
	spec.key     = OAPI_KEY_P;
	spec.msgproc = UNVPAMFD::MsgProc;
	mode = oapiRegisterMFDMode (spec);
}

DLLCLBK void opcDLLExit (HINSTANCE hDLL)
{
	oapiUnregisterMFDMode (mode);
	for (int i = 0; i < g_data.size(); i++)
		delete g_data[i];
}
UNVPAMFD::UNVPAMFD (DWORD w, DWORD h, VESSEL *vessel)
: MFD (w, h, vessel)
{
	height=h/24;
	width=w/35;

	if (!UPM.FocusIsRegistered()){
		this->data=NULL;
		return;
	}
	bool found = false;
	for (int i = 0; i < g_data.size(); i++)
	{
		if (g_data[i]->v->hook==vessel->GetHandle())
		{
			data=g_data[i];
			found=true;
			break;
		}
	}
	if (!found)
	{
		MFDData * data = new MFDData(UPM.GetFocus());
		this->data=data;
		g_data.push_back(data);
	}
}
UNVPAMFD::~UNVPAMFD ()
{
}
int UNVPAMFD::MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case OAPI_MSG_MFD_OPENED:
		return (int)(new UNVPAMFD (LOWORD(wparam), HIWORD(wparam), (VESSEL*)lparam));
	}
	return 0;
}
bool UNVPAMFD::ConsumeKeyBuffered (DWORD key)
{
	if (!UPM.FocusIsRegistered())
		return false;
	if (data==NULL)
		return false;
	if (data->v->chutes.size() < 1)
		return false;
	VOBJ * v = data->v;
	PARACHUTE * chute = v->chutes[data->SelectedChute];
	switch(key)
	{
	case OAPI_KEY_RBRACKET:
		{
			nextChute();
			return true;
		}
	case OAPI_KEY_LBRACKET:
		{
			prevChute();
			return true;
		}
	case OAPI_KEY_J:
		{
			PARASTATE state = chute->getState();
			if (state==PACKED)
				chute->deploy();
			else if (state==DEPLOYED)
				chute->jettison();
			return true;
		}
	case OAPI_KEY_F:
		{
			oapiOpenInputBox("Enter chute area: ", SetChuteArea, 0, 20, chute);
			return true;
		}
	}
	return false;
}

bool UNVPAMFD::ConsumeButton (int bt, int event)
{
	if (!(event & PANEL_MOUSE_LBDOWN)) return false;
	switch(bt)
	{
	case 0:
		return ConsumeKeyBuffered(OAPI_KEY_RBRACKET);
	case 1:
		return ConsumeKeyBuffered(OAPI_KEY_LBRACKET);
	case 2:
		return ConsumeKeyBuffered(OAPI_KEY_J);
	case 3:
		return ConsumeKeyBuffered(OAPI_KEY_F);
	}
	return false;
}
int UNVPAMFD::ButtonMenu (const MFDBUTTONMENU **menu) const
{
	static const MFDBUTTONMENU mnu[4] = {
		{"Next chute", 0, ']'},
		{"Prev chute",  0, '['},
		{"Deploy",  0, 'J'},
		{"Set  Area", 0, 'F'}
	};
	if (menu) *menu = mnu;
	return 4;
}
char *UNVPAMFD::ButtonLabel (int bt)
{
	char * label[4] = {"NXT", "PRV", "DEP", "SET"};
	return (bt < 4 ? label[bt] : 0);
}
void UNVPAMFD::Update (HDC hDC)
{
	Title (hDC, "Universal Parachute MFD");

	double y = height*2;
	char cbuf[255];
	int len;
	if (!UPM.FocusIsRegistered())
	{
		SetTextColor(hDC, RED);
		len = sprintf(cbuf, "No parachutes available");
		TextOut(hDC, width, y, cbuf, len); 
		return;
	}
	if (data==NULL)
	{
		SetTextColor(hDC, RED);
		len = sprintf(cbuf, "Hit F8 3 times");
		TextOut(hDC, width, y, cbuf, len);
		return;
	}
	VOBJ * v = data->v;
	PARACHUTE * chute = v->chutes[data->SelectedChute];
	char mesh[255];
	chute->getMesh(mesh);
	len = sprintf(cbuf, "Selected Chute : %d of %d",data->SelectedChute+1, v->chutes.size());
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1;
	len = sprintf(cbuf, "Chute    Mesh  : %s", mesh);
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1;
	PARASTATE state = chute->getState();
	if (state==PACKED)
		len = sprintf(cbuf, "Chute    State : Packed");
	else if (state==DEPLOYED)
		len = sprintf(cbuf, "Chute    State : Deployed");
	else
		len = sprintf(cbuf, "Chute    State : Jettisoned");
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1;
	VECTOR3 ofs = chute->getOfs();
	len = sprintf(cbuf, "Chute    Offset: %.2f %.2f %.2f",ofs.x, ofs.y, ofs.z);
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1;
	double force = chute->getForce();
	len = sprintf(cbuf, "Chute    Force : %.2f N", force);
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1; 
	len = sprintf(cbuf, "Chute    Area  : %.2f m^2", chute->getArea());
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1;
}
