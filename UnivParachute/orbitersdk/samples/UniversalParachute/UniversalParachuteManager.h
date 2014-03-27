#ifndef _H_COMPUTEREX_UNIVERSAL_PARACHUTE_MANAGER_H_
#define _H_COMPUTEREX_UNIVERSAL_PARACHUTE_MANAGER_H_
#pragma once

#include "VOBJ.h"
#include <orbitersdk.h>
#include <vector>

struct PPARAM
{
	char      mesh[255];
	double    area, dcoef;
	VECTOR3   ofs;
	CHUTETYPE type;
};
class UniversalParachuteManager
{
public:
	~UniversalParachuteManager()
	{
		for (int i = 0; i < atList.size(); i++)
			delete atList[i];
	}
	bool    hasConfig(OBJHANDLE h);
	bool    isSpacecraft(OBJHANDLE h);
	bool    isRegistered(OBJHANDLE hVes);
	VOBJ*   Register(OBJHANDLE hVes);
	bool    FocusIsRegistered();
	VOBJ*   GetFocus();
	void    TimeStep(void);
	void    Prune();
	std::vector<VOBJ*> atList;
};
#endif

