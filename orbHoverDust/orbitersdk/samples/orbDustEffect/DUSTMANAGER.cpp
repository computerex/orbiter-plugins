#include <orbitersdk.h>
#include "VOBJ.h"
#include "DUSTMANAGER.h"
#include <vector>
#include <math.h>
#include "FileReader.h"

#define TIME_OFS 1 
static PARTICLESTREAMSPEC exhaust_hover;
double alt_ofs = 10;
FILEREADER g_FR;

void read_data(void)
{
	char ini[255];
	char cbuf[255];
	sprintf(ini,"./Modules/Plugin/orbHoverDust.ini");
	g_FR.LoadDouble(ini,"SETTINGS","srcsize",exhaust_hover.srcsize);
	g_FR.LoadDouble(ini,"SETTINGS","srcrate",exhaust_hover.srcrate);
	g_FR.LoadDouble(ini,"SETTINGS","v0",exhaust_hover.v0);
	g_FR.LoadDouble(ini,"SETTINGS","srcspread",exhaust_hover.srcspread);
	g_FR.LoadDouble(ini,"SETTINGS","lifetime",exhaust_hover.lifetime);
	g_FR.LoadDouble(ini,"SETTINGS","growthrate",exhaust_hover.growthrate);
	g_FR.LoadDouble(ini,"SETTINGS","atmslowdown",exhaust_hover.atmslowdown);
	g_FR.LoadDouble(ini,"SETTINGS","lmin",exhaust_hover.lmin);
	g_FR.LoadDouble(ini,"SETTINGS","lmax",exhaust_hover.lmax);
	g_FR.LoadDouble(ini,"SETTINGS","amin",exhaust_hover.amin);
	g_FR.LoadDouble(ini,"SETTINGS","amax",exhaust_hover.amax);
	g_FR.LoadString(ini,"SETTINGS","LTYPE",cbuf);
	if (!strcmp(cbuf,"EMISSIVE"))
		exhaust_hover.ltype=PARTICLESTREAMSPEC::EMISSIVE;
	else if (!strcmp(cbuf,"DIFFUSE"))
		exhaust_hover.ltype=PARTICLESTREAMSPEC::DIFFUSE;
	else
		exhaust_hover.ltype=PARTICLESTREAMSPEC::DIFFUSE;
	g_FR.LoadString(ini,"SETTINGS","LEVELMAP",cbuf);
	if (!strcmp(cbuf,"LVL_FLAT"))
		exhaust_hover.levelmap=PARTICLESTREAMSPEC::LVL_FLAT;
	else if(!strcmp(cbuf,"LVL_LIN"))
		exhaust_hover.levelmap=PARTICLESTREAMSPEC::LVL_LIN;
	else if(!strcmp(cbuf,"LVL_SQRT"))
		exhaust_hover.levelmap=PARTICLESTREAMSPEC::LVL_SQRT;
	else if(!strcmp(cbuf,"LVL_PLIN"))
		exhaust_hover.levelmap=PARTICLESTREAMSPEC::LVL_PLIN;
	else if (!strcmp(cbuf,"LVL_PSQRT"))
		exhaust_hover.levelmap=PARTICLESTREAMSPEC::LVL_PSQRT;
	else
		exhaust_hover.levelmap=PARTICLESTREAMSPEC::LVL_SQRT;
	g_FR.LoadString(ini,"SETTINGS","ATMSMAP",cbuf);
	if (!strcmp(cbuf,"ATM_FLAT"))
		exhaust_hover.atmsmap=PARTICLESTREAMSPEC::ATM_FLAT;
	else if (!strcmp(cbuf,"ATM_PLIN"))
		exhaust_hover.atmsmap=PARTICLESTREAMSPEC::ATM_PLIN;
	else if (!strcmp(cbuf,"ATM_PLOG"))
		exhaust_hover.atmsmap=PARTICLESTREAMSPEC::ATM_PLOG;
	else
		exhaust_hover.atmsmap=PARTICLESTREAMSPEC::ATM_PLOG;
	g_FR.LoadString(ini,"SETTINGS","tex",cbuf);
	if (strcmp(cbuf,"none"))
	{
		SURFHANDLE texture = oapiRegisterExhaustTexture(cbuf);
		exhaust_hover.tex=texture;
	}
	g_FR.LoadDouble(ini,"SETTINGS","alt_ofs",alt_ofs);
}
int DUSTMANAGER::hasHovers(OBJHANDLE hVes)
{
	VESSEL * v = oapiGetVesselInterface(hVes);
	THGROUP_HANDLE hGroup = v->GetThrusterGroupHandle(THGROUP_HOVER);

	if (hGroup != NULL)
	{
		return v->GetGroupThrusterCount(THGROUP_HOVER);
	}
	return 0;
}
VECTOR3 dist2grnd(OBJHANDLE hves, THRUSTER_HANDLE th)
{
	VESSEL * vo = oapiGetVesselInterface(hves);
	VECTOR3 ref;
	vo->GetThrusterRef(th,ref);
	double ofs = (vo->GetAltitude()-fabs(ref.y))/cos(fabs(vo->GetBank()));
	if (ofs > 0)
		ofs *= -1;
	ref.y=ofs;
	return ref;
}
bool DUSTMANAGER::isAttended(VESSEL *v)
{
	if (!v)
		return false;
	for (int i = 0; i < atList.size(); i++)
	{
		if (!atList[i])
			continue;
		if (!oapiIsVessel(atList[i]->hook))
			continue;
		if (atList[i]->hook == v->GetHandle())
			return true;
	}
	return false;
}
void DUSTMANAGER::PreStep(void)
{
	static bool init = false;
	if (!init)
	{
		init = true; 
		read_data();
	}
	OBJHANDLE  hVes = 0;
	VESSEL  *  ves  = 0;
	VOBJ    *  v    = 0;
	VECTOR3    ref;
	double     simt = oapiGetSimTime();
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		hVes = oapiGetVesselByIndex(i);
		if (!oapiIsVessel(hVes))
			continue;
		ves = oapiGetVesselInterface(hVes);
		if (isAttended(ves))
			continue;
		if (!hasHovers(hVes))
			continue;
		VOBJ * vbj = new VOBJ(hVes);
		atList.push_back(vbj);
	}
	for (int i = 0; i < atList.size(); i++)
	{
		if (!atList[i])
			continue;
		if (!oapiIsVessel(atList[i]->hook))
			continue;
		v    = atList[i];
		ves  = oapiGetVesselInterface(atList[i]->hook);
		hVes = atList[i]->hook;
		for (int j = 0; j < v->pHandle.size(); j++)
		{
			if (simt > v->LP_Time[j])
			{
				if (v->pHandle[j])
					ves->DelExhaustStream(v->pHandle[j]);
				v->pHandle[j]=0;
			}
			ref = dist2grnd(hVes, ves->GetGroupThruster(THGROUP_HOVER, j));
			if (-ref.y>alt_ofs)
				continue;
			if (!v->pHandle[j])
			{
				v->pHandle[j]=ves->AddExhaustStream(ves->GetGroupThruster(THGROUP_HOVER, j),ref,&exhaust_hover);
			    v->LP_Time[j]=simt+TIME_OFS;
			}
		}
	}
}
