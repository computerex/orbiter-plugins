#include "RMANAGER.h"
#include "FileReader.h"

FILEREADER g_FR;

double RMANAGER::calcFlux(VESSEL *vessel)
{
	if (!vessel)
		return 0;
	double p, q, v;
	p = vessel->GetAtmDensity();
	v = vessel->GetAirspeed();
	v = v*v*v;
	q = 0.5 * p * v;
	return q;
}
bool RMANAGER::isFocus(VESSEL * v)
{
	if (v->GetHandle() == oapiGetFocusInterface()->GetHandle())
		return true;
	return false;
}
void RMANAGER::TimeStep()
{
	VESSEL * v = NULL;
	OBJHANDLE hVes = NULL;

	char ini[255];
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		hVes = oapiGetVesselByIndex(i);
		if (!oapiIsVessel(hVes))
			continue;
		v    = oapiGetVesselInterface(hVes);
		if (isAttended(v))
			continue;
		if (!hasConfig(v))
			continue;
		sprintf(ini,"./Config/orbReentryStream/%s.ini",INI(v));
		atList.push_back(ReadAndAssign(ini,v));
	}
	VOBJ * vbj = NULL;
	for (int i = 0; i < atList.size(); i++)
	{
		if (!atList[i])
			continue;
		if (!oapiIsVessel(atList[i]->hook))
			continue;
		vbj  = atList[i];
		v    = oapiGetVesselInterface(vbj->hook);
		hVes = v->GetHandle();
		if (calcFlux(v) > vbj->flux)
		{
			if (!vbj->th)
				CreateResourceAndStreams(vbj);
			v->SetThrusterLevel(vbj->th,1);
		}else if (vbj->th)
			v->SetThrusterLevel(vbj->th,0);
	}
}

VOBJ * RMANAGER::ReadAndAssign(char * ini, VESSEL * v)
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
	g_FR.LoadString(ini,"SETTINGS","heatFlux",cbuf);
	if (!strcmp("default",cbuf))
		vbj->flux=DEFAULT_FLUX;
	else
		vbj->flux=atof(cbuf);
	g_FR.LoadVector(ini,"SETTINGS","dir",vbj->dir);
	vbj->hook=v->GetHandle();
	vbj->streams=visuals;
	CreateResourceAndStreams(vbj);
	return vbj;
}
void RMANAGER::CreateResourceAndStreams(VOBJ * vbj)
{
	if (!oapiIsVessel(vbj->hook))
		return;
	VESSEL * vessel = oapiGetVesselInterface(vbj->hook);
	PROPELLANT_HANDLE hpr = vessel->CreatePropellantResource(1);
	vbj->th = vessel->CreateThruster(_V(0,0,0),vbj->dir,1e0,hpr,500000e50);
	for (int i = 0; i < vbj->streams.size(); i++)
	{
		vessel->AddExhaustStream(vbj->th,vbj->streams[i]->pos,vbj->streams[i]->stream);
	}
}
bool RMANAGER::isSpacecraft(VESSEL * v)
{
	if (!v)
		return false;
	if (v->GetClassName() == NULL)
		return true;
	if (!strncmp("Spacecraft",v->GetClassName(),10))
		return true;
	return false;
}
bool RMANAGER::hasConfig(VESSEL * v)
{
	if (!v)
		return false;
	char cbuf[255];
	if (isSpacecraft(v))
		sprintf(cbuf,"./Config/orbReentryStream/%s.ini",v->GetName());
	else
		sprintf(cbuf,"./Config/orbReentryStream/%s.ini",v->GetClassName());
	if (fopen(cbuf,"r"))
		return true;
	else
		return false;
	return false;
}
bool RMANAGER::isAttended(VESSEL * v)
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
		vessel = oapiGetVesselInterface(atList[i]->hook);
		if (v->GetHandle() == vessel->GetHandle())
			return true;
	}
	return false;
}
char * RMANAGER::INI(VESSEL * v)
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