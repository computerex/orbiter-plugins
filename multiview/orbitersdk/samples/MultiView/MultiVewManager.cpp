#include "MultiViewManager.h"
#include "Parser.h"
#include "Logger.h"

#define CFGPATH "Config/MultiView/"

bool MULTIVIEWMANAGER::isSpacecraft(OBJHANDLE h)
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
bool MULTIVIEWMANAGER::hasConfig(OBJHANDLE h)
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
bool MULTIVIEWMANAGER::isRegistered(char *typ)
{
	for (int i = 0; i < profiles.size(); i++)
	{
		if (!strcmp(profiles[i].type,typ))
			return true;
	}
	return false;
}
CVTYPE MULTIVIEWMANAGER::Register(char *typ)
{
	char cfg[255];
	LOGGER log("MultiView.log","log");

	sprintf(cfg,"%s%s.cfg",CFGPATH,typ);
	sprintf(log.line(),"%s",cfg);
	log.write();
	PARSER parser(cfg);
	parser.read();

	int viewcount = parser.Int("ViewCount");
	
	std::vector<CAM> camVector;
	CAM              cam;
	
	char cbuf[255];
	for (int i = 0; i < viewcount; i++)
	{
		sprintf(cbuf,"CAM_%d_Pos",i+1);
		cam.pos=parser.Vector3(cbuf);
		sprintf(cbuf,"CAM_%d_dir",i+1);
		cam.dir=parser.Vector3(cbuf);
		camVector.push_back(cam);
	}
	CVTYPE cv;
	strcpy(cv.type,typ);
	cv.cameras=camVector;
	return cv;
}
bool MULTIVIEWMANAGER::FocusIsRegistered()
{
	VESSEL * v = oapiGetFocusInterface();
	if (isSpacecraft(v->GetHandle()))
		return isRegistered(v->GetName());
	else
		return isRegistered(v->GetClassName());
}
CVTYPE MULTIVIEWMANAGER::GetFocus()
{
	char typ[255];
	VESSEL * v = oapiGetFocusInterface();
	
	if (isSpacecraft(v->GetHandle()))
		strcpy(typ,v->GetName());
	else
		strcpy(typ,v->GetClassName());

	for (int i = 0; i < profiles.size(); i++)
	{
		if (!strcmp(typ,profiles[i].type))
			return profiles[i];
	}
	return CVTYPE();
}
void MULTIVIEWMANAGER::TimeStep(void)
{
	OBJHANDLE hVes = NULL;
	VESSEL * v = NULL;
	char typ[255];
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		hVes = oapiGetVesselByIndex(i);
		if (!oapiIsVessel(hVes))
			continue;
		v = oapiGetVesselInterface(hVes);
		if (!hasConfig(hVes))
			continue;
		if (isSpacecraft(hVes))
			strcpy(typ,v->GetName());
		else
			strcpy(typ,v->GetClassName());
		if (isRegistered(typ))
			continue;
		profiles.push_back(Register(typ));
	}
		
}