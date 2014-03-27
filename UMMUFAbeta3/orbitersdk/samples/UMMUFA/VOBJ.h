#ifndef _H_VOBJ_AGAIN_ELYSA_COMPUTEREX_4EVR_
#define _H_VOBJ_AGAIN_ELYSA_COMPUTEREX_4EVR_

#include <orbitersdk.h>
#include <vector>
#include <UMmuSDK.h>
#include "HudScroll.h"
#include "LOGGER.h"

class AirLock
{
public:
	AirLock(VECTOR3 c1, VECTOR3 c2, VECTOR3 r, VECTOR3 eva)
	{
		posp = c1;
		posn = c2;
		rot  = r;
		pose = eva;
	}
	AirLock()
	{
		posp = _V(0,0,0);
		posn = _V(0,0,0);
		rot  = _V(0,0,0);
		pose = _V(0,0,0);
	}
	VECTOR3 posp, posn, rot, pose;
};

class VOBJ
{
public:
	VOBJ(std::vector<AirLock> a, std::vector<CREWMEMBERDEF> b, OBJHANDLE h, DWORD s)
	{
		alv            = a;
		hook           = h;
		lock           = 0;
		seat           = s;
		slot           = lock;
		init(b);
		hudprint       = HUDSCROLL(4);
		koc            = FALSE;
		khv            = -4;
	}
	VOBJ()
	{
		hook           = 0;
		lock           = 0;
		seat           = 1;
		slot           = lock;
		hudprint       = HUDSCROLL(4);
		koc            = FALSE;
		khv            = -4;
	}
	void init(std::vector<AirLock> a, std::vector<CREWMEMBERDEF> b, OBJHANDLE h, DWORD s);
protected:
	void init(std::vector<CREWMEMBERDEF> def);
public:
	int  TimeStep(void);
	int nextlock()
	{
		if (lock+1 >= alv.size())
			lock=0;
		else
			lock++;
		crew.DefineAirLockShape(TRUE,alv[lock].posn.x,alv[lock].posp.x,alv[lock].posn.y,alv[lock].posp.y,alv[lock].posn.z,alv[lock].posp.z);
		crew.SetMembersPosRotOnEVA(alv[lock].pose, alv[lock].rot);
		crew.SetActiveDockForTransfer(lock);
		return lock;
	}
	char * nextslot()
	{
		if (slot+1 >= crew.GetCrewTotalNumber())
			slot=0;
		else
			slot++;
		return crew.GetCrewNameBySlotNumber(slot);
	}
	int eva()
	{
		char TempMiscID[5]={0};
		int  I=0;
		char MeshToUse[26]={"ummu/ummu"};
		strcpy(TempMiscID,crew.GetCrewMiscIdByName(crew.GetCrewNameBySlotNumber(slot)));
		for(I=0;I<NUMBER_OF_MMU_TYPE;I++)
		{
			if(strcmp(UmmuMiscID[I],TempMiscID)==0)
			{
				strcpy(MeshToUse,UmmuMeshUsed[I]);
				break;
			}
		}
		crew.SetAlternateMeshToUseForEVASpacesuit(MeshToUse);
		return crew.EvaCrewMember(crew.GetCrewNameBySlotNumber(slot));
	}
	std::vector<AirLock> alv;
	UMMUCREWMANAGMENT    crew;
	OBJHANDLE            hook;
	DWORD                lock;
	DWORD                seat;
	DWORD                slot;
	HUDSCROLL            hudprint;
	BOOL                 koc;
	double               khv;
};

typedef std::vector<AirLock>       AirLockVector;
typedef std::vector<CREWMEMBERDEF> CrewMemberDef;

#endif
