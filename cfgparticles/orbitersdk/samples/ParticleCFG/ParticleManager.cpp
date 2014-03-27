#include "ParticleManager.h"
#include <orbitersdk.h>
#include "Parser.h"

char attLabel[15][64] = 
{
	"main",
	"retro",
	"hover",
	"pitchup",
	"pitchdown",
	"yawleft",
	"yawright",
	"bankleft",
	"bankright",
	"right",
	"left",
	"up",
	"down",
	"forward",
	"back"
};
ParticleManager::~ParticleManager()
{
	for (DWORD i = 0; i < atList.size(); i++)
	{
		for (DWORD j = 0; j < atList[i]->thList.size(); j++)
		{
			if (atList[i]->thList[j])
				delete atList[i]->thList[j];
		}
		delete atList[i];
	}
}
bool ParticleManager::isRegistered(OBJHANDLE hObj)
{
	for (DWORD i = 0; i < atList.size(); i++)
	{
		if (!oapiIsVessel(atList[i]->hook))
			continue;
		if (atList[i]->hook==hObj)
			return true;
	}
	return false;
}
bool ParticleManager::hasConfig(OBJHANDLE hObj, char * cfg)
{
	VESSEL * v = oapiGetVesselInterface(hObj);
	if (v->GetClassName()==NULL)
		sprintf(cfg, "%s%s.cfg", CFGPATH, v->GetName());
	else 
		sprintf(cfg, "%s%s.cfg", CFGPATH, v->GetClassName());
	if (fopen(cfg, "r"))
		return true;
	return false;
}
bool ParticleManager::hasEngines(OBJHANDLE hObj)
{
	VESSEL * v = oapiGetVesselInterface(hObj);
	int count = 0;
	for (int i = 0; i < 14; i++)
		count+=v->GetGroupThrusterCount((THGROUP_TYPE)i);
	return (count>0);
}
void ParticleManager::TimeStep()
{
	OBJHANDLE hObj = NULL;
	DWORD inx = oapiGetVesselCount();
	char cfg[255];
	for (DWORD i = 0; i < inx; i++)
	{
		hObj = oapiGetVesselByIndex(i);
		if (!oapiIsVessel(hObj)){
			continue;
		}
		if (isRegistered(hObj)){
			continue;
		}
		if (!hasEngines(hObj)){
			continue;
		}
		if (!hasConfig(hObj, cfg)){
			continue;
		}
		apply(hObj, cfg);
	}
	for (DWORD i = 0; i < atList.size(); i++)
	{
		atList[i]->TimeStep();
	}
}
void ParticleManager::apply(OBJHANDLE hObj, char * cfg)
{
	VESSEL * v = oapiGetVesselInterface(hObj);
	PARSER gpr(cfg);
	gpr.read();

	int pCount = gpr.Int("streamcount");
	std::vector<ParticleProfile*> profiles;
	ParticleProfile *profile=NULL;
	char cbuf[255];
	char buffer[255];
	for (int i = 0; i < pCount; i++)
	{
		profile = new ParticleProfile();
		sprintf(cbuf, "stream_%d_amax", i+1);
		profile->spec.amax=gpr.Double(cbuf);
		sprintf(cbuf, "stream_%d_amin", i+1);
		profile->spec.amin=gpr.Double(cbuf);
		sprintf(cbuf, "stream_%d_slowdown", i+1);
		profile->spec.atmslowdown = gpr.Double(cbuf);
		sprintf(cbuf, "stream_%d_atmsmap", i+1);
		gpr.String(cbuf, buffer);
		if (!strcasecmp(buffer, "flat"))
			profile->spec.atmsmap=PARTICLESTREAMSPEC::ATM_FLAT;
		else if (!strcasecmp(buffer, "plin"))
			profile->spec.atmsmap=PARTICLESTREAMSPEC::ATM_PLIN;
		else
			profile->spec.atmsmap=PARTICLESTREAMSPEC::ATM_PLOG;
		sprintf(cbuf, "stream_%d_growthrate", i+1);
		profile->spec.growthrate=gpr.Double(cbuf);
		sprintf(cbuf, "stream_%d_levelmap", i+1);
		gpr.String(cbuf, buffer);
		if (!strcasecmp(buffer, "flat"))
			profile->spec.levelmap=PARTICLESTREAMSPEC::LVL_FLAT;
		else if (!strcasecmp(buffer, "lin"))
			profile->spec.levelmap=PARTICLESTREAMSPEC::LVL_LIN;
		else if (!strcasecmp(buffer, "sqrt"))
			profile->spec.levelmap=PARTICLESTREAMSPEC::LVL_SQRT;
		else if (!strcasecmp(buffer, "plin"))
			profile->spec.levelmap=PARTICLESTREAMSPEC::LVL_PLIN;
		else 
			profile->spec.levelmap=PARTICLESTREAMSPEC::LVL_PSQRT;
		sprintf(cbuf, "stream_%d_lifetime", i+1);
		profile->spec.lifetime=gpr.Double(cbuf);
		sprintf(cbuf, "stream_%d_lmax", i+1);
		profile->spec.lmax=gpr.Double(cbuf);
		sprintf(cbuf, "Stream_%d_lmin", i+1);
		profile->spec.lmin=gpr.Double(cbuf);
		sprintf(cbuf, "stream_%d_ltype", i+1);
		gpr.String(cbuf, buffer);
		if (!strcasecmp(buffer, "emissive"))
			profile->spec.ltype=PARTICLESTREAMSPEC::EMISSIVE;
		else
			profile->spec.ltype=PARTICLESTREAMSPEC::DIFFUSE;
		sprintf(cbuf, "stream_%d_srcrate", i+1);
		profile->spec.srcrate=gpr.Double(cbuf);
		sprintf(cbuf, "stream_%d_srcsize", i+1);
		profile->spec.srcsize=gpr.Double(cbuf);
		sprintf(cbuf, "stream_%d_srcspread", i+1);
		profile->spec.srcspread=gpr.Double(cbuf);
		sprintf(cbuf, "stream_%d_v0", i+1);
		profile->spec.v0=gpr.Double(cbuf);
		sprintf(cbuf, "stream_%d_tex", i+1);
		gpr.String(cbuf, buffer);
		if (!strcasecmp(buffer, "none"))
			profile->spec.tex=NULL;
		else
			profile->spec.tex=oapiRegisterExhaustTexture(buffer);
		profiles.push_back(profile);
	}
	VECTOR3 pos, dir;
	DWORD count = 0;
	std::vector<THRUSTER*> thList;
	THRUSTER * thruster = NULL;
	PROPELLANT_HANDLE hpr = v->CreatePropellantResource(1);
	DWORD n = 0;
	for (int i = 0; i < 15; i++)
	{
		sprintf(cbuf, "%s_count", attLabel[i]);
		count = gpr.Int(cbuf);
		if (count<=0)
			continue;
		sprintf(cbuf, "%s_exhaust", attLabel[i]);
		n = gpr.Int(cbuf)-1;
		if (n<profiles.size())
		{
			for (DWORD j = 0; j < count; j++)
			{
				thruster = new THRUSTER();
				sprintf(cbuf, "%s_%d_pos", attLabel[i], j+1);
				pos = gpr.Vector3(cbuf);
				sprintf(cbuf, "%s_%d_dir", attLabel[i], j+1);
				dir = gpr.Vector3(cbuf);
				thruster->th=v->CreateThruster(pos, dir, 0e1, hpr, 500e50);
				thruster->type=(THGROUP_TYPE)i;
				v->AddExhaustStream(thruster->th, &profiles[n]->spec);
				thList.push_back(thruster);
			}
		}
	}
	atList.push_back(new VOBJ(v->GetHandle(), thList));
	for (DWORD i = 0; i < profiles.size(); i++)
		if (profiles[i])
			delete profiles[i];
}
void ParticleManager::Prune()
{
    if (atList.empty())
        return;
    std::vector<VOBJ*>::iterator it = atList.begin() + 1;
    for (DWORD i = 0; i < atList.size(); i++)
    {
        it = atList.begin() + i;
        if (!oapiIsVessel(atList[i]->hook))
        {
			delete atList[i];
			atList.erase(it);
        }
    }
}