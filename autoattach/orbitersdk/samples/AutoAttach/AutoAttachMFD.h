#ifndef _H_COMPUTEREX_AUTO_ATTACH_MFD_H_
#define _H_COMPUTEREX_AUTO_ATTACH_MFD_H_
#include <orbitersdk.h>
#include "AutoAttachManager.h"

class AAMANAGERMFD: public MFD {
public:
	AAMANAGERMFD (DWORD w, DWORD h, VESSEL *vessel);
	~AAMANAGERMFD ();
	bool ConsumeKeyBuffered (DWORD key);
	void Update (HDC hDC);
	static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);
	DWORD width, height;
};

#endif