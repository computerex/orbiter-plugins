#ifndef _H_COMPUTEREX_PARACHUTE_VESSEL_H_
#define _H_COMPUTEREX_PARACHUTE_VESSEL_H_
#pragma once

class Parachute: public VESSEL2 {
public:
	Parachute (OBJHANDLE hVessel, int flightmodel);
	void clbkSetClassCaps (FILEHANDLE cfg);
	void clbkPreStep(double simt, double simdt, double mjd);
	void clbkDrawHUD(int mode, const HUDPAINTSPEC * hps, HDC hDC);
};
#endif
