#define STRICT
#define ORBITER_MODULE

#include "Teleporter.h"

TELEPORTER::TELEPORTER(OBJHANDLE hVessel, int flightmodel) : VESSEL2(hVessel, flightmodel)
{
	sprintf(hDestVessel,"none");
	Tele_Distance =  20; 
}
void TELEPORTER::clbkPostCreation(void)
{
}
bool TELEPORTER::isTeleporter(OBJHANDLE hObj)
{
	if (!hObj)
		return false;
	if (!strcmp(VESSEL(hObj).GetClassName(),"Teleporter"))
		return true;
	return false;
}
void TELEPORTER::clbkSetClassCaps(FILEHANDLE cfg)
{
	SetSize (5);
	SetEmptyMass (5000.0);
	SetTouchdownPoints (_V(0,43.4,2), _V(-1,43.4,-1.5), _V(1,43.4,-1.5));
	AddMesh("Teleporter");
}
bool TELEPORTER::AreVesselsInRange()
{
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		if (GetHandle() == oapiGetVesselByIndex(i))
			continue;
		VECTOR3 relpos;
		GetRelativePos(oapiGetVesselByIndex(i),relpos);
		double dist = length(relpos);
		if (dist <= Tele_Distance)
		return true;
	}
	return false;
}
void TELEPORTER::clbkLoadStateEx(FILEHANDLE scn, void *vs)
{
	char * line;
	char cbuf[255];
	while (oapiReadScenario_nextline(scn, line))
	{
		if (!strnicmp (line, "DIST", 4))
		{
			sscanf(line+4, "%lf",&Tele_Distance);
		}
		if (!strnicmp (line, "TARGET", 6))
		{
			sscanf(line+6, "%s", hDestVessel);
		}
		else
		{
			ParseScenarioLineEx(line, vs);
		}
	}
}
void TELEPORTER::clbkSaveState(FILEHANDLE scn)
{
	char cbuf[255];
	VESSEL2::clbkSaveState(scn);
	sprintf(cbuf,"%s",hDestVessel);
	oapiWriteScenario_string(scn,"TARGET",cbuf);
	sprintf(cbuf,"%f",Tele_Distance);
	oapiWriteScenario_string(scn,"DIST",cbuf);
}
OBJHANDLE TELEPORTER::GetVesselInRange()
{
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		if (GetHandle() == oapiGetVesselByIndex(i))
			continue;
		VECTOR3 relp;
		GetRelativePos(oapiGetVesselByIndex(i),relp);
		if (length(relp) < Tele_Distance)
			return oapiGetVesselByIndex(i);
	}
	return NULL;
}
bool TargetInput (void *id, char *str, void *user_data)
{
	bool found = false;
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		if (oapiGetVesselByName(str) == oapiGetVesselByIndex(i))
		{
			if (!strcmp(VESSEL(oapiGetVesselByName(str)).GetClassName(),"Teleporter"))
			{
				found = true;
				sprintf(((TELEPORTER*)user_data)->hDestVessel,"%s",str);
				break;
			}
		}
	}
	if (!found)
		return false;
	else
		return true;
}
bool TeleDistance (void*id, char*str, void * user_data)
{
	*(double*)user_data=atof(str);
	return true;
}
int TELEPORTER::clbkConsumeBufferedKey(DWORD key, bool down, char *kstate)
{
	if (!down) return 0;

	switch(key)
	{
	case OAPI_KEY_LBRACKET:
		{
			oapiOpenInputBox("Enter teleporter's name: ",TargetInput,0,20,this);
			return 0;
		}
	case OAPI_KEY_D:
		{
			oapiOpenInputBox("Enter teleportation distance: ", TeleDistance, 0, 20, &Tele_Distance);
			return true;
		}
	default:
		return 0;
	}
	return 0;
}
void TELEPORTER::clbkPreStep(double simt, double simdt, double mjd)
{
	static double eTime    = oapiGetSimTime();
	static bool   effects  = false;
	static double aperture = oapiCameraAperture(); 
	if(AreVesselsInRange())
	{
		if (oapiGetVesselByName(hDestVessel))
		{
			if (isTeleporter(oapiGetVesselByName(hDestVessel)))
			{
				OBJHANDLE hVes = GetVesselInRange();
				OBJHANDLE hTel = oapiGetVesselByName(hDestVessel);
				VESSEL * v = oapiGetVesselInterface(hTel);
				VESSEL * medium = oapiGetVesselInterface(hVes);
				VESSELSTATUS vs; v->GetStatus(vs);
				VESSELSTATUS vs2; medium->GetStatus(vs2);
				VECTOR3 Pos;
				oapiGetGlobalPos(v->GetHandle(),&Pos);
				v->Global2Local(Pos,Pos);
				Pos.x+=((TELEPORTER*)v)->Tele_Distance+5;
				Pos.y+=((TELEPORTER*)v)->Tele_Distance+5;
				Pos.z+=((TELEPORTER*)v)->Tele_Distance+5;
				v->Local2Rel(Pos,Pos);
				vs2.arot=vs.arot;
				for (int i = 0; i < 10; i++)
					vs2.vdata[i]=vs.vdata[i];
				vs2.vrot=vs.vrot;
				vs2.rpos=Pos;
				vs2.rbody=vs.rbody;
				vs2.rvel=vs.rvel;
				vs2.status=vs.status;
				medium->DefSetState(&vs2);
				eTime=oapiGetSimTime();
				effects = true;
				aperture = oapiCameraAperture();
			}
		}
	}
	if (effects)
	{
		if (oapiGetSimTime() < eTime+3)
		{
			TeleEffect();
		}
		else{
			oapiCameraSetAperture(aperture);
			effects = false;
		}
	}
	
}
void TELEPORTER::TeleEffect(void)
{
	static double ITime = oapiGetSimTime();
    static double md    = 1;
	if (oapiGetSimTime() > ITime+0.01)
	{
		ITime=oapiGetSimTime();
		if (md==1)
			md++;
		else md--;
	}
	if (md==1)
		oapiCameraSetAperture(oapiCameraAperture()-0.01);
	else
		oapiCameraSetAperture(oapiCameraAperture()+0.01);
}
int TELEPORTER::ResolveIndex(OBJHANDLE hVes)
{
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		if (hVes == oapiGetVesselByIndex(i))
			return i;
	}
	return -1;
}
void TELEPORTER::clbkDrawHUD(int mode, const HUDPAINTSPEC *hps, HDC hDC)
{
	VESSEL2::clbkDrawHUD(mode, hps, hDC);
	char cbuf[255];

	OBJHANDLE hVessel = oapiGetVesselByName(hDestVessel);
	if (!hVessel || !isTeleporter(hVessel))
	{
		sprintf(cbuf,"Teleporter name: none selected");
		TextOut(hDC,10,150,cbuf,strlen(cbuf));
		return;
	}
	sprintf(cbuf,"Teleporter name: %s",hDestVessel);
	TextOut(hDC,10,150,cbuf,strlen(cbuf));
	sprintf(cbuf,"Spherical field: %.2f meters",Tele_Distance);
	TextOut(hDC,10,170,cbuf,strlen(cbuf));
}
DLLCLBK VESSEL *ovcInit (OBJHANDLE hvessel, int flightmodel)
{
	return new TELEPORTER (hvessel, flightmodel);
}

// --------------------------------------------------------------
// Vessel cleanup
// --------------------------------------------------------------
DLLCLBK void ovcExit (VESSEL *vessel)
{
	if (vessel) delete (TELEPORTER*)vessel;
}