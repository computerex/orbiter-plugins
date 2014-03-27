#define _CRT_SECURE_NO_DEPRECATE
#if defined(_WIN32) || defined(_WIN64)
  #define snprintf _snprintf
  #define vsnprintf _vsnprintf
  #define strcasecmp _stricmp
  #define strncasecmp _strnicmp
#endif

#include "UMMUFA.h"
#include "Parser.h"
#include "Logger.h"

#define CFGFILE "Config/UMMUFA/%s.cfg"
#define IGNORELIST "Modules/UMMUFAIGNORE.cfg"

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
		if (isUmmuCompatible(handle))
			continue;
		if (isAttended(handle))
			continue;
		if (isUmmu(handle))
			continue;
		if (!hasDocks(handle))
			continue;
		atlist.push_back(new VOBJ(cmd, handle));
	}
	VESSEL * vessel = NULL;
	for (int i = 0; i < atlist.size(); i++)
	{
		atlist[i]->TimeStep();
	}
}
bool UMMUFA::init()
{
	std::ifstream file(IGNORELIST);
	std::string line;
	if (file.is_open())
	{
		while(!file.eof())
		{
			getline(file, line);
			size_t CMT_Check = line.find(";");
            if (CMT_Check!=std::string::npos)
            {
                line.erase(CMT_Check);
                if (CMT_Check == 0)
                    continue;
            }
			if (line.length() < 1)
				continue;
			IgnoreList.push_back(line);
		}
		file.close();
	}
	return Read();
}
bool UMMUFA::Read()
{
	if (fopen(cfg,"r")==0){
		return false;
	}
	PARSER parser(cfg);
	parser.read();

	int vcount   = parser.Int("VesselCount");
	OBJHANDLE h  = NULL;

	CREWMEMBERDEF  cd;
	CrewMemberDef  cmd;
	char           key[255], name[255];
	int            cnt=0;
	for (int i = 0; i < vcount; i++)
	{
		sprintf(key,"Vessel_%d_Name",i+1);
		sprintf(name,"%s",parser.String(key,name));
		h = oapiGetObjectByName(name);
		if (!oapiIsVessel(h))
			continue;
		if (isUmmuCompatible(h))
			continue;
		sprintf(key,"Vessel_%d_CrewCount",i+1);
		cnt = parser.Int(key);
		cmd.clear();
		for (int j = 0; j < cnt; j++)
		{
			sprintf(key,"Vessel_%d_mmu_%d_Name",i+1,j+1);
			sprintf(cd.Name,"%s",parser.String(key,cd.Name));
			sprintf(key,"Vessel_%d_mmu_%d_Age",i+1,j+1);
			cd.Age=parser.Int(key);
			sprintf(key,"Vessel_%d_mmu_%d_Pulse",i+1,j+1);
			cd.Pulse=parser.Int(key);
			sprintf(key,"Vessel_%d_mmu_%d_weight",i+1,j+1);
			cd.Weight=parser.Double(key);
			sprintf(key,"Vessel_%d_mmu_%d_ID",i+1,j+1);
			sprintf(cd.MiscId,"%s",parser.String(key,cd.MiscId));
			cmd.push_back(cd);
		}
		atlist.push_back(new VOBJ(cmd,h));
	}
	return true;
}
void UMMUFA::save(void)
{
	if (SpecialSave==TRUE)
	{
		SpecialSave=FALSE;
		save(SpecialSaveBuffer);
		return;
	}
	save(cfg);
}
void UMMUFA::save(char * file)
{
	LOGGER logger(file,"write");
	char slog[255], sep[150];
	sprintf(sep,";----------------------------------------");
	
	sprintf(slog,"VesselCount = %d",atlist.size());
	logger.write(slog);
	logger.write(sep);
	logger.write(sep);
	VOBJ * v = NULL;
	char name[255];
	for (int i = 0; i < atlist.size(); i++)
	{
		v = atlist[i];
		sprintf(slog,";%s GENERAL PARAMTERS",VESSEL(v->hook).GetName());
		logger.write(sep);
		logger.write(slog);
		logger.write(sep);
		sprintf(slog,"Vessel_%d_Name = %s",i+1,VESSEL(v->hook).GetName());
		logger.write(slog);
		sprintf(slog,"Vessel_%d_CrewCount = %d",i+1,v->crew.GetCrewTotalNumber());
		logger.write(slog);
		sprintf(slog,";%s MMU PARAMTERS",VESSEL(v->hook).GetName());
		logger.write(sep);
		logger.write(slog);
		logger.write(sep);
		for (int j = 0; j < v->crew.GetCrewTotalNumber(); j++)
		{
			strcpy(name,v->crew.GetCrewNameBySlotNumber(j));
			sprintf(slog,";Crew Member: %s",name);
			logger.write(slog);
			sprintf(slog,"Vessel_%d_mmu_%d_Name = %s",i+1,j+1,name);
			logger.write(slog);
			sprintf(slog,"Vessel_%d_mmu_%d_Age = %d",i+1,j+1,v->crew.GetCrewAgeByName(name));
			logger.write(slog);
			sprintf(slog,"Vessel_%d_mmu_%d_Pulse = %d",i+1,j+1,v->crew.GetCrewPulseByName(name));
			logger.write(slog);
			sprintf(slog,"Vessel_%d_mmu_%d_weight = %d",i+1,j+1,v->crew.GetCrewWeightByName(name));
			logger.write(slog);
			sprintf(slog,"Vessel_%d_mmu_%d_ID = %s",i+1,j+1,v->crew.GetCrewMiscIdByName(name));
			logger.write(slog);
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
	if (!oapiIsVessel(h))
		return false;
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
	for (int i = 0; i < IgnoreList.size(); i++)
	{
		if (!strcasecmp(IgnoreList[i].c_str(), v->GetClassName()))
			return true;
	}
	return false;
}