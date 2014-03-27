#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include "UMMUFA.h"
#include "HudScroll.h"
#include "EHUD.h"
#include <windows.h>
#include "common.h"
#include "LOGGER.h"
#include "parser.h"
#include "resource.h"
#include <time.h>

VECTOR3   COLOR        =  _V(0,1,0);
double    Size         =  0.6;
double    LINE_SPACE   =  0.05;
double    X_OFS_START  =  0.1;
double    Y_OFS_START  =  0.3;
HINSTANCE g_hInst;

UMMUFA    g_UFA;
BOOL      Init         = FALSE;
EHUD      HUD;
HHOOK     hhook;
BOOL      Ejection     = FALSE;
double    lEjectionTime = 0;

PARTICLESTREAMSPEC exhaust_eject = {
	0, 2.0, 10, 200, 0.05, 0.05, 8, 1.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_SQRT, 0, 1,
		PARTICLESTREAMSPEC::ATM_PLOG, 1e-5, 0.1
	};

BOOL CALLBACK MsgProc (HWND, UINT, WPARAM, LPARAM);
int rnum(int from, int to)
{
	// in case I have gone insane
	if (from>to){
		double t = from;
		from = to;
		to = t;
	}
	return rand() % from + to;
}
VECTOR3 norm(VECTOR3 vec)
{
	double l = length(vec);
	return _V(vec.x/l, vec.y/l, vec.z/l);
}
void LoadOptions(void)
{
	PARSER parser("Modules/UMMUFA.cfg");
	parser.read();
	Size  = parser.Double("Size");
	COLOR = parser.Vector3("Col");
	LINE_SPACE=parser.Double("LineSpace");
	X_OFS_START=parser.Double("X_OFS");
	Y_OFS_START=parser.Double("Y_OFS");
}

void EVA(VOBJ * v)
{
	int r = v->eva();
	char cbuf[255];
	sprintf(cbuf,"");
	switch(r)
	{
	case TRANSFER_TO_DOCKED_SHIP_OK:
		{
			sprintf(cbuf,"%s transfered to docked ship",v->crew.GetLastEvaedCrewName());
			break;
		}
	case EVA_OK:
		{
			sprintf(cbuf,"%s on EVA",v->crew.GetLastEvaedCrewName());
			break;
		}
	case ERROR_NO_ONE_ON_BOARD:
		{
			sprintf(cbuf,"No one on board!");
			break;
		}
	case ERROR_DOCKED_SHIP_HAVE_AIRLOCK_CLOSED:
		{
			sprintf(cbuf,"Docked ship airlock closed");
			break;
		}
	case ERROR_DOCKED_SHIP_IS_FULL:
		{
			sprintf(cbuf,"Docked ship full");
			break;
		}
	case ERROR_CREW_MEMBER_NOT_FOUND:
		{
			sprintf(cbuf,"Crew member not found!");
			break;
		}
	case ERROR_DOCKEDSHIP_DONOT_USE_UMMU:
		{
			sprintf(cbuf,"Docked ship does not use UMMU!");
			break;
		}
	case ERROR_MISC_ERROR_EVAFAILED:
		{
			sprintf(cbuf,"Unknown error, EVA failed!");
			break;
		}
	}
	v->nextslot();
	v->hudprint.insert(cbuf);
}
LRESULT CALLBACK WinProc(int nCode, WPARAM w, LPARAM l)
{
	if ((GetKeyState(VK_TAB) & 0x800)== 0 || (GetKeyState(w) & 0x800) == 0 || nCode < 0 || Ejection==TRUE)
		return CallNextHookEx(hhook, nCode, w, l);
	DWORD key = OAPI_KEY_TAB;
	if (!GetKey(w,key))
		return CallNextHookEx(hhook,nCode,w,l);

	VOBJ * v = g_UFA.GetFocus();
	if (v==NULL)
		return CallNextHookEx(hhook,nCode,w,l);
	char cbuf[255];
	switch(key)
	{
	case OAPI_KEY_EQUALS:
		{
			if (v->crew.GetCrewTotalNumber() < 1)
				sprintf(cbuf,"No one on board!");
			else
			sprintf(cbuf,"Selected member: %s",v->nextslot());
			v->hudprint.insert(cbuf);
			break;
		}
	case OAPI_KEY_UP:
		{
			EVA(v);
			break;
		}
	case OAPI_KEY_MINUS:
		{
			sprintf(cbuf,"Selected airlock: %d",v->nextlock()+1);
			v->hudprint.insert(cbuf);
			break;
		}
	case OAPI_KEY_RIGHT:
		{
			OBJHANDLE e = v->crew.GetObjHandleOfLastEVACrew();
			if (oapiIsVessel(e))
				oapiSetFocusObject(e);
			break;
		}
	case OAPI_KEY_LEFT:
		{
			oapiOpenDialog(g_hInst, IDD_DIALOG1 ,MsgProc,0);
			break;
		}
	case OAPI_KEY_E:
		{
			Ejection=TRUE;
			VESSEL * vessel = oapiGetVesselInterface(v->hook);
			for ( unsigned int i = 0; i < vessel->GetThrusterCount(); i++ )
				vessel->SetThrusterLevel(vessel->GetThrusterHandleByIndex(i), 0);
			v->hudprint.insert("EJECTION!");
			lEjectionTime=oapiGetSimTime()-1;
			break;
		}
	}
	return CallNextHookEx(hhook, nCode, w, l);
}
DLLCLBK void opcOpenRenderViewport(HWND hWnd, DWORD w, DWORD h, BOOL fc)
{
	hhook = SetWindowsHookEx(WH_KEYBOARD, WinProc, NULL, GetWindowThreadProcessId(hWnd, NULL));
}
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	if (Init==FALSE)
	{
		LoadOptions();
		Init=TRUE;
		srand(time(NULL));
		return;
	}
	g_UFA.TimeStep();
	HUD.TimeStep();
	VOBJ * v = g_UFA.GetFocus();
	if (v==NULL)
		return;
	char cbuf[255];
	double yofs = Y_OFS_START;
	for (int i = 0; i < v->hudprint.m.size(); i++)
	{
		sprintf(cbuf,"%s",v->hudprint.m[i].c_str());
		HUD.Add(0.0001,v->hook,HUD._D(X_OFS_START,1,yofs,1,Size,COLOR,cbuf));
		yofs+=LINE_SPACE;
	}
	HUD.TimeStep();
	if (Ejection==TRUE)
	{
		if (lEjectionTime>simt)
			return;
		lEjectionTime=simt+1;
		int crewcount = v->crew.GetCrewTotalNumber();
		if (!crewcount){
			Ejection=FALSE;
			return;
		}
		double thrust = 500e1, propmass=0.1;
		VESSEL * vessel = NULL;
		VESSEL * focus = oapiGetFocusInterface();
		VESSELSTATUS vs; 
		OBJHANDLE hVes=NULL;
		if (VESSEL(v->hook).GetAtmPressure() < 2.5){
			thrust=50e1;
			propmass=0.01;
		}
		v->crew.SetEjectPosRotRelSpeed(_V(0, 0, 0),_V(0,1,0), _V(0,0,0));
		v->crew.EjectCrewMember(v->crew.GetCrewNameBySlotNumber(0));
		hVes = v->crew.GetObjHandleOfLastEVACrew();
		if (!oapiIsVessel(hVes))
			return;
		vessel = oapiGetVesselInterface(hVes);
		sprintf(cbuf,"%s-pack",vessel->GetName());
		PROPELLANT_HANDLE hp = vessel->CreatePropellantResource(propmass);
		THRUSTER_HANDLE th = vessel->CreateThruster(_V(0,0,0), _V(0,1,0), thrust, hp);
		vessel->AddExhaustStream(th, &exhaust_eject);
		vessel->SetThrusterLevel(th, 1.0);
		vessel->GetStatus(vs);
		VECTOR3 rofs;
		vessel->GlobalRot(_V(0, 1, 0), rofs);
		vs.rvel.x+=rofs.x;
		vs.rvel.y+=rofs.y;
		vs.rvel.z+=rofs.z;
		oapiCreateVessel(cbuf,"Ummuturbopack",vs);
	}
	
}

DLLCLBK void opcSaveState (FILEHANDLE scn)
{
	g_UFA.save(scn);
}
DLLCLBK void opcLoadState (FILEHANDLE scn)
{
	g_UFA.init(scn);
}
DLLCLBK void opcDLLInit (HINSTANCE hDLL)
{
	g_hInst = hDLL; 
}
BOOL CALLBACK MsgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	VOBJ * v = g_UFA.GetFocus();
	if (v==NULL)
		return FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			SendDlgItemMessage(hDlg,IDC_COMBO_AGE,CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(hDlg,IDC_COMBO_WEIGHT,CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(hDlg,IDC_COMBO_ID,CB_RESETCONTENT, 0, 0);
			char cbuf[255];
			for (int i = 0; i < 300; i++)
			{
				sprintf(cbuf,"%d",i+1);
				SendDlgItemMessage(hDlg, IDC_COMBO_AGE, CB_ADDSTRING, 0, (LPARAM)cbuf);
				SendDlgItemMessage(hDlg, IDC_COMBO_WEIGHT, CB_ADDSTRING, 0, (LPARAM)cbuf);
			}
			for (int i = 0; i < iNbrOfIdConfigLoaded; i++)
			{
				strcpy(cbuf,UmmuMiscID[i]);
				SendDlgItemMessage(hDlg, IDC_COMBO_ID, CB_ADDSTRING, 0, (LPARAM)cbuf);
			}
			return TRUE;
		}

	case WM_DESTROY:
		{
			return TRUE;
		}

	case WM_COMMAND:
		{
			switch (LOWORD (wParam)) 
			{
			case IDCANCEL:
				{
					oapiCloseDialog(hDlg);
					return TRUE;
				}
			case IDC_BUTTON_ADD:
				{
					// hack, but that's the only way
					v->crew.SetMaxSeatAvailableInShip(v->crew.GetCrewTotalNumber()+1);
					char name[255];
					char cbuf[255];
					int len = GetDlgItemText(hDlg, IDC_EDIT_NAME, name,255);
					if (len < 1)
						return FALSE;
					len = GetDlgItemText(hDlg, IDC_COMBO_AGE, cbuf, 255);
					if (len < 1)
						return FALSE;
					int age = atoi(cbuf);
					len = GetDlgItemText(hDlg, IDC_COMBO_WEIGHT, cbuf, 255);
					if (len < 1)
						return FALSE;
					int weight = atoi(cbuf);
					len = GetDlgItemText(hDlg, IDC_COMBO_ID, cbuf, 255);
					if (len > 4)
						return FALSE;
					v->crew.AddCrewMember(name, age, 70, weight, cbuf);
					char mess[255];
					sprintf(mess,"%s %s aged %d added",cbuf,name, age);
					v->hudprint.insert(mess);
					return TRUE;
				}
			case IDC_BUTTON_SAVE:
				{
					char scn[255];
					char cbuf[255];
					int len = GetDlgItemText(hDlg, IDC_EDIT_FILENAME, cbuf, 255);
					if (len < 1)
						return FALSE;
					sprintf(scn,"%s",cbuf);
					sprintf(cbuf,"UMMUFA Save at %.2f seconds from %s",oapiGetSimTime(),VESSEL(v->hook).GetName());
					oapiSaveScenario(scn, cbuf);
					sprintf(cbuf,"%s scenario saved",scn);
					v->hudprint.insert(cbuf);
					return TRUE;
				}
			}
			return TRUE;
		}
	}
	return oapiDefDialogProc (hDlg, uMsg, wParam, lParam);
}

