#ifndef _H_BOOM_MANAGER_COMPUTEREX_POWER
#define _H_BOOM_MANAGER_COMPUTEREX_POWER

#include <orbitersdk.h>
#include "VOBJ.h"
#include <MFDSoundSDK35.h>
class BOOMMANAGER
{
public:

	void TimeStep(void);
	VOBJ* ReadAndAssign(char * file, VESSEL * v);
	bool hasConfig(VESSEL * v);
	bool isSpacecraft(VESSEL * v);
	bool isAttended(VESSEL * v);
	char * INI(VESSEL * v);
	std::vector<VOBJ*> atList;
	int MyID;
};
#endif