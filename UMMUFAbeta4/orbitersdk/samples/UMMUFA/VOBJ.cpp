#include "VOBJ.h"

int VOBJ::TimeStep(void)
{
	int r = crew.ProcessUniversalMMu();
	char cbuf[255];
	switch(r)
	{
	case UMMU_RETURNED_TO_OUR_SHIP:
		{
			sprintf(cbuf,"%s %s aged %d entered the ship",crew.GetCrewMiscIdByName(crew.GetLastEnteredCrewName()),crew.GetLastEnteredCrewName(),crew.GetCrewAgeByName(crew.GetLastEnteredCrewName()));
			hudprint.insert(cbuf);
			break;
		}
	case UMMU_TRANSFERED_TO_OUR_SHIP:
		{
			sprintf(cbuf,"%s %s aged %d entered to ship",crew.GetCrewMiscIdByName(crew.GetLastEnteredCrewName()),crew.GetLastEnteredCrewName(),crew.GetCrewAgeByName(crew.GetLastEnteredCrewName()));
			hudprint.insert(cbuf);
			break;
		}
	}
	return r;
}
void VOBJ::init(std::vector<CREWMEMBERDEF> d)
{
	crew.InitUmmu(hook);
	crew.SetMaxSeatAvailableInShip(10);
	CREWMEMBERDEF c;
	nextlock();
	for (int i = 0; i < d.size(); i++)
	{
		c = d[i];
		crew.AddCrewMember(c.Name,c.Age,c.Pulse,c.Weight,c.MiscId);
	}
}
void VOBJ::init(std::vector<CREWMEMBERDEF> b, OBJHANDLE h)
{
	hook = h;
	init(b);
}
int  VOBJ::nextlock()
{
	VESSEL * v = oapiGetVesselInterface(hook);
	UINT count = v->DockCount();
	if (dock+1 >= count)
		dock=0;
	else
		dock++;
	VECTOR3 pos, rot, dir;
	DOCKHANDLE dHandle = v->GetDockHandle(dock);
	v->GetDockParams(dHandle, pos, dir, rot);
	//crew.DefineAirLockShape(TRUE,-5,5,-5,5,-5,5);
	crew.DefineAirLockShape(TRUE, min(pos.x, -pos.x)-5, max(pos.x, -pos.x)+5, min(pos.y, -pos.y)-5, max(pos.y, -pos.y)+5, 
		min(pos.z, -pos.z)-5, max(pos.z, -pos.z)+5);
	crew.SetMembersPosRotOnEVA(pos, rot);
	crew.SetActiveDockForTransfer(dock);
	return dock;
}
char * VOBJ::nextslot()
{
	if (UmmuSlot+1 >= crew.GetCrewTotalNumber())
		UmmuSlot=0;
	else
		UmmuSlot++;
	return crew.GetCrewNameBySlotNumber(UmmuSlot);
}
int VOBJ::eva()
{
	char TempMiscID[5]={0};
	int  I=0;
	char MeshToUse[26]={"ummu/ummu"};
	strcpy(TempMiscID,crew.GetCrewMiscIdByName(crew.GetCrewNameBySlotNumber(UmmuSlot)));
	for(I=0;I<NUMBER_OF_MMU_TYPE;I++)
	{
		if(strcmp(UmmuMiscID[I],TempMiscID)==0)
		{
			strcpy(MeshToUse,UmmuMeshUsed[I]);
			break;
		}
	}
	crew.SetAlternateMeshToUseForEVASpacesuit(MeshToUse);
	return crew.EvaCrewMember(crew.GetCrewNameBySlotNumber(UmmuSlot));
}