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
	if (!koc || !crew.GetCrewTotalNumber())
		return r;
	VESSEL * v = oapiGetVesselInterface(hook);
	VECTOR3 horz;
	v->GetHorizonAirspeedVector(horz);
	if (horz.y < khv && v->GroundContact())
	{
		for (int i = 0; i < crew.GetCrewTotalNumber(); i++)
		{
			crew.SetCrewMemberPulseByName(crew.GetCrewNameBySlotNumber(i),0);
		}
		while(crew.GetCrewTotalNumber())
			crew.EjectCrewMember(crew.GetCrewNameBySlotNumber(0));
		sprintf(cbuf,"crash at %.2f everybody dead",horz.y);
		hudprint.insert(cbuf);
	}
	return r;
}
void VOBJ::init(std::vector<CREWMEMBERDEF> d)
{
	crew.InitUmmu(hook);
	crew.SetMaxSeatAvailableInShip(seat);
	CREWMEMBERDEF c;
	nextlock();
	for (int i = 0; i < d.size(); i++)
	{
		c = d[i];
		crew.AddCrewMember(c.Name,c.Age,c.Pulse,c.Weight,c.MiscId);
	}
}
void VOBJ::init(std::vector<AirLock> a, std::vector<CREWMEMBERDEF> b, OBJHANDLE h, DWORD s)
{
	alv  = a;
	hook = h;
	seat = s;
	init(b);
}
