#define STRCIT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include <windows.h>
#include "FTMFD.h"
#include <vector>
#include <MFDSoundSDK35.h>

#define RED RGB(255, 0, 0)
#define GREEN RGB(0, 255, 0)
#define BLUE RGB(0, 0, 255)
#define WHITE RGB(255, 255, 255)


std::vector<MFDData*> g_data;
BOOL Init=FALSE;
int  mode;
int  SoundID=-1;

void Prune()
{
	if (g_data.empty())
		return;
	std::vector<MFDData*>::iterator it = g_data.begin() + 1;
    for (int i = 0; i < g_data.size(); i++)
	{
		it = g_data.begin() + i;
		if (!g_data[i])
		{
			g_data.erase(it);
		}else
		{
			if (!oapiIsVessel(g_data[i]->hook->getHook()))
			{
				delete g_data[i];
				g_data.erase(it);
			}
		}
	}
}
bool GetDouble(void * id, char * str, void * data)
{
	double dbl = atof(str);
	if (dbl<0)
		dbl=-dbl;
	((FTMFD*)data)->data->hook->setTransferRate(dbl);
	return true;
}
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	if (Init==FALSE)
	{
		Init=TRUE;
		SoundID=ConnectMFDToOrbiterSound("FuelTransferMFD_computerex");
		LoadMFDWave(SoundID, 0, "Sound/FuelTransferMFD_computerex/fueling.wav");
		LoadMFDWave(SoundID, 1, "Sound/FuelTransferMFD_computerex/fueling_done.wav");
	}
	Prune();
	for (int i = 0; i < g_data.size(); i++)
	{
		g_data[i]->hook->TimeStep(SoundID);
	}
}
DLLCLBK void opcDLLInit (HINSTANCE hDLL)
{
	static char *name = "Fuel Transfer MFD";
	MFDMODESPEC spec;
	spec.name    = name;
	spec.key     = OAPI_KEY_F;
	spec.msgproc = FTMFD::MsgProc;
	mode = oapiRegisterMFDMode (spec);
}

DLLCLBK void opcDLLExit (HINSTANCE hDLL)
{
	oapiUnregisterMFDMode (mode);
	for (int i = 0; i < g_data.size(); i++)
		delete g_data[i];
}
FTMFD::FTMFD (DWORD w, DWORD h, VESSEL *vessel)
: MFD (w, h, vessel)
{
	bool found = false;
	for (int i = 0; i < g_data.size(); i++)
	{
		if (vessel->GetHandle()==g_data[i]->hook->getHook())
		{
			found=true;
			this->data=g_data[i];
			break;
		}
	}
	if (!found)
	{
		MFDData * dt = new MFDData(vessel->GetHandle());
		g_data.push_back(dt);
		data = dt;
	}
	height=h/24;
	width=w/35;
}
FTMFD::~FTMFD ()
{
}
int FTMFD::MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case OAPI_MSG_MFD_OPENED:
		return (int)(new FTMFD (LOWORD(wparam), HIWORD(wparam), (VESSEL*)lparam));
	}
	return 0;
}
bool FTMFD::ConsumeKeyBuffered (DWORD key)
{
	switch(key)
	{
	case OAPI_KEY_M:
		{
			data->hook->toggleMode();
			return true;
		}
	case OAPI_KEY_T:
		{
			data->hook->toggleStatus(SoundID);
			return true;
		}
	case OAPI_KEY_LBRACKET:
		{
			data->hook->nextSelectSource();
			return true;
		}
	case OAPI_KEY_D:
		{
			oapiOpenInputBox("Enter transfer rate: ", GetDouble, 0, 20, this);
			return true;
		}
	case OAPI_KEY_H:
	    {
			if (data->ShowHelp==TRUE)
				data->ShowHelp=FALSE;
			else
				data->ShowHelp=TRUE;
			return true;
		}
	}
	if (data->hook->getMode()==MAGIC)
		return false;
	switch(key)
	{
	case OAPI_KEY_N:
		{
			data->hook->nextTarget();
			return true;
		}
	case OAPI_KEY_B:
		{
			data->hook->prevTarget();
			return true;
		}
	case OAPI_KEY_RBRACKET:
		{
			data->hook->nextTargetSource();
			return true;
		}
	}
	return false;
}

bool FTMFD::ConsumeButton (int bt, int event)
{
	if (!(event & PANEL_MOUSE_LBDOWN)) return false;

	switch(bt)
	{
	case 0:
		return ConsumeKeyBuffered(OAPI_KEY_N);
	case 1:
		return ConsumeKeyBuffered(OAPI_KEY_B);
	case 2:
		return ConsumeKeyBuffered(OAPI_KEY_T);
	case 3:
		return ConsumeKeyBuffered(OAPI_KEY_LBRACKET);
	case 4:
		return ConsumeKeyBuffered(OAPI_KEY_RBRACKET);
	case 5:
		return ConsumeKeyBuffered(OAPI_KEY_D);
	case 6:
		return ConsumeKeyBuffered(OAPI_KEY_M);
	case 7:
		return ConsumeKeyBuffered(OAPI_KEY_H);
	}
	return false;
}

char *FTMFD::ButtonLabel (int bt)
{
	char *label[8] = {"S+", "S-", "STA", "VT+", "ST+","SPD", "MOD", "HLP"};
	return (bt < 8 ? label[bt] : 0);
}

int FTMFD::ButtonMenu (const MFDBUTTONMENU **menu) const
{
	static const MFDBUTTONMENU mnu[8] = {
		{"Next source vessel", 0, 'N'},
		{"Previous source vessel", 0, 'B'},
		{"start/stop transfer", 0, 'T'},
		{"next destination tank",0, '['},
		{"next source tank", 0, ']'},
		{"set transfer rate", 0, 'D'},
		{"toggle modes", 0, 'M'},
		{"Show help page", 0, 'H'}
	};
	if (menu) *menu = mnu;
	return 8;
}
void FTMFD::Update (HDC hDC)
{
	Title (hDC, "Fuel Transfer MFD");

	double y = height*2;
	char cbuf[255];
	int len;
	VOBJ * v = data->hook;
	if (data->ShowHelp==TRUE)
	{
		if (v->getMode()==VTOV)
		{
			len = sprintf(cbuf, "Mode: Vessel To Vessel");
			TextOut(hDC, width, y, cbuf, len);
			y+=height*1;
			SetTextColor(hDC, RED);
			len = sprintf(cbuf, "Use:  Transfer fuel between vessels");
			TextOut(hDC, width, y, cbuf, len);
			y+=height*1;
			SetTextColor(hDC, GREEN);
			len = sprintf(cbuf, "How:");
			TextOut(hDC, width, y, cbuf, len);
			y+=height*1;
			len = sprintf(cbuf, "Choose the source vessel");
			TextOut(hDC, width, y, cbuf, len);
			y+=height*1;
			len = sprintf(cbuf, "Choose a source tank, and hit STA");
			TextOut(hDC, width, y, cbuf, len);
		}else
		if (v->getMode()==DUMPING)
		{
			len = sprintf(cbuf, "Mode: Dumping");
			TextOut(hDC, width, y, cbuf, len);
			y+=height*1;
			SetTextColor(hDC, RED);
			len = sprintf(cbuf, "Use:  Dump fuel overboard");
			TextOut(hDC, width, y, cbuf, len);
			y+=height*1;
			SetTextColor(hDC, GREEN);
			len = sprintf(cbuf, "How:  Choose a tank, and hit STA");
			TextOut(hDC, width, y, cbuf, len);
		}else
		if (v->getMode()==MAGIC)
		{
			len = sprintf(cbuf, "Mode: Magic");
			TextOut(hDC, width, y, cbuf, len);
			y+=height*1;
			SetTextColor(hDC, RED);
			len = sprintf(cbuf, "Use:  Fuel your tanks magically");
			TextOut(hDC, width, y, cbuf, len);
			y+=height*1;
			SetTextColor(hDC, GREEN);
			len = sprintf(cbuf, "How:  Choose a tank, and hit STA");
			TextOut(hDC, width, y, cbuf, len);
		}
		return;
	}
	if (v->vhook()->GetPropellantCount() < 1){
		len = sprintf(cbuf, "%s", v->message());
		TextOut(hDC, width, y, cbuf, len);
		return;
	}
	PROPELLANT_HANDLE hpr = oapiGetPropellantHandle(v->getHook(), v->getSelectedSource());
	if (!hpr)
	{
		len = sprintf(cbuf, "Unknown error, restart Orbiter");
		TextOut(hDC, width, y, cbuf, len);
		return;
	}
	double mass = oapiGetPropellantMass(hpr);
	double maxmass = oapiGetPropellantMaxMass(hpr);

	if (v->getMode() == VTOV)
	{
		if (v->vtarget()->GetPropellantCount() < 1)
		{
			len = sprintf(cbuf, "%s has no fuel sources", v->vtarget()->GetName());
			TextOut(hDC, width, y, cbuf, len);
			return;
		}
		PROPELLANT_HANDLE htarp = oapiGetPropellantHandle(v->getTarget(), v->getTargetSource());
		if (!htarp)
		{
			len = sprintf(cbuf, "Unknown error, restart Orbiter");
		    TextOut(hDC, width, y, cbuf, len);
			return;
		}
		double tmass = oapiGetPropellantMass(htarp);
		double tmaxmass = oapiGetPropellantMaxMass(htarp);
		len = sprintf(cbuf, "Mode: Vessel to Vessel transfer");
		TextOut(hDC, width, y, cbuf, len);
		y+=height*2;
		SetTextColor(hDC, GREEN);
		len = sprintf(cbuf, "Source: %s", v->vtarget()->GetName());
		TextOut(hDC, width, y, cbuf, len);
		y+=height*1;
		len = sprintf(cbuf, "Source tank: [%d] [%.2f%%] ", v->getTargetSource()+1, tmass/tmaxmass*100);
		TextOut(hDC, width, y, cbuf, len);
		y+=height*1;
		len = sprintf(cbuf, "Source tank mass: %.2f kg", tmass);
		TextOut(hDC, width, y, cbuf, len);
		y+=height*3;
		SetTextColor(hDC, BLUE);
		len = sprintf(cbuf, "Vessel tank: [%d] [%.2f%%]", v->getSelectedSource()+1, mass/maxmass*100);
		TextOut(hDC, width, y, cbuf, len);
	    y+=height*1;
		len = sprintf(cbuf, "Vessel tank mass: %.2f kg", mass);
		TextOut(hDC, width, y, cbuf, len);
		y+=height*2;
	}else if (v->getMode() == MAGIC)
	{
		len = sprintf(cbuf, "Mode: Magic");
		TextOut(hDC, width, y, cbuf, len);
		y+=height*2;
		SetTextColor(hDC, GREEN);
		len = sprintf(cbuf, "Tank: [%d] [%.2f%%]", v->getSelectedSource()+1,mass/maxmass*100);
		TextOut(hDC, width, y, cbuf, len);
		y+=height*1;
		len = sprintf(cbuf, "Tank mass: %.2f kg", mass);
		TextOut(hDC, width, y, cbuf, len);
		y+=height*2;
	}else if (v->getMode() == DUMPING)
	{
		len = sprintf(cbuf, "Mode: Dumping");
		TextOut(hDC, width, y, cbuf, len);
		y+=height*2;
		SetTextColor(hDC, GREEN);
		len = sprintf(cbuf, "Tank: [%d] [%.2f%%] ", v->getSelectedSource()+1, mass/maxmass*100);
		TextOut(hDC, width, y, cbuf, len);
		y+=height*1;
		len = sprintf(cbuf, "Mass: %.2f kg", mass);
		TextOut(hDC, width, y, cbuf, len);
		y+=height*2;
	}
	SetTextColor(hDC, WHITE);
	len = sprintf(cbuf, "Transfer Rate %.2f kg/s", v->getTransferRate());
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1;
	len = sprintf(cbuf, "%s", v->message());
	TextOut(hDC, width, y, cbuf, len);
	y+=height*3;
	SetTextColor(hDC, GREEN);
	len = sprintf(cbuf, "Destination Tank %.2f kg", mass);
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1.5;
	double x = mass/maxmass*(width*35);
	if (x<width)
		x=width;
	Rectangle(hDC, width, y, width*35, y+height*2);
	HBRUSH hBrush     = CreateSolidBrush(BLUE);
	HBRUSH hOldBrush  = (HBRUSH)SelectObject(hDC, hBrush);
	Rectangle(hDC, width, y, x, y+height*2);
	SelectObject(hDC, hOldBrush);
	len = sprintf(cbuf, "100%%");
	TextOut(hDC, width*35-width*5, y+(height/2), cbuf, len);
	len = sprintf(cbuf, "50%%");
	TextOut(hDC, width*35/2, y+(height/2), cbuf, len);
	len = sprintf(cbuf, "0%%");
	TextOut(hDC, width, y+(height/2), cbuf, len);
	y+=height*2;
	if (v->getMode()==VTOV)
	{
		PROPELLANT_HANDLE htarp = oapiGetPropellantHandle(v->getTarget(), v->getTargetSource());
		if (htarp)
		{
			double mass = oapiGetPropellantMass(htarp);
			double maxmass = oapiGetPropellantMaxMass(htarp);
			len = sprintf(cbuf, "Source Tank %.2f kg", mass);
			TextOut(hDC, width, y, cbuf, len);
			y+=height*1.5;
			Rectangle(hDC, width, y, width*35, y+height*2);
			hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
			double x = mass/maxmass*(width*35);
			if (x<width)
				x=width;
			Rectangle(hDC, width, y, x, y+height*2);
			SelectObject(hDC, hOldBrush);
			len = sprintf(cbuf, "100%%");
			TextOut(hDC, width*35-width*5, y+(height/2), cbuf, len);
			len = sprintf(cbuf, "50%%");
			TextOut(hDC, width*35/2, y+(height/2), cbuf, len);
			len = sprintf(cbuf, "0%%");
			TextOut(hDC, width, y+(height/2), cbuf, len);
		}
	}
	DeleteObject(hBrush);
}
