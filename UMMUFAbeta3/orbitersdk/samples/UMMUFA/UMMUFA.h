#ifndef _H_UMMU_FOR_ALL_CTEX_
#define _H_UMMU_FOR_ALL_CTEX_

#include "VOBJ.h"
#include "HudScroll.h"
#include "EHUD.h"

enum SYSTEMSTATE { ST_GENERIC=0, ST_ALL };

class UMMUFA
{
public:
	UMMUFA()
	{
		strcpy(cfg,"NONE");
		SpecialSave=FALSE;
		DisableWhenEmpty=FALSE;
	}
	void  TimeStep(void);
	VOBJ* GetFocus(void)
	{
		VESSEL * v = oapiGetFocusInterface();
		for (int i = 0; i < atlist.size(); i++)
		{
			if (atlist[i]->hook==v->GetHandle())
				return atlist[i];
		}
		return NULL;
	}
	bool init();
	void save();
	void save(char * file);
	bool  hasConfig(OBJHANDLE h);
	bool  isSpacecraft(OBJHANDLE h);
	bool  isAttended(OBJHANDLE h);
protected:
	bool  Read();
	VOBJ* ReadGeneric(char * config, OBJHANDLE h);
	void  Prune();
	char* CFG(OBJHANDLE h, char * buffer);
public:
	char cfg[255];
	std::vector<VOBJ*> atlist;
	SYSTEMSTATE state;
	BOOL SpecialSave;
	char SpecialSaveBuffer[255];
	BOOL DisableWhenEmpty;
};
#endif
