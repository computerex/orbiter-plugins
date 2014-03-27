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


VECTOR3   COLOR        =  _V(0,1,0);
double    Size         =  0.6;
double    LINE_SPACE   =  0.05;
double    X_OFS_START  =  0.1;
double    Y_OFS_START  =  0.3;
bool      SaveState    =  false;
HINSTANCE g_hInst;

UMMUFA    g_UFA;
BOOL      Init         = FALSE;
BOOL      Loaded       = FALSE;
EHUD      HUD;
HHOOK     hhook;

PARTICLESTREAMSPEC exhaust_eject = {
		0, 2.0, 10, 200, 0.05, 0.05, 8, 1.0, PARTICLESTREAMSPEC::EMISSIVE,
		PARTICLESTREAMSPEC::LVL_SQRT, 0, 1,
		PARTICLESTREAMSPEC::ATM_PLOG, 1e-5, 0.1
	};

BOOL CALLBACK MsgProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK CFGCreate (HWND, UINT, WPARAM, LPARAM);
void LoadOptions(void)
{
	PARSER parser("Modules/UMMUFA.cfg");
	parser.read();
	Size  = parser.Double("Size");
	COLOR = parser.Vector3("Col");
	LINE_SPACE=parser.Double("LineSpace");
	X_OFS_START=parser.Double("X_OFS");
	Y_OFS_START=parser.Double("Y_OFS");
	SaveState=parser.Bool("SaveState");
	g_UFA.DisableWhenEmpty=parser.Int("DisableWhenEmpty");
}
char * RidSpaces(char * line)
{
     int len = strlen(line);
     char *buffer = new char[len];
     
     int i = 0, j = 0;
     char input = line[0];
     while(input != '\0')
     {
          if (input != ' ')
          {
               buffer[j]=line[i];
               j++;
          }
          i++;
          input=line[i];
     }
     buffer[j]='\0';
     strcpy(line,buffer);
     delete [] buffer;
     return line; 
}
void GetFileName(char * buffer)
{
	char cbuf[255];
	for (int i = 0; i < 500; i++)
	{
		sprintf(cbuf,"Scenarios/UMMUFA/CurrentState%d.cfg",i+1);
		if (fopen(cbuf,"r")==0)
		{
			strcpy(buffer,cbuf);
			return;
		} 
	}
	strcpy(buffer,"Scenarios/UMMUFA/CurrentState.cfg");
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
	if ((GetKeyState(VK_TAB) & 0x800)== 0 || (GetKeyState(w) & 0x800) == 0 || nCode < 0)
		return CallNextHookEx(hhook, nCode, w, l);
	DWORD key = OAPI_KEY_TAB;
	if (!GetKey(w,key))
		return CallNextHookEx(hhook,nCode,w,l);

	switch(key)
	{
	case OAPI_KEY_D:
		{
			oapiOpenDialog(g_hInst, IDD_DIALOG2,CFGCreate,0);
			return TRUE;
		}
	}

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
			double eVel = 10;
			if (VESSEL(v->hook).GetAtmPressure() > 2.5)
				eVel=50;
			double x =0;
			v->crew.SetEjectPosRotRelSpeed(_V(0,0,x),_V(0,1,0),_V(0,eVel,0));
			OBJHANDLE hVes;
			VESSEL * vessel = NULL;
			PROPELLANT_HANDLE hpr=NULL;
			THRUSTER_HANDLE th = NULL;
			char cbuf[255];
			while(v->crew.GetCrewTotalNumber())
			{
				x+=2;
				v->crew.SetEjectPosRotRelSpeed(_V(0,0,x),_V(0,1,0),_V(0,eVel,0));
				v->crew.EjectCrewMember(v->crew.GetCrewNameBySlotNumber(0));
				hVes = v->crew.GetObjHandleOfLastEVACrew();
				if (!oapiIsVessel(hVes))
					continue;
				vessel = oapiGetVesselInterface(hVes);
				hpr = vessel->CreatePropellantResource(1);
				th = vessel->CreateThruster(_V(0,0,0),_V(0,1,0),0.1e1,hpr,1);
				vessel->AddExhaustStream(th,&exhaust_eject);
				vessel->SetThrusterLevel(th,1);
				sprintf(cbuf,"%s-pack",vessel->GetName());
				VESSELSTATUS vs; 
				vessel->GetStatus(vs);
				oapiCreateVessel(cbuf,"Ummuturbopack",vs);
			}
			v->hudprint.insert("EJECTION!");
			return TRUE;
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
	if (Init==FALSE && oapiGetSimTime() > 1)
	{
		LoadOptions();
	    Init=TRUE;
		g_UFA.state=ST_GENERIC;
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
}

DLLCLBK void opcSaveState (FILEHANDLE scn)
{
	char filename[255];
	sprintf(filename,"Scenarios/UMMUFA/CurrentState.cfg");
	if (g_UFA.SpecialSave==TRUE)
	{
		oapiWriteScenario_string (scn, "CFG", g_UFA.SpecialSaveBuffer);
		g_UFA.save();
		return;
	}
	if (SaveState)
	{
		oapiWriteScenario_string (scn, "CFG", filename);
		sprintf(g_UFA.cfg,"%s", filename);
		g_UFA.save();
	}
}
DLLCLBK void opcLoadState (FILEHANDLE scn)
{
	char *line;
	while (oapiReadScenario_nextline (scn, line)) {
		if (!strnicmp (line, "CFG", 3)) {
			sscanf (line+3, "%s", &g_UFA.cfg);
		}
	}
	if (g_UFA.init())
	{
		Loaded=TRUE;
		g_UFA.state=ST_ALL;
	}
	LoadOptions();
	Init=TRUE;
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
			char cbuf[255];
			for (int i = 0; i < 300; i++)
			{
				sprintf(cbuf,"%d",i+1);
				SendDlgItemMessage(hDlg, IDC_COMBO_AGE, CB_ADDSTRING, 0, (LPARAM)cbuf);
				SendDlgItemMessage(hDlg, IDC_COMBO_WEIGHT, CB_ADDSTRING, 0, (LPARAM)cbuf);
			}
			for (int i = 0; i < NUMBER_OF_MMU_TYPE; i++)
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
					if (v->seat < v->crew.GetCrewTotalNumber()+1)
					{
						v->hudprint.insert("No more room left!");
						return TRUE;
					}
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
					char cfg[255];
					char cbuf[255];
					int len = GetDlgItemText(hDlg, IDC_EDIT_FILENAME, cbuf, 255);
					if (len < 1)
						return FALSE;
					sprintf(scn,"%s",cbuf);
					sprintf(cfg,"Scenarios/UMMUFA/%s.cfg",RidSpaces(cbuf));
					g_UFA.SpecialSave=TRUE;
					sprintf(g_UFA.SpecialSaveBuffer,"%s",cfg);
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
bool AreThereBlankFields(HWND hDlg, int from, int to)
{
	char cbuf[255];
	int len;
	for (int i = from; i < to+1; i++)
	{
		len = GetDlgItemText(hDlg, i, cbuf, 255);
		if (len < 1)
			return true;
	}
	return false;
}
char * GetField(HWND hDlg, int control, char * buffer, int size)
{
	GetDlgItemText(hDlg, control, buffer, size);
	return buffer;
}
bool isBlank(HWND hDlg, int control, char * buffer, int size)
{
	int len = GetDlgItemText(hDlg, control, buffer, size);
	if (len < 1)
		return true;
	return false;
}
bool Assign(VOBJ * v)
{
	char cfgName[255];
	char nam[255];

	VESSEL * vessel = oapiGetVesselInterface(v->hook);

	if (g_UFA.isSpacecraft(v->hook))
	{
		sprintf(nam,"%s",vessel->GetName());
		sprintf(cfgName,"config/UMMUFA/%s.cfg",nam);
	}
	else{
		sprintf(nam,"%s",vessel->GetClassName());
		sprintf(cfgName,"config/UMMUFA/%s.cfg",nam);
	}
	
	char cbuf[255];
	char sep[255];
	sprintf(sep, ";---------------------------------------------");
	
	LOGGER log(cfgName,"write");
	
	log.write(sep);
	sprintf(log.line(),";%s UMMUFA configuration file",nam);
	log.write();
	log.write(";Generated by UMMUFA");
	log.write(sep);
	sprintf(log.line(),"AirlockCount = %d", v->alv.size());
	log.write();
	sprintf(log.line(),"seats = %d",v->seat);
	log.write();
	sprintf(log.line(),"KillOnCrash = %d",v->koc);
	log.write();
	sprintf(log.line(),"KillHVel = %.2f",v->khv);
	log.write();
	log.write(sep);
	sprintf(log.line(),";%s Airlock definitions",nam);
	log.write();
	log.write(sep);
	AirLock al;
	for (int i = 0; i < v->alv.size(); i++)
	{
		al = v->alv[i];
		sprintf(log.line(),";Airlock %d",i+1);
		log.write();
		sprintf(log.line(),"Airlock_%d_PosP = %.2f %.2f %.2f",i+1,al.posp.x, al.posp.y, al.posp.z);
		log.write();
		sprintf(log.line(),"Airlock_%d_PosN = %.2f %.2f %.2f",i+1,al.posn.x, al.posn.y, al.posn.z);
		log.write();
		sprintf(log.line(),"Airlock_%d_rot = %.2f %.2f %.2f",i+1,al.rot.x, al.rot.y, al.rot.z);
		log.write();
		sprintf(log.line(),"Airlock_%d_Pos = %.2f %.2f %.2f",i+1,al.pose.x, al.pose.y, al.pose.z);
		log.write();
	}
	sprintf(cbuf,"%s created. Press TAB + LEFT to add crew",cfgName);
	v->hudprint.insert(cbuf);
	return true;
}
bool isNull(VOBJ * v)
{
	if (v==NULL)
		return true;
	return false;
}
BOOL CALLBACK CFGCreate(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	VOBJ * v = g_UFA.GetFocus();
	static std::vector<AirLock> alv;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			alv.clear();
			return TRUE;
		}
	case WM_DESTROY:
		{
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDCANCEL:
				{
					oapiCloseDialog(hDlg);
					return TRUE;
				}
			case IDC_BUTTON_CLKME:
				{
					MessageBox(hDlg, "Enter 0 to turn off the collision kill feature","Just so you know...", MB_OK);
					return TRUE;
				}
			case IDC_BUTTON_ADDAIRLOCK:
				{
					if (!isNull(v))
						return FALSE;
					char cbuf[255];
					if (isBlank(hDlg, IDC_EDIT_ALCOUNT, cbuf, 255))
						return FALSE;
					int alc = atoi(cbuf);
					if (alc < alv.size()+1)
						return FALSE;
					VECTOR3 posp;
					if (isBlank(hDlg, IDC_EDIT_POSPX, cbuf, 255))
						return FALSE;
					posp.x=atof(cbuf);
					if (isBlank(hDlg, IDC_EDIT_POSPY, cbuf, 255))
						return FALSE;
					posp.y=atof(cbuf);
					if (isBlank(hDlg, IDC_EDIT_POSPZ, cbuf, 255))
						return FALSE;
					posp.z=atof(cbuf);
					/////
					VECTOR3 posn;
					if (isBlank(hDlg, IDC_EDIT_POSNX, cbuf, 255))
						return FALSE;
					posn.x=atof(cbuf);
					if (isBlank(hDlg, IDC_EDIT_POSNY, cbuf, 255))
						return FALSE;
					posn.y=atof(cbuf);
					if (isBlank(hDlg, IDC_EDIT_POSNZ, cbuf, 255))
						return FALSE;
					posn.z=atof(cbuf);
					/////
					VECTOR3 rot;
					if (isBlank(hDlg, IDC_EDIT_ROTX, cbuf, 255))
						return FALSE;
					rot.x=atof(cbuf);
					if (isBlank(hDlg, IDC_EDIT_ROTY, cbuf, 255))
						return FALSE;
					rot.y=atof(cbuf);
					if (isBlank(hDlg, IDC_EDIT_ROTZ, cbuf, 255))
						return FALSE;
					rot.z=atof(cbuf);
					////
					VECTOR3 pos;
					if (isBlank(hDlg, IDC_EDIT_POSX, cbuf, 255))
						return FALSE;
					pos.x=atof(cbuf);
					if (isBlank(hDlg, IDC_EDIT_POSY, cbuf, 255))
						return FALSE;
					pos.y=atof(cbuf);
					if (isBlank(hDlg, IDC_EDIT_POSZ, cbuf, 255))
						return FALSE;
					pos.z=atof(cbuf);
					alv.push_back(AirLock(posp,posn,rot,pos));
					sprintf(cbuf,"Airlock %d",alv.size());
					SendDlgItemMessage(hDlg, IDC_LIST_AIRLOCK,LB_ADDSTRING,0,(LPARAM)cbuf); 
					return TRUE;
				}
			case IDC_BUTTON_CLEAR:
				{
					SendDlgItemMessage(hDlg, IDC_LIST_AIRLOCK, LB_RESETCONTENT, 0, 0);
					alv.clear();
					return TRUE;
				}
			case IDC_BUTTON_CREATECFG:
				{
					if (!isNull(v))
					{
						MessageBox(hDlg, "Vessel is already registered!", "Just so you know...",MB_OK);
						return FALSE;
					}
					if (alv.size() < 1)
					{
						MessageBox(hDlg, "You must have at least one airlock defined!","Just so you know...",MB_OK);
						return FALSE;
					}
					char cbuf[255];
					if (isBlank(hDlg, IDC_EDIT_SEATCOUNT, cbuf, 255))
						return FALSE;
					int seat = atoi(cbuf);
					if (isBlank(hDlg, IDC_EDIT_KILLVEL, cbuf, 255))
						return FALSE;
					double khv = atof(cbuf);
					BOOL koc = TRUE;
					if (khv == 0)
						koc = FALSE;
					CrewMemberDef cmd;
					cmd.clear();
					VOBJ * vbj = new VOBJ(alv, cmd, oapiGetFocusInterface()->GetHandle(), seat);
					vbj->koc=koc;
					vbj->khv=khv;
					g_UFA.atlist.push_back(vbj);
					Assign(g_UFA.GetFocus());
					return TRUE;
				}
			}
			return TRUE;
		}
		
	}
	return oapiDefDialogProc(hDlg, uMsg,wParam, lParam);
}
			
