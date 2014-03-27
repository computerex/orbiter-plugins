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
	}
	void  TimeStep(void);
	VOBJ* GetFocus(void);
	bool  init(FILEHANDLE scn);
	void  save(FILEHANDLE scn);
	bool  isAttended(OBJHANDLE h);
	int   hasDocks(OBJHANDLE h);
	bool  ischecked(OBJHANDLE ves);
	bool  isUmmu(OBJHANDLE h);
	bool  isUmmuCompatible(OBJHANDLE h);
protected:
	bool  Read(FILEHANDLE scn);
	void  Prune();
public:
	std::vector<VOBJ*> atlist;
	std::vector<OBJHANDLE> bklist;
};
#endif
