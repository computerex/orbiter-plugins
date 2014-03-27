#ifndef _H_COMPUTEREX_PARTICLE_MANAGER_H_
#define _H_COMPUTEREX_PARTICLE_MANAGER_H_
#pragma once
#include <orbitersdk.h>
#include <vector>
#define  CFGPATH "config/particles/"

class ParticleProfile
{
public:
	PARTICLESTREAMSPEC spec;
};
struct THRUSTER
{
	THGROUP_TYPE    type;
	THRUSTER_HANDLE th;
};
class VOBJ
{
public:
	VOBJ(OBJHANDLE hook, std::vector<THRUSTER*> thList)
	{
		this->hook=hook;
		this->thList=thList;
	}
	VOBJ()
	{
		hook=NULL;
	}
	void TimeStep()
	{
		VESSEL * v = oapiGetVesselInterface(hook);
		double lvl = 0;
		for (int i = 0; i < 15; i++)
		{
			lvl = v->GetThrusterGroupLevel((THGROUP_TYPE)i); 
			if (lvl)
			{
				for (DWORD j = 0; j < thList.size(); j++)
				{
					if (thList[j]->type==(THGROUP_TYPE)i)
						v->SetThrusterLevel(thList[j]->th, lvl);
				}
			}else
			{
				for (DWORD j = 0; j < thList.size(); j++)
				{
					if (thList[j]->type==(THGROUP_TYPE)i)
						v->SetThrusterLevel(thList[j]->th, 0);
				}
			}
		}
	}
	OBJHANDLE hook;
	std::vector<THRUSTER*> thList;
};
class ParticleManager
{
public:
	~ParticleManager();
	bool isRegistered(OBJHANDLE hObj);
	bool hasConfig(OBJHANDLE hObj, char * cfg);
	bool hasEngines(OBJHANDLE hObj);
	void TimeStep();
	void apply(OBJHANDLE hObj, char * cfg);
	void Prune();
	std::vector<VOBJ*> atList;
};

#endif
