#include "FileReader.h"
#include "orbitersdk.h"
#include "SpotLightManager.h"

FILEREADER g_FR;

bool SPOTLIGHTMANAGER::isSpacecraft(VESSEL * v)
{
	if (!v)
		return false;
	if (v->GetClassName()==NULL)
		return true;
	if (!strncmp(v->GetClassName(),"Spacecraft",10))
		return true;
	return false;
}
bool SPOTLIGHTMANAGER::hasConfig(VESSEL * v)
{
	if (!v)
		return false;
	char cbuf[255];
	if (isSpacecraft(v))
		sprintf(cbuf,"./Config/spotlights/%s.ini",v->GetName());
	else
		sprintf(cbuf,"./Config/spotlights/%s.ini",v->GetClassName());
	if (fopen(cbuf,"r"))
		return true;
	return false;
}
std::vector<SPOTLIGHT*> SPOTLIGHTMANAGER::ReadSpotlightData(char * inifile)
{
	std::vector<SPOTLIGHT*> spList;
	char cbuf[255];

	int spCount = 0;
	g_FR.LoadInt(inifile,"SETTINGS","count",spCount);

	double w = 0, l = 0;
	double lvl = 0;
	VECTOR3 pos, dir;
	char tex[255];
	for (int i = 0; i < spCount; i++)
	{
		sprintf(cbuf,"SP_%d_width",i+1);
		g_FR.LoadDouble(inifile,"SETTINGS",cbuf,w);
		sprintf(cbuf,"SP_%d_length",i+1);
		g_FR.LoadDouble(inifile,"SETTINGS",cbuf,l);
		sprintf(cbuf,"SP_%d_pos",i+1);
		g_FR.LoadVector(inifile,"SETTINGS",cbuf,pos);
		sprintf(cbuf,"SP_%d_dir",i+1);
		g_FR.LoadVector(inifile,"SETTINGS",cbuf,dir);
		sprintf(cbuf,"SP_%d_lvl",i+1);
		g_FR.LoadDouble(inifile,"SETTINGS",cbuf,lvl);
		sprintf(cbuf,"SP_%d_tex",i+1);
		g_FR.LoadString(inifile,"SETTINGS",cbuf,tex);
		spList.push_back(new SPOTLIGHT(w,l,lvl,pos,dir,tex));
	}
	return spList;
}
std::vector<BEACON*> SPOTLIGHTMANAGER::ReadBeaconData(char * inifile)
{
	std::vector<BEACON*> bList;
	char cbuf[255];

	int bCount = 0;
	g_FR.LoadInt(inifile,"SETTINGS","BeaconCount",bCount); 

	BEACONLIGHTSPEC bcd;
	VECTOR3 pos, col;
	VECTOR3 *ppos, *pcol;
	for (int i = 0; i < bCount; i++)
	{
		ppos = new VECTOR3();
		pcol = new VECTOR3();
		char sBType[255];
		
		sprintf(cbuf,"B_%d_pos",i+1);
		g_FR.LoadVector(inifile,"SETTINGS",cbuf,pos);
		sprintf(cbuf,"B_%d_col",i+1);
		g_FR.LoadVector(inifile,"SETTINGS",cbuf,col);
		*(ppos)=pos;
		*(pcol)=col;
		bcd.pos=ppos;
		bcd.col=pcol;
		sprintf(cbuf,"B_%d_size",i+1);
		g_FR.LoadDouble(inifile,"SETTINGS",cbuf,bcd.size);
		sprintf(cbuf,"B_%d_falloff",i+1);
		g_FR.LoadDouble(inifile,"SETTINGS",cbuf,bcd.falloff);
		sprintf(cbuf,"B_%d_period",i+1);
		g_FR.LoadDouble(inifile,"SETTINGS",cbuf,bcd.period);
		sprintf(cbuf,"B_%d_duration",i+1);
		g_FR.LoadDouble(inifile,"SETTINGS",cbuf,bcd.duration);
		sprintf(cbuf,"B_%d_tofs",i+1);
		g_FR.LoadDouble(inifile,"SETTINGS",cbuf,bcd.tofs);
		sprintf(cbuf,"B_%d_shape",i+1);
		g_FR.LoadString(inifile,"SETTINGS",cbuf,sBType);
		if (!strcmp(sBType,"compact"))
		{
			bcd.shape=BEACONSHAPE_COMPACT;
		}else if (!strcmp(sBType,"diffuse"))
		{
			bcd.shape=BEACONSHAPE_DIFFUSE;
		}else
			bcd.shape=BEACONSHAPE_STAR;
		bcd.active=true;
		bList.push_back(new BEACON(bcd));
	}
	return bList;
}
bool SPOTLIGHTMANAGER::isAttended(VESSEL *v)
{
	if (!v)
		return false;
	for(int i = 0; i < atList.size(); i++)
	{
		if (oapiIsVessel(atList[i]->hook))
		{
			if (atList[i]->hook == v->GetHandle())
			{
				return true;
			}
		}
	}
	return false;
}
void SPOTLIGHTMANAGER::AssignSpotlights(VOBJ * vbj)
{
	if (!vbj)
		return;
	if (!oapiIsVessel(vbj->hook))
		return;
	VESSEL * vessel = oapiGetVesselInterface(vbj->hook);
	PROPELLANT_HANDLE hpr = vessel->CreatePropellantResource(1);
	double l, w;
	VECTOR3 pos, dir;
	char tex[255];
	for (int i = 0; i < vbj->lights.size(); i++)
	{
		if (!vbj->lights[i])
			continue;
		l    = vbj->lights[i]->l;
		w    = vbj->lights[i]->w;
		dir  = vbj->lights[i]->dir;
		pos  = vbj->lights[i]->pos;
		sprintf(tex,"%s",vbj->lights[i]->texture);
		vbj->lights[i]->hHandle=vessel->CreateThruster(pos,dir,1e0,hpr,50000000000e50);
		SURFHANDLE hTex = oapiRegisterExhaustTexture(tex);
		vessel->AddExhaust(vbj->lights[i]->hHandle,l,w,hTex);
		vessel->SetThrusterLevel(vbj->lights[i]->hHandle,vbj->lights[i]->lvl);
	}
}
void SPOTLIGHTMANAGER::AssignBeacons(VOBJ * vbj)
{
	if (!vbj)
		return;
	if (!oapiIsVessel(vbj->hook))
		return;
	VESSEL * vessel = oapiGetVesselInterface(vbj->hook);
	for (int i = 0; i < vbj->beacons.size(); i++)
	{
		if (!vbj->beacons[i])
			continue;
		vessel->AddBeacon(&vbj->beacons[i]->beacon);
	}
}
void SPOTLIGHTMANAGER::GetConfigurations()
{
	char inifile[255];
	char cbuf[255];

	OBJHANDLE hVes = NULL;
	VESSEL * v = NULL;
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		hVes = oapiGetVesselByIndex(i);
		if (!oapiIsVessel(hVes))
			continue;
		v = oapiGetVesselInterface(hVes);
		if (isAttended(v))
			continue;
		if (!hasConfig(v))
			continue;
		VOBJ * oVes = new VOBJ();
		oVes->hook=hVes;
		if (isSpacecraft(v))
			sprintf(inifile,"./Config/spotlights/%s.ini",v->GetName());
		else
			sprintf(inifile,"./Config/spotlights/%s.ini",v->GetClassName());
		oVes->lights=ReadSpotlightData(inifile);
		oVes->beacons=ReadBeaconData(inifile);
		AssignSpotlights(oVes);
		AssignBeacons(oVes);
		atList.push_back(oVes);
	}
}
void SPOTLIGHTMANAGER::TimeStep()
{
	static bool acc = false;
	GetConfigurations();
	VESSEL * vessel;

	if (oapiGetTimeAcceleration() > 1 && !acc)
	{
		acc=true;
		for (int i = 0; i < atList.size(); i++)
		{
			if (!atList[i])
				continue;
			if (!oapiIsVessel(atList[i]->hook))
				continue;
			atList[i]->status(false);
		}
	}
	if (oapiGetTimeAcceleration() <= 1 && acc)
	{
		acc = false;
	}
	if (!acc)
	{
		for (int i = 0; i < atList.size(); i++)
		{
			if (!atList[i])
				continue;
			if (!oapiIsVessel(atList[i]->hook))
				continue;
			if (atList[i]->night())
			{
				if (!atList[i]->active())
					atList[i]->status(true);
			}else
			{
				if (atList[i]->active())
					atList[i]->status(false);
			}
		}
	}
}