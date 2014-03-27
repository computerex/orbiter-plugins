#include "UMMUFA.h"
#include "Parser.h"
#include "Logger.h"

#define CFGFILE "Config/UMMUFA/%s.cfg"

void UMMUFA::TimeStep()
{
	Prune();
	OBJHANDLE handle = NULL;
	char cbuf[255];
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		handle = oapiGetVesselByIndex(i);
		if (isAttended(handle))
			continue;
		if (!hasConfig(handle))
			continue;
		sprintf(cbuf,"%s",CFG(handle,cbuf));
		ReadGeneric(cbuf,handle);
	}
	VESSEL * vessel = NULL;
	for (int i = 0; i < atlist.size(); i++)
	{
		atlist[i]->TimeStep();
		if (DisableWhenEmpty==TRUE)
		{
			vessel = oapiGetVesselInterface(atlist[i]->hook);
			DWORD modes  = vessel->GetADCtrlMode();
			if (atlist[i]->crew.GetCrewTotalNumber() == 0)
			{
				vessel->SetAttitudeMode(RCS_NONE);
				vessel->SetADCtrlMode(modes & 0x0);
				for (int i = 0; i < vessel->GetThrusterCount(); i++)
				{
					vessel->SetThrusterLevel(vessel->GetThrusterHandleByIndex(i),0);
				}
				VECTOR3 fVector;
				vessel->GetThrustVector(fVector);
				fVector.x*=-1;
				fVector.y*=-1;
				fVector.z*=-1;
				vessel->AddForce(fVector,_V(0,0,0));
			}
			else
			{
				vessel->SetADCtrlMode(modes & 0x7);
			}
		}
	}
}
bool UMMUFA::init()
{
	state=ST_GENERIC;
	return Read();
}
VOBJ* UMMUFA::ReadGeneric(char * config, OBJHANDLE handle)
{
	if (fopen(config,"r")==0){
		return NULL;
	}
	if (!oapiIsVessel(handle))
		return NULL;

	PARSER parser(config);
	parser.read();

	AirLockVector alv;
	AirLock       al;
	CrewMemberDef cmd;
	cmd.clear();
	int           alc=0;
	char          key[255];
	int           seat=0;
	BOOL          koc=FALSE;
	double        khv=-4;
	VOBJ*         vbj=NULL;
	
	alc  = parser.Int("AirlockCount");
	seat = parser.Int("seats");
	koc  = parser.Int("KillOnCrash");
	
	if (koc == TRUE)
		khv=parser.Double("KillHVel");
	
	for (int i = 0; i < alc; i++)
	{
		sprintf(key,"Airlock_%d_PosP",i+1);
		al.posp=parser.Vector3(key);
		sprintf(key,"Airlock_%d_PosN",i+1);
		al.posn=parser.Vector3(key);
		sprintf(key,"Airlock_%d_Pos",i+1);
		al.pose=parser.Vector3(key);
		sprintf(key,"Airlock_%d_rot",i+1);
		al.rot=parser.Vector3(key);
		alv.push_back(al);
	}
	vbj = new VOBJ(alv,cmd,handle,seat);
	vbj->koc=koc;
	vbj->khv=khv;
	atlist.push_back(vbj);
	return vbj;
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

	AirLockVector  alv;
	AirLock        al;
	CREWMEMBERDEF  cd;
	CrewMemberDef  cmd;
	int            cnt = 0;
    int            alc = 0;
	char           key[255];
	int            seat=0;
	BOOL           koc=FALSE;
	double         khv=-4;
	VOBJ*          vbj=NULL;
	for (int i = 0; i < vcount; i++)
	{
		alv.clear();
		cmd.clear();

		sprintf(key,"Vessel_%d_Name",i+1);
		char n[255];
		sprintf(n,"%s",parser.String(key,n));
		h = oapiGetObjectByName(n);
		if (!oapiIsVessel(h))
			continue;

		sprintf(key,"Vessel_%d_AirlockCount",i+1);
		alc = parser.Int(key);
		sprintf(key,"Vessel_%d_seats",i+1);
		seat=parser.Int(key);
		sprintf(key,"Vessel_%d_KillOnCrash",i+1);
		koc=parser.Int(key);
		if (koc==TRUE)
		{
			sprintf(key,"Vessel_%d_KillHVel",i+1);
			khv=parser.Double(key);
		}
		for (int j = 0; j < alc; j++)
		{
			sprintf(key,"Vessel_%d_Airlock_%d_PosP",i+1,j+1);
			al.posp=parser.Vector3(key);
			sprintf(key,"Vessel_%d_Airlock_%d_PosN",i+1,j+1);
			al.posn=parser.Vector3(key);
			sprintf(key,"Vessel_%d_Airlock_%d_Pos",i+1,j+1);
			al.pose=parser.Vector3(key);
			sprintf(key,"Vessel_%d_Airlock_%d_rot",i+1,j+1);
			al.rot=parser.Vector3(key);
			alv.push_back(al);
		}
		sprintf(key,"Vessel_%d_CrewCount",i+1);
		cnt = parser.Int(key);
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
		vbj=new VOBJ(alv,cmd,h,seat);
		vbj->koc=koc;
		vbj->khv=khv;
		atlist.push_back(vbj);
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
	AirLock al;
	char name[255];
	for (int i = 0; i < atlist.size(); i++)
	{
		v = atlist[i];
		sprintf(slog,";Vessel %d GENERAL PARAMTERS",i+1);
		logger.write(sep);
		logger.write(slog);
		logger.write(sep);
		sprintf(slog,"Vessel_%d_Name = %s",i+1,VESSEL(v->hook).GetName());
		logger.write(slog);
		sprintf(slog,"Vessel_%d_AirlockCount = %d",i+1,v->alv.size());
		logger.write(slog);
		sprintf(slog,"Vessel_%d_CrewCount = %d",i+1,v->crew.GetCrewTotalNumber());
		logger.write(slog);
		sprintf(slog,"Vessel_%d_seats = %d",i+1,v->seat);
		logger.write(slog);
		sprintf(slog,"Vessel_%d_KillOnCrash = %d",i+1, v->koc);
		logger.write(slog);
		sprintf(slog,"Vessel_%d_KillHVel = %.2f",i+1,v->khv);
		logger.write(slog);
		
		sprintf(slog,";Vessel %d AIRLOCK PARAMTERS",i+1);
		logger.write(sep);
		logger.write(slog);
		logger.write(sep);

		for (int j = 0; j < v->alv.size(); j++)
		{
			al = v->alv[j];
			sprintf(slog,";Airlock %d",j+1);
			logger.write(slog);
			sprintf(slog,"Vessel_%d_Airlock_%d_PosP = %.2f %.2f %.2f",i+1,j+1,al.posp.x,al.posp.y,al.posp.z);
			logger.write(slog);
			sprintf(slog,"Vessel_%d_Airlock_%d_PosN = %.2f %.2f %.2f",i+1,j+1,al.posn.x,al.posn.y,al.posn.z);
			logger.write(slog);
			sprintf(slog,"Vessel_%d_Airlock_%d_Pos = %.2f %.2f %.2f",i+1,j+1,al.pose.x,al.pose.y,al.pose.z);
			logger.write(slog);
			sprintf(slog,"Vessel_%d_Airlock_%d_rot = %.2f %.2f %.2f",i+1,j+1,al.rot.x,al.rot.y,al.rot.z);
			logger.write(slog);
		}
		sprintf(slog,";Vessel %d MMU PARAMTERS",i+1);
		logger.write(sep);
		logger.write(slog);
		logger.write(sep);

		for (int j = 0; j < v->crew.GetCrewTotalNumber(); j++)
		{
			strcpy(name,v->crew.GetCrewNameBySlotNumber(j));
			sprintf(slog,";MMU %d",j+1);
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
bool UMMUFA::isSpacecraft(OBJHANDLE h)
{
	if (!oapiIsVessel(h))
		return false;
	VESSEL * v = oapiGetVesselInterface(h);
	if (v->GetClassName() == NULL)
		return true;
	if (!strncmp("Spacecraft",v->GetClassName(),10))
		return true;
	return false;
}
bool UMMUFA::hasConfig(OBJHANDLE h)
{
	if (!oapiIsVessel(h))
		return false;
	char cbuf[255];
	if (isSpacecraft(h))
		sprintf(cbuf,"Config/UMMUFA/%s.cfg",VESSEL(h).GetName());
	else
		sprintf(cbuf,"Config/UMMUFA/%s.cfg",VESSEL(h).GetClassName());
	if (fopen(cbuf,"r"))
		return true;
	return false;
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
char * UMMUFA::CFG(OBJHANDLE h, char *buffer)
{
	if (isSpacecraft(h))
		sprintf(buffer,"Config/UMMUFA/%s.cfg",VESSEL(h).GetName());
	else
		sprintf(buffer,"Config/UMMUFA/%s.cfg",VESSEL(h).GetClassName());
	return buffer;
}
