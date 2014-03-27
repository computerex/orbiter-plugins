#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include "resource.h"
#include <vector>

HINSTANCE g_hInst;  
DWORD     g_dwCmd; 
double    fDistance = 5000; 
VESSEL *  v = NULL;
VECTOR3   vpos;
OBJHANDLE hvessel;
std::vector<OBJHANDLE> hvList;
void      OpenDlgClbk (void *context);
BOOL      CALLBACK MsgProc (HWND, UINT, WPARAM, LPARAM);

DLLCLBK void opcDLLInit (HINSTANCE hDLL)
{
	g_hInst = hDLL; 
	g_dwCmd = oapiRegisterCustomCmd ("Simon",
		"Set the field distance",
		OpenDlgClbk, NULL);
}
DLLCLBK void opcDLLExit (HINSTANCE hDLL)
{
	oapiUnregisterCustomCmd (g_dwCmd);
}
void OpenDlgClbk (void *context)
{
	HWND hDlg = oapiOpenDialog (g_hInst, IDD_DIALOG1, MsgProc);
}void CloseDlg (HWND hDlg)
{
	oapiCloseDialog (hDlg);
}
BOOL CALLBACK MsgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char cbuf[255];
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			sprintf(cbuf,"%.2f", fDistance);
			SetDlgItemText(hDlg, IDC_EDIT_DIST, cbuf);
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
					GetDlgItemText(hDlg, IDC_EDIT_DIST, cbuf, 255);
					if (strlen(cbuf))
						fDistance=atof(cbuf);
					CloseDlg (hDlg);
					return TRUE;
				}
			}
		}
		break;
	}
	return oapiDefDialogProc (hDlg, uMsg, wParam, lParam);
}
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	hvList.clear();
	v = oapiGetFocusInterface();
	double engLevels[3];
	for (int i = 0; i < 3; i++)
		engLevels[i]=v->GetThrusterGroupLevel((THGROUP_TYPE)i);
	double attlevels[6];
	if (v->GetAttitudeMode()==ATTMODE_ROT)
		for (int i = 3, x = 0; i <= 8; i++){
			attlevels[x]=v->GetThrusterGroupLevel((THGROUP_TYPE)i);
			x++;
		}
	else if (v->GetAttitudeMode()==ATTMODE_LIN)
		for (int i = 9, x = 0; i <= 14; i++){
			attlevels[x]=v->GetThrusterGroupLevel((THGROUP_TYPE)i);
			x++;
		}else
			for (int i = 0; i < 6; i++)
				attlevels[i]=0;
	double surflevels[6];
	for (int i = 0; i < 6; i++)
		surflevels[i]=v->GetControlSurfaceLevel((AIRCTRL_TYPE)i);
	DWORD inx = oapiGetVesselCount();
	for (DWORD i = 0; i < inx; i++)
	{
		hvessel=oapiGetVesselByIndex(i);
		if (hvessel==v->GetHandle())
			continue;
		v->GetRelativePos(hvessel, vpos);
		if (length(vpos) > fDistance)
			continue;
		hvList.push_back(hvessel);
	}
	for (DWORD i = 0; i < hvList.size(); i++)
	{
		v = oapiGetVesselInterface(hvList[i]);
		if (v->GetAttitudeMode()==ATTMODE_ROT)
			for (int i = 3, x = 0; i <= 8; i++){
				v->SetThrusterGroupLevel((THGROUP_TYPE)i, attlevels[x]);
				x++;
			}
		else if (v->GetAttitudeMode()==ATTMODE_LIN)
			for (int i = 9, x = 0; i <= 14; i++){
				v->SetThrusterGroupLevel((THGROUP_TYPE)i, attlevels[x]);
				x++;
			}
			for (int i = 0; i < 6; i++)
				v->SetControlSurfaceLevel((AIRCTRL_TYPE)i, surflevels[i]);
			for (int i = 0; i < 3; i++)
				v->SetThrusterGroupLevel((THGROUP_TYPE)i, engLevels[i]);
	}
}