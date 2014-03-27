class CAMERA: public VESSEL2 {
public:
	CAMERA (OBJHANDLE hVessel, int flightmodel);
	void clbkPreStep(double simt, double simdt, double mjd);
	int  clbkConsumeDirectKey (char *kstate);
	int  clbkConsumeBufferedKey (DWORD key, bool down, char *kstate);
	void clbkDrawHUD (int mode, const HUDPAINTSPEC *hps, HDC hDC);
	OBJHANDLE hTarget;
};