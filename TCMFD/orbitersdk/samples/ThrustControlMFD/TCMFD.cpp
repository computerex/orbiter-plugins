#define STRCIT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include <windows.h>
#include "TCMFD.h"
#include "MfdSoundSDK35.h"
#include "REMT.h"

#define WHITE RGB(255, 255, 255)
#define BLACK RGB(0, 0, 0)
#define RED   RGB(255, 0, 0)
#define GREEN RGB(0,255,0)
#define BLUE  RGB(0,0,255)
#define DTRQ  RGB(0,206, 209)
#define LYEL  RGB(255, 255, 224)
#define OVLG  RGB(0, 128, 0)

std::vector<MFDData*> g_data;
int mode, id = 0;
BOOL Init = FALSE;

void Prune()
{
	if (g_data.empty())
		return;
	std::vector<MFDData*>::iterator it = g_data.begin() + 1;
    for (int i = 0; i < g_data.size(); i++)
	{
		it = g_data.begin() + i;
		if (!oapiIsVessel(g_data[i]->v->getHookHandle()))
		{
			delete g_data[i];
			g_data.erase(it);
		}
	}
}
bool GetDouble(void * id, char * str, void * data)
{
	double * value = (double*)data;
	if (strlen(str)==0)
		return false;
	*value=atof(str);
	return true;
}
bool GetBurnDuration(void * id, char * str, void * data)
{
	MFDData * Mdata = (MFDData*)data;
	if (strlen(str)==0)
		return false;
	double val = atof(str);
	if (val<0)
		val*=-1;
	Mdata->remoteThrust->setBurnDuration(val);
	return true;
}
bool GetCountdown(void * id, char * str, void * data)
{
	MFDData * Mdata = (MFDData*)data;
	if (strlen(str)==0)
		return false;
	double val = atof(str);
	if (val<0)
		val*=-1;
	Mdata->remoteThrust->setCountdown(val);
	return true;
}
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	if (Init==FALSE)
	{
		id = ConnectMFDToOrbiterSound("computerex_Thrust_controlMFD");
		LoadMFDWave(id, 0, "Sound/Vessel/countdown.wav");
		Init=TRUE;
	}
	Prune();
	MFDData * data;
	double    err=0;
	double    p  =0;
	for (int i = 0; i < g_data.size(); i++)
	{
		g_data[i]->v->TimeStep();
		g_data[i]->remoteThrust->TimeStep();
		data = g_data[i];
		if (data->v->ap[data->SelectedAP].active)
		{
			if (oapiGetSimTime()-data->lSampleTime > 1)
			{
				data->lSampleTime=oapiGetSimTime();
				for (int i = 0; i < 17; i++)
				{
					if (data->points[i].x-data->width*3<data->width)
					{
						data->points[i].x=data->width*35;
						if (data->SelectedAP==AP_HOV)
						{
							VECTOR3 hVel;
							data->v->getHook()->GetHorizonAirspeedVector(hVel);
							err=fabs(data->v->ap[data->SelectedAP].sp-hVel.y);
						}else if (data->SelectedAP==AP_VEL)
						{
							err = fabs(data->v->ap[data->SelectedAP].sp-data->v->getHook()->GetAirspeed());
						}
						p=err/500;
						p=p*(data->height*24);
						data->points[i].y=data->height*24-p;
					}else{
						if (i==0)
							data->points[i].x-=data->width*3;
						else
							data->points[i].x=data->points[i-1].x-data->width*3;
					}
				}
			}
		}
	}
}
DLLCLBK void opcDLLInit (HINSTANCE hDLL)
{
	static char *name = "Thruster Control MFD";
	MFDMODESPEC spec;
	spec.name    = name;
	spec.key     = OAPI_KEY_T;
	spec.msgproc = TCMFD::MsgProc;
	mode = oapiRegisterMFDMode (spec);
}

DLLCLBK void opcDLLExit (HINSTANCE hDLL)
{
	oapiUnregisterMFDMode (mode);
	for (int i = 0; i < g_data.size(); i++)
		delete g_data[i];
}
TCMFD::TCMFD (DWORD w, DWORD h, VESSEL *vessel)
: MFD (w, h, vessel)
{
	bool found = false;
	for (int i = 0; i < g_data.size(); i++)
	{
		if (vessel->GetHandle()==g_data[i]->v->getHookHandle())
		{
			found=true;
			this->data=g_data[i];
			break;
		}
	}
	if (!found)
	{
		MFDData * data = new MFDData(new VOBJ(vessel->GetHandle()), w, h, id);
		g_data.push_back(data);
		this->data=data;
	}
	height=h/24;
	width=w/35;
}
TCMFD::~TCMFD ()
{
}
int TCMFD::MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case OAPI_MSG_MFD_OPENED:
		return (int)(new TCMFD (LOWORD(wparam), HIWORD(wparam), (VESSEL*)lparam));
	}
	return 0;
}
bool TCMFD::ConsumeKeyBuffered (DWORD key)
{
	VESSEL * vessel = data->v->getHook();
	switch(key)
	{
	case OAPI_KEY_LBRACKET:
		{
			data->v->prevThruster();
			return true;
		}
	case OAPI_KEY_RBRACKET:
		{
			data->v->nextThruster();
			return true;
		}
	case OAPI_KEY_EQUALS:
		{
			nextPage();
			InvalidateButtons();
			InvalidateDisplay();
			return true;
		}
	}
	if (vessel->GetThrusterCount() < 1)
		return false;
	switch(data->Page)
	{
	case PAGE_MAIN:
		switch(key)
		{
		case OAPI_KEY_R:
			{
				oapiOpenInputBox("Enter new ref [x y z]: ", VectorInputRef, 0, 20, data->v);
				return true;
			}
	    case OAPI_KEY_D:
			{
				oapiOpenInputBox("Enter new dir [x y z]: ", VectorInputDir, 0, 20, data->v);
				return true;
			}
		case OAPI_KEY_V:
			{
				oapiOpenInputBox("Enter new vacume rating [N]: ", ResetThrustRating, 0, 20, data->v);
				return true;
			}
		case OAPI_KEY_I:
			{
				oapiOpenInputBox("Enter new ISP [m/s]: " , SetISP, 0, 20, data->v);
				return true;
			}
		case OAPI_KEY_L:
			{
				oapiOpenInputBox("Enter thruster level [0-100]: ", SetThrusterLevel, 0, 20, data->v);
				return true;
			}
		case OAPI_KEY_G:
			{
				data->v->toggleGimbling();
				return true;
			}
		case OAPI_KEY_MINUS:
			{
				if (data->v->gimbleGroup()==THGROUP_MAIN)
					data->v->setGimbleGroup(THGROUP_HOVER);
				else
					data->v->setGimbleGroup(THGROUP_MAIN);
				return true;
			}
		}break;
	case PAGE_AP:
		switch(key)
		{
		case OAPI_KEY_W:
			{
				nextAP();
				return true;
			}
		case OAPI_KEY_Q:
			{
				if (data->v->ap[data->SelectedAP].thg==THGROUP_MAIN)
					data->v->ap[data->SelectedAP].thg = THGROUP_HOVER;
				else
					data->v->ap[data->SelectedAP].thg=THGROUP_MAIN;
				return true;
			}
		case OAPI_KEY_S:
			{
				oapiOpenInputBox("Enter setpoint: ", GetDouble, 0, 20, &data->v->ap[data->SelectedAP].sp);
				return true;
			}
		case OAPI_KEY_T:
			{
				if (data->v->ap[data->SelectedAP].active)
					data->v->ap[data->SelectedAP].active=false;
				else
					data->v->ap[data->SelectedAP].active=true;
				return true;
			}
		case OAPI_KEY_D:
			{
				oapiOpenInputBox("Enter burn duration: ", GetBurnDuration, 0, 20, data);
				return true;
			}
		case OAPI_KEY_B:
			{
				if (data->remoteThrust->isCountStarted() || data->remoteThrust->isBurning()){
					data->remoteThrust->Abort();
					return false;
				}
				if (data->remoteThrust->getCountdown()<=0)
					return false;
					data->remoteThrust->StartCountdown();
				return true;
			}
		case OAPI_KEY_C:
			{
				oapiOpenInputBox("Enter countdown: ", GetCountdown, 0, 20, data);
				return true;
			}

		}
		break;
	}
	return false;
}

bool TCMFD::ConsumeButton (int bt, int event)
{
	if (!(event & PANEL_MOUSE_LBDOWN)) return false;

	switch(data->Page)
	{
	case PAGE_MAIN:
		switch(bt)
		{
		case 0:
			return ConsumeKeyBuffered(OAPI_KEY_EQUALS);
		case 1:
			return ConsumeKeyBuffered(OAPI_KEY_RBRACKET);
	    case 2:
		    return ConsumeKeyBuffered(OAPI_KEY_LBRACKET);
	    case 3:
		    return ConsumeKeyBuffered(OAPI_KEY_R);
	    case 4:
		    return ConsumeKeyBuffered(OAPI_KEY_D);
	    case 5:
		    return ConsumeKeyBuffered(OAPI_KEY_V);
	    case 6:
	    	return ConsumeKeyBuffered(OAPI_KEY_I);
	    case 7:
		    return ConsumeKeyBuffered(OAPI_KEY_L);
	    case 8:
		    return ConsumeKeyBuffered(OAPI_KEY_G);
	    case 9:
		    return ConsumeKeyBuffered(OAPI_KEY_MINUS);
		}
	case PAGE_AP:
		switch(bt)
		{
		case 0:
			return ConsumeKeyBuffered(OAPI_KEY_EQUALS);
		case 1:
			return ConsumeKeyBuffered(OAPI_KEY_W);
		case 2:
			return ConsumeKeyBuffered(OAPI_KEY_Q);
		case 3:
			return ConsumeKeyBuffered(OAPI_KEY_S);
		case 4:
			return ConsumeKeyBuffered(OAPI_KEY_T);
		case 5:
			return ConsumeKeyBuffered(OAPI_KEY_D);
		case 6:
			return ConsumeKeyBuffered(OAPI_KEY_C);
		case 7:
			return ConsumeKeyBuffered(OAPI_KEY_B);
		}
	}
	return false;
}
int TCMFD::ButtonMenu (const MFDBUTTONMENU **menu) const
{
	switch(data->Page)
	{
	case PAGE_MAIN:
		{
			static const MFDBUTTONMENU mnu[10] = {
				{"Next Page", 0, '='},
				{"Next thruster",  0, ']'},
				{"Prev thruster",  0, '['},
		        {"Set  reference", 0, 'R'},
		        {"Set  direction", 0, 'D'},
		        {"Set  vac. rating", 0, 'V'},
		        {"Set  ISP", 0, 'I'},
	         	{"Set  Level", 0, 'L'},
	        	{"Toggle Gimbling", 0, 'G'},
		        {"Which engine?", 0, '-'}
			};
			if (menu) *menu = mnu;
			return 10;
		}
	case PAGE_AP:
		{
			static const MFDBUTTONMENU mnu[8] = {
				{"Next Page", 0, '='},
				{"Next AP", 0, 'W'},
				{"Which group?", 0, 'Q'},
				{"Enter setpoint", 0, 'S'},
				{"Toggle AP", 0, 'T'},
				{"Burn Duration", 0, 'D'},
				{"Set Countdown", 0, 'C'},
				{"Start/Abort burn", 0, 'B'}
			};
			if (menu) *menu = mnu;
			return 8;
		}
	}
	return 0;
}
char *TCMFD::ButtonLabel (int bt)
{
	switch(data->Page)
	{
	case PAGE_MAIN:
		{
			char *label[10] = {"PG", "Nxt", "Pre", "REF", "DIR","VAC", "ISP", "LVL","GMB","GM-"};
		    return (bt<10 ? label[bt] : 0);
		}
	case PAGE_AP:
		{
			char * label[8] = {"PG", "NAP", "GRP", "SP", "TGL", "BDT", "CNT", "STA"};
	 	    return (bt < 8 ? label[bt] : 0);
		}
	}
	return 0;
}
void TCMFD::PrintAP(HDC hDC)
{
	double y = height*2;
	char cbuf[255];
	int len;
	VESSEL * vessel = data->v->getHook();
	VECTOR3 hvel;
	double pval = 0;
	switch(data->SelectedAP)
	{
	case AP_HOV:
		len = sprintf(cbuf, "Autopilot       : V/S Holder");
		vessel->GetHorizonAirspeedVector(hvel);
		pval = hvel.y;
		break;
	case AP_VEL:
		len = sprintf(cbuf, "Autopilot       : Velocity Holder");
		pval = vessel->GetAirspeed();
		break;
	}
	TextOut(hDC, width, y, cbuf, len);
	y+=height*2;
	SetTextColor(hDC, GREEN);
	len = sprintf(cbuf, "Process Value   : %.2f", pval);
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1;
	len = sprintf(cbuf, "Set Point       : %.2f", data->v->ap[data->SelectedAP].sp);
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1;
	if (data->v->ap[data->SelectedAP].active)
		len = sprintf(cbuf, "State           : Active");
	else
		len = sprintf(cbuf, "State           : Inactive");
	TextOut(hDC, width, y, cbuf, len);
	y+=height*2;
	SetTextColor(hDC, LYEL);
	if (data->v->ap[data->SelectedAP].thg==THGROUP_MAIN)
		len = sprintf(cbuf, "Engine          : Main");
	else 
		len = sprintf(cbuf, "Engine          : Hover");
	TextOut(hDC, width, y, cbuf, len);
	y+=height*2;
	Rectangle(hDC, width, y, width*35, y+(height*2));
	HBRUSH hBrush = CreateSolidBrush(OVLG);
	HBRUSH hOld   = (HBRUSH)SelectObject(hDC, hBrush);
	double lvl = data->v->getHook()->GetThrusterGroupLevel((THGROUP_TYPE)data->v->ap[data->SelectedAP].thg);
	double x = width*35*lvl;
	if (x<width)
		x=width;
	Rectangle(hDC, width, y, x, y+(height*2));
	SelectObject(hDC, hOld);
	SetTextColor(hDC, GREEN);
	len = sprintf(cbuf, "Level: %.2f%%", lvl*100);
	TextOut(hDC, width*35/2-width*6, y+(height/2), cbuf, len);
	SetTextColor(hDC, LYEL);
	if (data->v->ap[data->SelectedAP].active)
	{
		for (int i = 0; i < 17; i++)
			TextOut(hDC, data->points[i].x, data->points[i].y,".",1);
	}
	y+=height*3;
	RTHRUST * rt = data->remoteThrust;
	if (rt->isCountStarted())
		len = sprintf(cbuf, "Countdown       : %.2f seconds", rt->getCountdownStartTime()-oapiGetSimTime());
	else
		len = sprintf(cbuf, "Countdown       : %.2f seconds", rt->getCountdown());
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1;
	if (rt->isBurning())
		len = sprintf(cbuf, "Duration        : %.2f seconds", rt->getBurnStartTime()-oapiGetSimTime());
	else
		len = sprintf(cbuf, "Duration        : %.2f seconds", rt->getBurnDuration());
	TextOut(hDC, width, y, cbuf, len);
	y+=height*2;
	if (rt->isCountStarted())
		len = sprintf(cbuf, "Progress        : Counting");
	else if (rt->isBurning())
		len = sprintf(cbuf, "Progress        : Burning");
	else
		len = sprintf(cbuf, "Progress        : IDLE");
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1.5;
	Rectangle(hDC, width, y, width*35, y+(height*2));
	hOld = (HBRUSH)SelectObject(hDC, hBrush);
	if (rt->isCountStarted())
		x = (rt->getCountdown()-(rt->getCountdownStartTime()-oapiGetSimTime()))/rt->getCountdown();
	else if (rt->isBurning())
		x = (rt->getBurnDuration()-(rt->getBurnStartTime()-oapiGetSimTime()))/rt->getBurnDuration();
	else
		x = 0;
	x=x*(width*35);
	if (x<width)
		x=width;
	Rectangle(hDC, width, y, x, y+(height*2));
	SelectObject(hDC, hOld);
	DeleteObject(hBrush);
}
void TCMFD::PrintMain(HDC hDC)
{
	double y      = height*2;
	char cbuf[255];
	int len;

	VESSEL * vessel = data->v->getHook();
	DWORD thcount = vessel->GetThrusterCount();
	THRUSTER_HANDLE th = data->v->getTargetHandle();
	VECTOR3 ref, dir;
	vessel->GetThrusterRef(th, ref);
	vessel->GetThrusterDir(th, dir);
	double currentRating = vessel->GetThrusterMax(th);
	double maxRating = vessel->GetThrusterMax0(th);
	double isp = vessel->GetThrusterIsp(th);
	double ispVac = vessel->GetThrusterIsp0(th);

	len = sprintf(cbuf, "Selected   : %d of %d", data->v->getTargetThruster()+1, thcount);
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1;
	SetTextColor(hDC, GREEN);
	len = sprintf(cbuf, "Position   : [%.2f] [%.2f] [%.2f]", ref.x, ref.y, ref.z);
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1;
	len = sprintf(cbuf, "Direction  : [%.2f] [%.2f] [%.2f]", dir.x, dir.y, dir.z);
	TextOut(hDC, width, y, cbuf, len);
	y+=height*2;
	SetTextColor(hDC, DTRQ);
	len = sprintf(cbuf, "Rating     : %.2f N", currentRating);
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1;
	len = sprintf(cbuf, "Max Rating : %.2f N", maxRating);
	TextOut(hDC, width, y, cbuf, len);
	y+=height*2;
	SetTextColor(hDC, LYEL);
	len = sprintf(cbuf, "ISP        : %.2f m/s", isp);
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1;
	len = sprintf(cbuf, "Vac. ISP   : %.2f m/s", ispVac);
	TextOut(hDC, width, y, cbuf, len);
	y+=height*1;
	if (data->v->gimbling())
	{
		char gtype[25];
		if (data->v->gimbleGroup()==THGROUP_MAIN)
			sprintf(gtype, "Main Engines");
		else
			sprintf(gtype, "Hover Engines");
		len = sprintf(cbuf, "Gimbling   : %s", gtype);
		TextOut(hDC, width, y, cbuf, len);
		y+=height*2;
	}else
		y+=height*1;
	SetTextColor(hDC, GREEN);
	y+=height*2;
	double percent = currentRating/maxRating;
	double x       = width*35*percent;
	if (x<width)
		x=width;
	Rectangle(hDC, width, y, width*35, y+height*2);
	HBRUSH hBrush = CreateSolidBrush(OVLG);
	HBRUSH hBOld  = (HBRUSH)SelectObject(hDC, hBrush);
	Rectangle(hDC, width, y, x, y+height*2);
	SelectObject(hDC, hBOld);
	len = sprintf(cbuf, "Rating: %.2f%%", percent*100);
	TextOut(hDC, width*35/2-width*6, y+height/2, cbuf, len);
	y+=height*3;
	percent = isp/ispVac;
	x = width*35*percent;
	if (x<width)
		x=width;
	Rectangle(hDC, width, y, width*35, y+height*2);
	hBOld = (HBRUSH)SelectObject(hDC, hBrush);
	Rectangle(hDC, width, y, x, y+height*2);
	SelectObject(hDC, hBOld);
	len = sprintf(cbuf, "ISP: %.2f%%", percent*100);
	TextOut(hDC, width*35/2-width*6, y+height/2, cbuf, len);
	y+=height*3;
	percent = vessel->GetThrusterLevel(th);
	x = width*35*percent;
	if (x<width)
		x=width;
	Rectangle(hDC, width, y, width*35, y+height*2);
	hBOld = (HBRUSH)SelectObject(hDC, hBrush);
	Rectangle(hDC, width, y, x, y+height*2);
	SelectObject(hDC, hBOld);
	len = sprintf(cbuf, "Level: %.2f%%", percent*100);
	TextOut(hDC, width*35/2-width*6, y+height/2, cbuf, len);
	y+=height*2.5;
	if (data->v->gimbling())
	{
		Rectangle(hDC, width, y, width*35, y+(height*1));
	    MoveToEx(hDC, width*35/2, y, NULL);
	    LineTo(hDC, width*35/2, y+(height*1));
		hBOld = (HBRUSH)SelectObject(hDC, hBrush);
		double clvl = (data->v->gimbleGroup()==THGROUP_MAIN ? vessel->GetControlSurfaceLevel(AIRCTRL_ELEVATOR) : vessel->GetControlSurfaceLevel(AIRCTRL_RUDDER));
		percent = fabs(clvl)*(width*35/2);
		if (clvl>0)
		{
			Rectangle(hDC, width*35/2, y, width*35/2+percent, y+(height*1));
		}else{
			HBRUSH hb2 = CreateSolidBrush(LYEL);
			HBRUSH hbl = (HBRUSH)SelectObject(hDC, hb2);
			Rectangle(hDC, (width*35/2-percent<width ? width : width*35/2-percent), y, width*35/2, y+(height*1));
			SelectObject(hDC, hbl);
			DeleteObject(hb2);
		}
		SelectObject(hDC, hBOld);
	}			
	DeleteObject(hBrush);
}
void TCMFD::Update (HDC hDC)
{
	Title (hDC, "Thruster Control MFD");

	double y      = height*2;
	char cbuf[255];
	int len;

	VESSEL * vessel = data->v->getHook();
	DWORD thcount = vessel->GetThrusterCount();
	if (thcount < 1)
	{
		len = sprintf(cbuf,"No thrusters detected on %s", vessel->GetName());
		TextOut(hDC, width, y, cbuf, len);
		y+=height*1;
		return;
	}
	switch(data->Page)
	{
	case PAGE_MAIN:
		PrintMain(hDC);
		break;
	case PAGE_AP:
		PrintAP(hDC);
		break;
	}
}
