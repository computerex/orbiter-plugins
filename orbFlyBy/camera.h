#ifndef _H_COMPUTEREX_ORB_FLY_BY_H_
#define _H_COMPUTEREX_ORB_FLY_BY_H_

#include <vector>

class CM_CFG
{
public:
	CM_CFG(VECTOR3 pos, VECTOR3 vel)
	{
		this->pos = pos;
		this->vel = vel;
	}
	CM_CFG()
	{
		pos = vel = _V(0,0,0);
	}
	VECTOR3 pos, vel;
};

class CAMERA: public VESSEL2 {
public:
	CAMERA (OBJHANDLE hVessel, int flightmodel);
	void clbkPreStep(double simt, double simdt, double mjd);
	int  clbkConsumeDirectKey (char *kstate);
	int  clbkConsumeBufferedKey (DWORD key, bool down, char *kstate);
	void clbkDrawHUD (int mode, const HUDPAINTSPEC *hps, HDC hDC);
	void setTarget(OBJHANDLE hTarget) { this->hTarget = hTarget; }
	void clbkSetClassCaps(FILEHANDLE cfg);
	OBJHANDLE getTarget() { return hTarget; }
private:
	OBJHANDLE hTarget;
	bool track;
	VECTOR3 pos, vel, pan;
	double ofs, panofs;
	std::vector<CM_CFG> views;
	DWORD selectedView;
	double camDist;
};

#endif

