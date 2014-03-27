#include "UniversalParachuteManager.h"
#include "Parser.h"
#define  CFGPATH "Config/UNVParachute/"

bool UniversalParachuteManager::hasConfig(OBJHANDLE h)
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
bool UniversalParachuteManager::isSpacecraft(OBJHANDLE h)
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
bool UniversalParachuteManager::isRegistered(OBJHANDLE hVes)
{
	for (int i = 0; i < atList.size(); i++)
	{
		if (!atList[i])
			continue;
		if (atList[i]->hook==hVes)
			return true;
	}
	return false;
}
bool UniversalParachuteManager::FocusIsRegistered()
{
	OBJHANDLE hObj = oapiGetFocusInterface()->GetHandle();
	return isRegistered(hObj);
}
VOBJ* UniversalParachuteManager::GetFocus()
{
	if (!FocusIsRegistered())
		return NULL;
	OBJHANDLE hFocus = oapiGetFocusInterface()->GetHandle();
	for (int i = 0; i < atList.size(); i++)
	{
		if (!atList[i])
			continue;
		if (atList[i]->hook==hFocus)
			return atList[i];
	}
	return NULL;
}
VOBJ* UniversalParachuteManager::Register(OBJHANDLE hVes)
{
	char     cfg[255], cbuf[255];
	VESSEL * v = oapiGetVesselInterface(hVes);
	if (isSpacecraft(hVes))
		sprintf(cfg, "%s%s.cfg", CFGPATH, v->GetName());
	else
		sprintf(cfg, "%s%s.cfg", CFGPATH, v->GetClassName());
	PARSER gpr(cfg);
	gpr.read();

	int pCount = gpr.Int("ChuteCount");
	PPARAM param;
	VOBJ * vbj = new VOBJ(hVes);
	char type[20];
	for (int i = 0; i < pCount; i++)
	{
		sprintf(cbuf, "Chute_%d_Mesh", i+1);
		sprintf(param.mesh, "%s", gpr.String(cbuf, param.mesh));
		sprintf(cbuf, "Chute_%d_OFS", i+1);
		param.ofs=gpr.Vector3(cbuf);
		sprintf(cbuf, "Chute_%d_DragCoef", i+1);
		param.dcoef=gpr.Double(cbuf);
		sprintf(cbuf, "Chute_%d_Area", i+1);
		param.area=gpr.Double(cbuf);
		sprintf(cbuf, "Chute_%d_Type", i+1);
		sprintf(type, "%s", gpr.String(cbuf, type));
		param.type=CTYPE_PARACHUTE;
		if (!strcmp(type, "drag") || !strcmp(type, "DRAG"))
			param.type=CTYPE_DRAG;
		vbj->chutes.push_back(new PARACHUTE(hVes, param.type, param.dcoef, param.area, param.mesh, PACKED, param.ofs));
	}
	atList.push_back(vbj);
	return vbj;
}
void UniversalParachuteManager::TimeStep()
{
	DWORD inx = oapiGetVesselCount();
	OBJHANDLE hVes = NULL;
	for (int i = 0; i < inx; i++)
	{
		hVes = oapiGetVesselByIndex(i);
		if (isRegistered(hVes))
			continue;
		if (!hasConfig(hVes))
			continue;
		Register(hVes);
	}
	Prune();
	for (int i = 0; i < atList.size(); i++)
	{
		atList[i]->TimeStep();
	}
}
void UniversalParachuteManager::Prune()
{
	if (atList.empty())
        return;
    std::vector<VOBJ*>::iterator it = atList.begin() + 1;
    for (int i = 0; i < atList.size(); i++)
    {
        it = atList.begin() + i;
        if (!oapiIsVessel(atList[i]->hook))
        {
            delete atList[i];
            atList.erase(it);
        }
    } 
}