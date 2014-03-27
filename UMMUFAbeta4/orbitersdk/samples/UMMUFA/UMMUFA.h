#ifndef _H_UMMU_FOR_ALL_CTEX_
#define _H_UMMU_FOR_ALL_CTEX_

#include "VOBJ.h"
#include "HudScroll.h"
#include "EHUD.h"

class UMMUFA
{
public:
	UMMUFA()
	{
		strcpy(cfg,"NONE");
		SpecialSave=FALSE;
	}
	void  TimeStep(void);
	VOBJ* GetFocus(void);
	bool  init();
	void  save();
	void  save(char * file);
	bool  isAttended(OBJHANDLE h);
	int   hasDocks(OBJHANDLE h);
	bool  isUmmu(OBJHANDLE h);
	bool  isUmmuCompatible(OBJHANDLE h);
protected:
	bool  Read();
	void  Prune();
public:
	char cfg[255];
	std::vector<VOBJ*> atlist;
	BOOL SpecialSave;
	char SpecialSaveBuffer[255];
	std::vector<std::string> IgnoreList;
};
#endif
