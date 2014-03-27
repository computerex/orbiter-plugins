
#define STRICT
#define ORBITER_MODULE

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "orbitersdk.h"
#include "nameChangeMFD.h"

int mode;

bool ChangeName (void *id, char *str, void *data)
{
	if ( strlen(str) == 0 ) return false;
	char * name = (char*)data;
	sprintf(name, "%s", str);
	return true;
}


DLLCLBK void opcDLLInit (HINSTANCE hDLL)
{
	static char *name = "Name Change MFD";
	MFDMODESPECEX spec;
	spec.name    = name;
	spec.key     = OAPI_KEY_N;
	spec.context = NULL;
	spec.msgproc = NameChangeMFD::MsgProc;
	
	mode = oapiRegisterMFDMode (spec);
}

DLLCLBK void opcDLLExit (HINSTANCE hDLL)
{
	oapiUnregisterMFDMode (mode);
}

DLLCLBK void opcPreStep (double simt, double simdt, double mjd)
{
	
}

NameChangeMFD::NameChangeMFD (DWORD w, DWORD h, VESSEL *vessel)
: MFD (w, h, vessel)
{
	
}

int NameChangeMFD::MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case OAPI_MSG_MFD_OPENEDEX: {
		MFDMODEOPENSPEC *ospec = (MFDMODEOPENSPEC*)wparam;
		return (int)(new NameChangeMFD (ospec->w, ospec->h, (VESSEL*)lparam));
		}
	}
	return 0;
}

bool NameChangeMFD::ConsumeKeyBuffered (DWORD key)
{

	switch (key) 
	{
	case OAPI_KEY_C:
		{
			oapiOpenInputBox("Enter new name: ", ChangeName, 0, 20, oapiGetFocusInterface()->GetName());
			break;
		}
	}
	return false;
}

bool NameChangeMFD::ConsumeButton (int bt, int event)
{
	if (!(event & PANEL_MOUSE_LBDOWN)) return false;
	static const DWORD btkey[1] = { OAPI_KEY_C};
	if (bt < 1) return ConsumeKeyBuffered (btkey[bt]);
	else return false;
}

char *NameChangeMFD::ButtonLabel (int bt)
{
	char *label[1] = {"CN"};
	return (bt < 1 ? label[bt] : 0);
	return 0;
}

int NameChangeMFD::ButtonMenu (const MFDBUTTONMENU **menu) const
{
	static const MFDBUTTONMENU mnu[1] = {
		{"Change Name", 0, 'C'}
	};
	if (menu) *menu = mnu;
	return 1;
}

void NameChangeMFD::Update (HDC hDC)
{
	Title (hDC, "Name Change MFD");

	char buffer[256];
	int len;

	VESSEL * v = oapiGetFocusInterface();
	len = sprintf(buffer, "focus name: %s", v->GetName());
	TextOut(hDC, this->W/35, this->H/24*6, buffer, len); 
}


