#ifndef _H_TELEPORTER_ORBITER_COMPUTEREX
#define _H_TELEPORTER_ORBITER_COMPUTEREX


#include "orbitersdk.h"


class TELEPORTER: public VESSEL2 {
public:
	TELEPORTER (OBJHANDLE hVessel, int flightmodel);

	void clbkSetClassCaps (FILEHANDLE cfg);
	int  clbkConsumeBufferedKey(DWORD key, bool down, char *kstate);
	void clbkDrawHUD(int mode, const HUDPAINTSPEC * hps, HDC hDC);
	void clbkPreStep(double simt, double simdt, double mjd);
	void clbkPostCreation(void);
	void clbkLoadStateEx (FILEHANDLE scn, void *vs);
	void clbkSaveState (FILEHANDLE scn);
	void TeleEffect(void);
	int  ResolveIndex(OBJHANDLE hVes);
	static bool isTeleporter(OBJHANDLE hObj);
	bool AreVesselsInRange(void);
	double Tele_Distance;
	OBJHANDLE GetVesselInRange(void);
	char  hDestVessel[255];
};

#endif