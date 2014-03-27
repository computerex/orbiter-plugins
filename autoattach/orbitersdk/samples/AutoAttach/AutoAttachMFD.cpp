#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include "AutoAttachManager.h"
#include "AutoAttachMFD.h"

AAMANAGER a_MNG;
VESSEL *  v = NULL;
int       mode; 
int iSelectedAttachment=0;
bool scanning=false;
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	v=oapiGetFocusInterface();
	if (scanning)
	   a_MNG.TimeStep();
}
DLLCLBK void opcDLLInit (HINSTANCE hDLL)
{
	static char *name = "AutoAttach Manager MFD";
	MFDMODESPEC spec;
	spec.name    = name;
	spec.key     = OAPI_KEY_A;
	spec.msgproc = AAMANAGERMFD::MsgProc;
	mode = oapiRegisterMFDMode (spec);
}
DLLCLBK void opcDLLExit (HINSTANCE hDLL)
{
	oapiUnregisterMFDMode (mode);
}
AAMANAGERMFD::AAMANAGERMFD (DWORD w, DWORD h, VESSEL *vessel)
: MFD (w, h, vessel)
{
	width=w/35;
	height=h/24;
}
AAMANAGERMFD::~AAMANAGERMFD ()
{
}
int AAMANAGERMFD::MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case OAPI_MSG_MFD_OPENED:
		return (int)(new AAMANAGERMFD (LOWORD(wparam), HIWORD(wparam), (VESSEL*)lparam));
	}
	return 0;
}
bool AAMANAGERMFD::ConsumeKeyBuffered (DWORD key)
{
	switch (key)
	{
	case OAPI_KEY_A:
		{
			ATTACHMENTHANDLE hAttach = NULL, hAttach2=NULL;
			VECTOR3 pos, dir, rot, rpos;
			VESSEL * vParent=NULL;
			OBJHANDLE hVes=NULL;
			VESSELSTATUS vs;
			for (int i = 0; i < v->AttachmentCount(true); i++)
			{
				hAttach=v->GetAttachmentHandle(true, i);
				hVes=v->GetAttachmentStatus(hAttach);
				if (!oapiIsVessel(hVes))
					continue;
				vParent=oapiGetVesselInterface(hVes);
				for (int j = 0; j < vParent->AttachmentCount(false); j++)
				{
					hAttach2=vParent->GetAttachmentHandle(false, j);
					hVes=vParent->GetAttachmentStatus(hAttach2);
					if (!oapiIsVessel(hVes))
						continue;
					vParent->DetachChild(hAttach2, 1);
					vParent->GetStatus(vs);
					vParent->GetAttachmentParams(hAttach2, pos, dir, rot);
					oapiGetGlobalPos(vParent->GetHandle(), &rpos);
					vParent->Global2Local(rpos, rpos);
					rpos.x+=pos.x+(dir.x*1.1);
					rpos.y+=pos.y+(dir.y*1.1);
					rpos.z+=pos.z+(dir.z*1.1);
					vParent->Local2Rel(rpos, rpos);
					vs.rpos=rpos;
					vs.status=0;
					v->DefSetState(&vs);
				}
			}
			return true;
		}
	case OAPI_KEY_S:
		{
			if (scanning)
				scanning=false;
			else
				scanning=true;
			return true;
		}

	}
	return false;
}
void AAMANAGERMFD::Update (HDC hDC)
{
	Title (hDC, "AutoAttach Manager MFD");
	char cbuf[255];
	int len=0;

	len = sprintf(cbuf, "Shift + A to free child");
	TextOut(hDC, width, height*2, cbuf, len);
	len = sprintf(cbuf, "Shift + S to scan");
	TextOut(hDC, width, height*3, cbuf, len);

	if (scanning)
		len = sprintf(cbuf, "status: scanning");
	else
		len = sprintf(cbuf, "status: idle");
	TextOut(hDC, width, height*4, cbuf, len);
}