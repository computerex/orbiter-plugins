#ifndef _H_COMPUTEREX_ALWAYS_SOME_TYPE_OF_MANAGER
#define _H_COMPUTEREX_ALWAYS_SOME_TYPE_OF_MANAGER

#include "VOBJ.h"

class RMANAGER
{
public:
	void TimeStep();
	bool hasConfig(VESSEL * v);
	bool isSpacecraft(VESSEL * v);
	bool isAttended(VESSEL * v);
	VOBJ*ReadAndAssign(char * file, VESSEL * v);
	void CreateResourceAndStreams(VOBJ * vbj);
	char * INI(VESSEL * v);
	double calcFlux(VESSEL * v);
	bool   isFocus(VESSEL * v);
	std::vector<VOBJ*> atList;
};
#endif