#include <orbitersdk.h>
#include "BOOMMANAGER.h"
#include "FileReader.h"
FILEREADER g_FR;

PARTICLESTREAMSPEC exhaust_hover = {
		0, 1.0, 5, 50, 0.3, 2.0, 3, 2.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_SQRT, 0, 1,
		PARTICLESTREAMSPEC::ATM_PLOG, -0.1, 0.1
};

double acc(VESSEL * vessel)
{
	VECTOR3 W,F,A;
    vessel->GetForceVector(F);
    vessel->GetWeightVector(W);
    A=(F-W)/vessel->GetMass();
	return A.z;
}
bool positive_acc(VESSEL * v)
{
	if (acc(v) > 0)
		return true;
	return false;
}
bool barrier_broken(VESSEL * v)
{
	if (v->GetMachNumber() >= 1)
		return true;
	return false;
}
void Shiver(void)
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
void CreateResourceAndStreams(VOBJ * vbj)
{
	if (!oapiIsVessel(vbj->hook))
		return;
	VESSEL * vessel = oapiGetVesselInterface(vbj->hook);
	PROPELLANT_HANDLE hpr = vessel->CreatePropellantResource(1);
	vbj->th = vessel->CreateThruster(_V(0,0,0),_V(0,0,1),1e0,hpr,500000e50);
	for (int i = 0; i < vbj->streams.size(); i++)
	{
		vessel->AddExhaustStream(vbj->th,vbj->streams[i]->pos,vbj->streams[i]->stream);
	}
}
VOBJ * BOOMMANAGER::ReadAndAssign(char * ini, VESSEL * v)
{
	char cbuf[255], str[255];
	VOBJ * vbj = new VOBJ();
	std::vector<PVISUAL*> visuals;
	int pCount = 0;
	g_FR.LoadInt(ini,"SETTINGS","pcount",pCount);

    PVISUAL *vis;
	PARTICLESTREAMSPEC *ps;
	VECTOR3 pos;
	for (int i = 0; i < pCount; i++)
	{
		ps = new PARTICLESTREAMSPEC();
		sprintf(cbuf,"P_%d_srcsize",i+1);
		g_FR.LoadDouble(ini,"SETTINGS",cbuf,ps->srcsize);
		sprintf(cbuf,"P_%d_srcrate",i+1);
		g_FR.LoadDouble(ini,"SETTINGS",cbuf,ps->srcrate);
		sprintf(cbuf,"P_%d_v0",i+1);
		g_FR.LoadDouble(ini,"SETTINGS",cbuf,ps->v0);
		sprintf(cbuf,"P_%d_srcspread",i+1);
		g_FR.LoadDouble(ini,"SETTINGS",cbuf,ps->srcspread);
		sprintf(cbuf,"P_%d_lifetime",i+1);
		g_FR.LoadDouble(ini,"SETTINGS",cbuf,ps->lifetime);
		sprintf(cbuf,"P_%d_growthrate",i+1);
		g_FR.LoadDouble(ini,"SETTINGS",cbuf,ps->growthrate);
		sprintf(cbuf,"P_%d_atmslowdown",i+1);
		g_FR.LoadDouble(ini,"SETTINGS",cbuf,ps->atmslowdown);
		sprintf(cbuf,"P_%d_lmin",i+1);
		g_FR.LoadDouble(ini,"SETTINGS",cbuf,ps->lmin);
		sprintf(cbuf,"P_%d_lmax",i+1);
		g_FR.LoadDouble(ini,"SETTINGS",cbuf,ps->lmax);
		sprintf(cbuf,"P_%d_amin",i+1);
		g_FR.LoadDouble(ini,"SETTINGS",cbuf,ps->amin);
		sprintf(cbuf,"P_%d_amax",i+1);
		g_FR.LoadDouble(ini,"SETTINGS",cbuf,ps->amax);
		sprintf(cbuf,"P_%d_pos",i+1);
		g_FR.LoadVector(ini,"SETTINGS",cbuf,pos);
		sprintf(str,"P_%d_LTYPE",i+1);
		g_FR.LoadString(ini,"SETTINGS",str,cbuf);
		if (!strcmp(cbuf,"EMISSIVE"))
			ps->ltype=PARTICLESTREAMSPEC::EMISSIVE;
		else if (!strcmp(cbuf,"DIFFUSE"))
			ps->ltype=PARTICLESTREAMSPEC::DIFFUSE;
		else
			ps->ltype=PARTICLESTREAMSPEC::DIFFUSE;
		sprintf(str,"P_%d_LEVELMAP",i+1);
		g_FR.LoadString(ini,"SETTINGS",str,cbuf);
		if (!strcmp(cbuf,"LVL_FLAT"))
			ps->levelmap=PARTICLESTREAMSPEC::LVL_FLAT;
		else if(!strcmp(cbuf,"LVL_LIN"))
			ps->levelmap=PARTICLESTREAMSPEC::LVL_LIN;
		else if(!strcmp(cbuf,"LVL_SQRT"))
			ps->levelmap=PARTICLESTREAMSPEC::LVL_SQRT;
		else if(!strcmp(cbuf,"LVL_PLIN"))
			ps->levelmap=PARTICLESTREAMSPEC::LVL_PLIN;
		else if (!strcmp(cbuf,"LVL_PSQRT"))
			ps->levelmap=PARTICLESTREAMSPEC::LVL_PSQRT;
		else
			ps->levelmap=PARTICLESTREAMSPEC::LVL_SQRT;
		sprintf(str,"P_%d_ATMSMAP",i+1);
		g_FR.LoadString(ini,"SETTINGS",str,cbuf);
		if (!strcmp(cbuf,"ATM_FLAT"))
			ps->atmsmap=PARTICLESTREAMSPEC::ATM_FLAT;
		else if (!strcmp(cbuf,"ATM_PLIN"))
			ps->atmsmap=PARTICLESTREAMSPEC::ATM_PLIN;
		else if (!strcmp(cbuf,"ATM_PLOG"))
			ps->atmsmap=PARTICLESTREAMSPEC::ATM_PLOG;
		else
			ps->atmsmap=PARTICLESTREAMSPEC::ATM_PLOG;
		sprintf(str,"P_%d_tex",i+1);
		g_FR.LoadString(ini,"SETTINGS",str,cbuf);
		if (strcmp(cbuf,"none"))
		{
			SURFHANDLE texture = oapiRegisterExhaustTexture(cbuf);
			ps->tex=texture;
		}
		vis = new PVISUAL(ps,pos);
		visuals.push_back(vis);
	}
	g_FR.LoadString(ini,"SETTINGS","sound",cbuf);
	if (!strcmp("default",cbuf))
		sprintf(vbj->sound,"Sound/Vessel/sonicbang.wav");
	else
		sprintf(vbj->sound,"%s",cbuf);
	g_FR.LoadDouble(ini,"SETTINGS","duration",vbj->duration);
	vbj->hook=v->GetHandle();
	vbj->streams=visuals;
	CreateResourceAndStreams(vbj);
	return vbj;
}
char * BOOMMANAGER::INI(VESSEL * v)
{
	if (!v)
		return NULL;
	if (!hasConfig(v))
		return NULL;
	if (isSpacecraft(v))
		return v->GetName();
	else
		return v->GetClassName();
	return NULL;
}
bool isFocus(VESSEL * v)
{
	if (oapiGetFocusInterface()->GetHandle() == v->GetHandle())
		return true;
	return false;
}
void BOOMMANAGER::TimeStep(void)
{
	static bool Init = false;
	if (!Init)
	{
		MyID=ConnectMFDToOrbiterSound("orbSonicBoom_computerex");
		Init=true;
	}
	VESSEL * v = 0;
	OBJHANDLE hVes = 0;
	char ini[255];
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		hVes = oapiGetVesselByIndex(i);
		v = oapiGetVesselInterface(hVes);
		if (isAttended(v))
			continue;
		if (!hasConfig(v))
			continue;
		sprintf(ini,"./Config/orbSonicBoom/%s.ini",INI(v));
		atList.push_back(ReadAndAssign(ini,v));
	}
	VOBJ * vbj;
	for (int i = 0; i < atList.size(); i++)
	{
		if (!atList[i])
			continue;
		if (!oapiIsVessel(atList[i]->hook))
			continue;
		vbj = atList[i];
		v=oapiGetVesselInterface(vbj->hook);;
		if (positive_acc(v) && barrier_broken(v))
		{
			if (!vbj->bk_mach)
			{
				if (!vbj->th)
					CreateResourceAndStreams(vbj);
				v->SetThrusterLevel(vbj->th,1);
				vbj->shiver=true;
				if (isFocus(v))
				{
					LoadMFDWave(MyID,1,vbj->sound);
					PlayMFDWave(MyID,1);
				}
				vbj->LP_Time=oapiGetSimTime();
				vbj->bk_mach=true;
			}
		}else if (!barrier_broken(v))
		{
			if (!vbj->th)
				CreateResourceAndStreams(vbj);
			v->SetThrusterLevel(vbj->th,0);
			vbj->bk_mach=false;
			vbj->shiver=false;
			StopMFDWave(MyID,0);
		}
		if (vbj->bk_mach)
		{
			if (oapiGetSimTime() > vbj->LP_Time+vbj->duration)
			{
				if (!vbj->th)
					CreateResourceAndStreams(vbj);
				v->SetThrusterLevel(vbj->th,0);
				vbj->shiver=false;
				StopMFDWave(MyID,0);
			}
		}
		if (vbj->shiver && isFocus(v))
			Shiver();
	}

}
bool BOOMMANAGER::isSpacecraft(VESSEL * v)
{
	if (!v)
		return false;
	if (v->GetClassName() == NULL)
		return true;
	if (!strncmp("Spacecraft",v->GetClassName(),10))
		return true;
	return false;
}
bool BOOMMANAGER::hasConfig(VESSEL * v)
{
	if (!v)
		return false;
	char cbuf[255];
	if (isSpacecraft(v))
		sprintf(cbuf,"./Config/orbSonicBoom/%s.ini",v->GetName());
	else
		sprintf(cbuf,"./Config/orbSonicBoom/%s.ini",v->GetClassName());
	if (fopen(cbuf,"r"))
		return true;
	else
		return false;
	return false;
}
bool BOOMMANAGER::isAttended(VESSEL * v)
{
	if (!v)
		return false;
	VESSEL * vessel;
	for (int i = 0; i < atList.size(); i++)
	{
		if (!atList[i])
			continue;
		if (!oapiIsVessel(atList[i]->hook))
			continue;
		vessel=oapiGetVesselInterface(atList[i]->hook);
		if (v->GetHandle() == vessel->GetHandle())
			return true;
	}
	return false;
}
