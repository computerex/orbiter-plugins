#ifndef _H_COMPUTEREX_VOBJ_UNIVERSAL_PARACHUTE_H_
#define _H_COMPUTEREX_VOBJ_UNIVERSAL_PARACHUTE_H_
#pragma once

#include "Parachute.h"
#include <vector>

class VOBJ
{
public:
	VOBJ(OBJHANDLE hook)
	{
		this->hook=hook;
	}
	VOBJ()
	{
		hook=NULL;
	}
	~VOBJ()
	{
		for (int i = 0; i < chutes.size(); i++)
			delete chutes[i];
	}
	void TimeStep()
	{
		for (int i = 0; i < chutes.size(); i++)
			chutes[i]->Update();
	}
	std::vector<PARACHUTE*> chutes;
	OBJHANDLE               hook;
};
#endif
