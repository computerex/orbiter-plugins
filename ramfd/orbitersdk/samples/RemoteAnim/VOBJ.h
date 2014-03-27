#ifndef _H_COMPUTEREX_REMOTE_ANIMATIONS_H_VOBJ_H_
#define _H_COMPUTEREX_REMOTE_ANIMATIONS_H_VOBJ_H_

#define CFGPATH "Config/RAMFD/"

#include <orbitersdk.h>
#include <vector>
#include "PARSER.h"

bool isSpacecraft(OBJHANDLE);
char * GetCFGName(OBJHANDLE h, char * buffer);
bool hasConfig(OBJHANDLE);
class ANIMATION
{
public:
	ANIMATION()
	{
		id = 0;
		strcpy(name," ");
		state=1;
	}
	ANIMATION(int i, char * n)
	{
		id = i;
		strcpy(name, n);
		state=1;
	}
	int    id;
	char   name[30];
	double state;
};

class VOBJ
{
public:
	VOBJ(OBJHANDLE h)
	{
		hook = h;
	}
	VOBJ()
	{
		hook=0;
	}
	void reset(int i)
	{
		VESSEL(hook).SetAnimation(av[i].id, 0);
		av[i].state=0;
	}
	void reset()
	{
		for (int i = 0; i < av.size(); i++)
		{
			VESSEL(hook).SetAnimation(av[i].id,0);
			av[i].state=0;
		}
	}
	void finish(int i)
	{
		VESSEL(hook).SetAnimation(av[i].id, 1);
		av[i].state=1;
	}
	void finish()
	{
		for (int i = 0; i < av.size(); i++)
		{
			VESSEL(hook).SetAnimation(av[i].id,1);
			av[i].state=1;
		}
	}
	void setstate(int i, double st)
	{
		if (i >= av.size())
			return;
		if (st > 1)
			st=1;
		if (st < 0)
			st=0;
		VESSEL(hook).SetAnimation(av[i].id, st);
		av[i].state=st;
	}
	void setstate(double st)
	{
		if (st > 1)
			st=1;
		if (st < 0)
			st=0;
		for (int i = 0; i < av.size(); i++)
		{
			VESSEL(hook).SetAnimation(av[i].id,st);
			av[i].state=st;
		}
	}
	double state(int i)
	{
		if (i >= av.size())
			return 0;
		return av[i].state;
	}
	bool HasSavedState(OBJHANDLE hv, PARSER gpr);
	bool init(PARSER gpr);
	bool init(void);
	OBJHANDLE hook;
	std::vector<ANIMATION> av;
};
#endif
