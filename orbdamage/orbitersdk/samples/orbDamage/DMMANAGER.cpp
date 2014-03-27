#include "DMMANAGER.h"

bool DMMANAGER::isSpacecraft(OBJHANDLE h)
{
	if (!oapiIsVessel(h))
		return false;
	VESSEL * v = oapiGetVesselInterface(h);
	if (v->GetClassName() == NULL)
		return true;
	if (!strnicmp("Spacecraft",v->GetClassName(),10))
		return true;
	return false;
}
bool DMMANAGER::hasConfig(OBJHANDLE h)
{
	if (!oapiIsVessel(h))
		return false;
	char cbuf[255];

	if (isSpacecraft(h))
		sprintf(cbuf,"%s%s.cfg",CFGPATH,VESSEL(h).GetName());
	else
		sprintf(cbuf,"%s%s.cfg",CFGPATH,VESSEL(h).GetClassName());
	if (fopen(cbuf,"r"))
		return true;
	return false;
}
bool DMMANAGER::isRegistered(OBJHANDLE hves)
{
	for (int i = 0; i < atlist.size(); i++)
	{
		if (!atlist[i])
			continue;
		if (!oapiIsVessel(atlist[i]->hook))
			continue;
		if (hves==atlist[i]->hook)
			return true;
	}
	return false;
}
VPROFILE DMMANAGER::Register(OBJHANDLE hVes)
{
	VPROFILE prf;
	VESSEL * v = oapiGetVesselInterface(hVes);
	char cfg[255];

	if (isSpacecraft(hVes)){
		strcpy(prf.name,v->GetName());
		sprintf(cfg,"%s%s.cfg", CFGPATH, v->GetName());
	}
	else{
		strcpy(prf.name,v->GetClassName());
		sprintf(cfg, "%s%s.cfg", CFGPATH, v->GetClassName());
	}
	PARSER prs(cfg);
	prs.read();
	prf.limit.maxAccel=prs.Double("MinMaxAccel");
	prf.limit.maxCollision=prs.Double("MaxGroundCollision");
	prf.limit.maxFlux=prs.Double("MaxFlux");
	prf.limit.maxG=prs.Double("MaxG");
	prf.limit.minG=prs.Double("MinG");
	prf.limit.maxPressure=prs.Double("MaxPressure");
	prf.limit.maxVel=prs.Double("MaxGroundVel");
	atlist.push_back(new VOBJ(hVes, prf.limit));
	sprintf(prf.name,"%s",cfg);
	return prf;
}
void DMMANAGER::PreStep()
{
	OBJHANDLE hVes = NULL;
	VLIMIT limit;
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		hVes=oapiGetVesselByIndex(i);
		if (isRegistered(hVes))
			continue;
		if (!hasConfig(hVes))
			continue;
		if (ProfileExists(hVes, limit))
		{
			atlist.push_back(new VOBJ(hVes, limit));
		}else
		{
			profiles.push_back(Register(hVes));
		}
	}
	Prune();
	for (int i = 0; i < atlist.size(); i++)
	{
		atlist[i]->PreStep(hudop);
	}
}
void DMMANAGER::PostStep()
{
	Prune();
	for (int i = 0; i < atlist.size(); i++)
	{
		atlist[i]->PostStep();
	}
}
void DMMANAGER::Prune()
{
	if(atlist.empty())
		return;
	std::vector<VOBJ*>::iterator it = atlist.begin() + 1;
	VOBJ * vb;
	for (int i = 0; i < atlist.size(); i++)
	{
		it = atlist.begin() + i;
		vb = atlist[i];

		if (!oapiIsVessel(vb->hook))
		{
			delete vb;
			atlist.erase(it);
		}
	}
}
bool DMMANAGER::ProfileExists(OBJHANDLE hVes, VLIMIT &vl)
{
	char name[255];
	if (isSpacecraft(hVes))
	{
		sprintf(name,"%s%s.cfg",CFGPATH, VESSEL(hVes).GetName());
	}
	else
	{
		sprintf(name,"%s%s.cfg",CFGPATH, VESSEL(hVes).GetClassName());
	}
	for (int i = 0; i < profiles.size(); i++)
	{
		if (!strcmp(profiles[i].name,name)){
			vl=profiles[i].limit;
			return true;
		}
	}
	return false;
}
void DMMANAGER::LoadHUDOptions()
{
	PARSER gpr("Modules/Plugin/Damage.cfg");
	gpr.read();
	hudop.color=gpr.Vector3("HUDCOLOR");
	hudop.size=gpr.Double("TextSize");
	hudop.xofs=gpr.Double("X_OFFSET");
	hudop.yofs=gpr.Double("Y_OFFSET");
	hudop.startx=gpr.Double("StartX");
	hudop.starty=gpr.Double("StartY");
}