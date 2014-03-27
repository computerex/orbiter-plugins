#include "VOBJ.h"
#include <stdio.h>
#include "Parser.h"


bool isSpacecraft(OBJHANDLE h)
{
	if (!oapiIsVessel(h))
		return false;
	char cbuf[255];
	VESSEL * v  = oapiGetVesselInterface(h);

	if (v->GetClassName() == NULL)
		return true;
	if (!strnicmp(v->GetClassName(),"Spacecraft",10))
		return true;
	return false;
}
char * GetCFGName(OBJHANDLE h, char * buffer)
{
	if (!oapiIsVessel(h))
		return NULL;
	VESSEL * v = oapiGetVesselInterface(h);
	if (isSpacecraft(h))
		sprintf(buffer,"%s%s.cfg",CFGPATH,v->GetName());
	else
		sprintf(buffer,"%s%s.cfg",CFGPATH,v->GetClassName());
	return buffer;
}
bool hasConfig(OBJHANDLE h)
{
	if (!oapiIsVessel(h))
		return false;
	char cbuf[255];
	if (fopen(GetCFGName(h, cbuf),"r"))
		return true;
	return false;
}
bool VOBJ::HasSavedState(OBJHANDLE hv, PARSER gpr)
{
	if (!oapiIsVessel(hv))
		return false;
	VESSEL * v = oapiGetVesselInterface(hv);
	int vcount = gpr.Int("VesselCount");
	char cbuf[255], vname[255];
	for (int i = 0; i < vcount; i++)
	{
		sprintf(cbuf, "Vessel_%d_Name", i+1);
		gpr.String(cbuf, vname);
		if (!strcmp(vname, v->GetName()))
			return true;
	}
	return false;
}
bool VOBJ::init()
{
	if (!hasConfig(hook))
		return true;
	char cfg[255];
	char cbuf[255];
	GetCFGName(hook, cfg);
	PARSER parser(cfg);
	parser.read();

	ANIMATION anim;
	char name[255];
	int aCount = parser.Int("AnimationCount");
	for (int i = 0; i < aCount; i++)
	{
		anim.id=i;
		sprintf(cbuf,"Anim_%d_Init",i+1);
		anim.state=parser.Double(cbuf);
		sprintf(cbuf,"Anim_%d_Name",i+1);
		parser.String(cbuf, name);
		if (strlen(name) > 0)
			strcpy(anim.name,name);
		else
			strcpy(anim.name," ");
		av.push_back(anim);
	}
	return true;
}
bool VOBJ::init(PARSER gpr)
{
	if (!init())
		return false;
	if (!oapiIsVessel(hook))
		return false;
	if (!HasSavedState(hook, gpr))
		return true;
	if (!hasConfig(hook))
		return true;
	VESSEL * v = oapiGetVesselInterface(hook);
	char cbuf[255];
	for (int i = 0; i < av.size(); i++)
	{
		sprintf(cbuf,"%s_Animation_%d_State", v->GetName(), i+1);
		av[i].state=gpr.Double(cbuf);
	}
	return true;
}