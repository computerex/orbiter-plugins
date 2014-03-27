#define _CRT_SECURE_NO_DEPRECATE

#include "UMMUFA.h"
#include "Parser.h"
#include "Logger.h"
#include <stdarg.h>



void UMMUFA::TimeStep()
{
	Prune();
	OBJHANDLE handle = NULL;
	char cbuf[255];
	CrewMemberDef cmd;
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		handle = oapiGetVesselByIndex(i);
		if (!oapiIsVessel(handle))
			continue;
		if (ischecked(handle))
			continue;
		if (isAttended(handle))
			continue;
		if (isUmmuCompatible(handle)){
			logwrite("UMMUFA: %s added to blacklist - Native UMMU compatible", VESSEL(handle).GetName());
			bklist.push_back(handle);
			continue;
		}
		if (isUmmu(handle)){
			logwrite("UMMUFA: %s added to blacklist - A UMMU vessel", VESSEL(handle).GetName());
			bklist.push_back(handle);
			continue;
		}
		if (!hasDocks(handle)){
			logwrite("UMMUFA: %s added to blacklist - Has no docking ports", VESSEL(handle).GetName());
			bklist.push_back(handle);
			continue;
		}
		atlist.push_back(new VOBJ(cmd, handle));
	}
	VESSEL * vessel = NULL;
	for (int i = 0; i < atlist.size(); i++)
	{
		atlist[i]->TimeStep();
	}
}
bool UMMUFA::init(FILEHANDLE scn)
{
	return Read(scn);
}
std::string getitem(std::vector<std::string> lines, const char * fmt, ...)
{
	char cbuf[255];   
	va_list arg;
	va_start(arg,fmt);
	 vsprintf(cbuf,fmt,arg);
	va_end(arg);
	unsigned int len = strlen(cbuf);
	for ( unsigned int i = 0; i < lines.size(); i++ )
	{
		if ( !strnicmp(lines[i].c_str(), cbuf, len) )
			return lines[i].substr(len+3).c_str();
	}
	return "";
}
bool UMMUFA::Read(FILEHANDLE scn)
{
	char * line;
	std::vector<std::string> lines;
	while (oapiReadScenario_nextline(scn, line))
	{
		lines.push_back(line);
	}
	unsigned int vcount = atoi(getitem(lines, "VesselCount").c_str()), cnt=0;
	OBJHANDLE h  = NULL;
	CREWMEMBERDEF  cd;
	CrewMemberDef  cmd;
	char cbuf[256], name[256];
	for ( unsigned int i = 0; i < vcount; i++ )
	{
		strcpy(name, getitem(lines, "Vessel_%d_Name", i+1).c_str());
		h = oapiGetObjectByName(name);
		if (!oapiIsVessel(h))
			continue;
		if (isUmmuCompatible(h))
			continue;
		cnt=atoi(getitem(lines, "Vessel_%d_CrewCount", i+1).c_str());
		cmd.clear();
		for (int j = 0; j < cnt; j++)
		{
			strcpy(cd.Name,getitem(lines,      "Vessel_%d_mmu_%d_Name",    i+1, j+1).c_str());
			cd.Age    = atoi(getitem(lines,    "Vessel_%d_mmu_%d_Age",     i+1, j+1).c_str());
			cd.Pulse  = atoi(getitem(lines,    "Vessel_%d_mmu_%d_Pulse",   i+1, j+1).c_str());
			cd.Weight = atoi(getitem(lines,    "Vessel_%d_mmu_%d_weight",  i+1, j+1).c_str());
			strcpy(cd.MiscId,getitem(lines,    "Vessel_%d_mmu_%d_ID",      i+1, j+1).c_str());
			cmd.push_back(cd);
		}
		atlist.push_back(new VOBJ(cmd,h));
	}
	return true;
}
void UMMUFA::save(FILEHANDLE scn)
{
	char cbuf[256], name[255];
	oapiWriteItem_string(scn, ";MUST be ordered thus", " ");
	oapiWriteItem_int(scn, "VesselCount", atlist.size());
	VOBJ * v = NULL;
	for (int i = 0; i < atlist.size(); i++)
	{
		v = atlist[i];
		sprintf(cbuf,"Vessel_%d_Name",i+1);
		oapiWriteItem_string(scn, cbuf, VESSEL(v->hook).GetName());
		sprintf(cbuf,"Vessel_%d_CrewCount",i+1);
		oapiWriteItem_int(scn, cbuf, v->crew.GetCrewTotalNumber());
		for (int j = 0; j < v->crew.GetCrewTotalNumber(); j++)
		{
			strcpy(name,v->crew.GetCrewNameBySlotNumber(j));
			sprintf(cbuf,"Vessel_%d_mmu_%d_Name",i+1,j+1);
			oapiWriteItem_string(scn, cbuf, name);
			sprintf(cbuf,"Vessel_%d_mmu_%d_Age",i+1,j+1);
			oapiWriteItem_int(scn, cbuf, v->crew.GetCrewAgeByName(name));
			sprintf(cbuf,"Vessel_%d_mmu_%d_Pulse",i+1,j+1);
			oapiWriteItem_int(scn, cbuf, v->crew.GetCrewPulseByName(name));
			sprintf(cbuf,"Vessel_%d_mmu_%d_weight",i+1,j+1);
			oapiWriteItem_int(scn, cbuf, v->crew.GetCrewWeightByName(name));
			sprintf(cbuf,"Vessel_%d_mmu_%d_ID",i+1,j+1);
			oapiWriteItem_string(scn, cbuf, v->crew.GetCrewMiscIdByName(name));
		}
	}
}
void UMMUFA::Prune()
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
bool UMMUFA::isAttended(OBJHANDLE h)
{
	for (int i = 0; i < atlist.size(); i++)
	{
		if (h == atlist[i]->hook)
			return true;
	}
	return false;
}
int UMMUFA::hasDocks(OBJHANDLE h)
{
	VESSEL * v = oapiGetVesselInterface(h);
	return v->DockCount();
}
VOBJ* UMMUFA::GetFocus(void)
{
	VESSEL * v = oapiGetFocusInterface();
	for (int i = 0; i < atlist.size(); i++)
	{
		if (atlist[i]->hook==v->GetHandle())
			return atlist[i];
	}
	return NULL;
}
bool UMMUFA::isUmmu(OBJHANDLE h)
{
	VESSEL * v = oapiGetVesselInterface(h);
	if (v->GetClassName() == NULL)
		return false;
	return (strcmp("UMmu", v->GetClassName()) == 0 ? true : false);
}
bool UMMUFA::isUmmuCompatible(OBJHANDLE handle)
{
	VESSEL * v = oapiGetVesselInterface(handle);
	if (v->GetClassName() == NULL)
		return false;
	ATTACHMENTHANDLE hCheck = NULL;
	DWORD count = v->AttachmentCount(false);
	for (DWORD i = 0; i < count; i++)
	{
		hCheck = v->GetAttachmentHandle(false, i);
		if (hCheck==NULL)
			continue;
		if (!strcmp(v->GetAttachmentId(hCheck), "UMmu"))
			return true;
	}
	return false;
}

bool UMMUFA::ischecked(OBJHANDLE ves)
{
	for ( unsigned int i = 0; i < bklist.size(); i++ )
	{
		if ( ves == bklist[i] ) return true;
	}
	return false;
}