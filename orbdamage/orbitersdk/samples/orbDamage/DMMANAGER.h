#ifndef _H_COMPUTEREX_DAMAGE_MANAGER_H_
#define _H_COMPUTEREX_DAMAGE_MANAGER_H_
#define CFGPATH "Config/Damage/"

#include "VOBJ.h"
#include "Parser.h"
#include <vector>
#include <time.h>

class VPROFILE
{
public:
	VPROFILE()
	{
		strcpy(name,"");
	}
	VPROFILE(char * nam, VLIMIT lim)
	{
		limit = lim;
		strcpy(name,nam);
	}
	VLIMIT limit;
	char   name[255];
};

class DMMANAGER
{
private:
	bool     hasConfig(OBJHANDLE h);
	bool     isSpacecraft(OBJHANDLE h);
	VPROFILE Register(OBJHANDLE hVes);
	bool     isRegistered(OBJHANDLE hves);
	bool     ProfileExists(OBJHANDLE hVes, VLIMIT &lm);
	void     Prune();
	void     LoadHUDOptions();
public:
	DMMANAGER()
	{
		LoadHUDOptions();
		srand(time(NULL));
	}
	void     PreStep();
	void     PostStep();
private:
	std::vector<VPROFILE> profiles;
	std::vector<VOBJ*>    atlist;
	HUDOPTIONS            hudop;
};
#endif

