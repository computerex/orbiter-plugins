#ifndef _H_DUST_MANAGER_ORB_DUST_COMPUTEREX_H
#define _H_DUST_MANAGER_ORB_DUST_COMPUTEREX_H

#include <orbitersdk.h>
#include "VOBJ.h"
#include <vector>

class DUSTMANAGER
{
public:
	void PreStep(void);
	bool isAttended(VESSEL * v);
	int  hasHovers(OBJHANDLE hVes);
	std::vector<VOBJ*> atList;
};
#endif