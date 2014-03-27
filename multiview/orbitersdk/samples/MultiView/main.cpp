#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include "MultiViewManager.h"
#include "resource.h"

MULTIVIEWMANAGER g_MVM;
HINSTANCE g_hInst; 
DWORD g_dwCmd;
int SelectedView = 0;
OBJHANDLE hFocus=NULL;

void OpenDlgClbk (void *context);
BOOL CALLBACK MsgProc (HWND, UINT, WPARAM, LPARAM);

DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	g_MVM.TimeStep();
}

DLLCLBK void opcDLLInit (HINSTANCE hDLL)
{
	g_hInst = hDLL; 
	g_dwCmd = oapiRegisterCustomCmd ("Camera Views",
		"Switch camera views",
		OpenDlgClbk, NULL);
}
DLLCLBK void opcDLLExit (HINSTANCE hDLL)
{
	oapiUnregisterCustomCmd (g_dwCmd);
}
void OpenDlgClbk (void *context)
{
	if (!g_MVM.FocusIsRegistered())
		return;
	VESSEL * v = oapiGetFocusInterface();
	hFocus=v->GetHandle();
	v->GetCameraDefaultDirection(g_MVM.defaultCam.dir);
	v->GetCameraOffset(g_MVM.defaultCam.pos);
	SelectedView = 0;
	oapiOpenDialog (g_hInst, IDD_DIALOG1, MsgProc);
}
void CloseDlg (HWND hDlg)
{
	SelectedView = 0;
	hFocus=NULL;
	oapiCloseDialog (hDlg);
}
void UpdateText(HWND hDlg, CVTYPE cv)
{
	char cbuf[255];
	sprintf(cbuf,"%d of %d views",SelectedView+1,cv.cameras.size());
	SetDlgItemText(hDlg,IDC_TEXT_VIEW,cbuf);
}
BOOL CALLBACK MsgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!g_MVM.FocusIsRegistered())
		CloseDlg(hDlg);
	CVTYPE cv = g_MVM.GetFocus();
	VESSEL * v = oapiGetFocusInterface();
	if (hFocus!=v->GetHandle())
		CloseDlg(hDlg);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			char cbuf[255];
			sprintf(cbuf,"Default View");
			SetDlgItemText(hDlg, IDC_TEXT_VIEW,cbuf);
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
					CloseDlg (hDlg);
					return TRUE;
				}
			case IDC_BUTTON_NEXT:
				{
					if (SelectedView+1 >= cv.cameras.size())
						SelectedView=0;
					else
						SelectedView++;
					v->SetCameraDefaultDirection(cv.cameras[SelectedView].dir);
					v->SetCameraOffset(cv.cameras[SelectedView].pos);
					UpdateText(hDlg, cv);
					return TRUE;
				}
			case IDC_BUTTON_DEFAULT:
				{
					char cbuf[255];
					SelectedView=0;
					v->SetCameraDefaultDirection(g_MVM.defaultCam.dir);
					v->SetCameraOffset(g_MVM.defaultCam.pos);
					sprintf(cbuf,"Default View");
					SetDlgItemText(hDlg, IDC_TEXT_VIEW,cbuf);
					return TRUE;
				}
			}
			break;
		}
	}
	return oapiDefDialogProc (hDlg, uMsg, wParam, lParam);
}
