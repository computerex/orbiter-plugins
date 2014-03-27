#ifndef _H_COMPUTEREX_SPOTLIGHT_SPOTLIGHTMANAGER_
#define _H_COMPUTEREX_SPOTLIGHT_SPOTLIGHTMANAGER_

#include "orbitersdk.h"
#include "VOBJ.h"
#include "FileReader.h"

class SPOTLIGHTMANAGER
{
public:

	void GetConfigurations(void);
	bool isAttended(VESSEL * v);
	std::vector<SPOTLIGHT*> ReadSpotlightData(char * inifile);
	std::vector<BEACON*> ReadBeaconData(char * inifile);
	void TimeStep(void);
	void AssignSpotlights(VOBJ * vbj);
	void AssignBeacons(VOBJ * vbj);
	bool hasConfig(VESSEL * v);
	bool isSpacecraft(VESSEL * v);
	std::vector<VOBJ*> atList;
	std::vector<double> thLvl;
};

#endif