#ifndef _H_COMPUTEREX_STAR_WARS_PHYSICS_MFD_THREE_AM_HEADER_H_
#define _H_COMPUTEREX_STAR_WARS_PHYSICS_MFD_THREE_AM_HEADER_H_

#include <orbitersdk.h>
#include "VOBJ.h"

class SWPMFD: public MFD {
public:
	SWPMFD (DWORD w, DWORD h, VESSEL *vessel);
	~SWPMFD ();
	bool ConsumeKeyBuffered (DWORD key);
	bool ConsumeButton (int bt, int event);
	char *ButtonLabel (int bt);
	void Update (HDC hDC);
	static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);
	VOBJ * vbj;
	DWORD height, width;
};

#endif
