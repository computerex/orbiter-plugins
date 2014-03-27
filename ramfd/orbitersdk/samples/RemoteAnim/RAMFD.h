#ifndef _H_REMOTE_ANIMATION_MFD_HEADER_COMPUTEREX_H_
#define _H_REMOTE_ANIMATION_MFD_HEADER_COMPUTEREX_H_

#include <orbitersdk.h>
#include "VOBJ.h"

bool  isAttended(OBJHANDLE);

class MFDData
{
public:
	MFDData()
	{
		iSelected=0;
		animSpeed=0.01;
		iSelectedAnimation=0;
		iSelectedAttachment=0;
		toparent=true;
		fAttachDistance=1;
		relVel=1;
	}
	int iSelected, iSelectedAnimation;
	double animSpeed;
	OBJHANDLE hook;
	bool      toparent;
	int       iSelectedAttachment;
	double    fAttachDistance;
	double    relVel;
};
class ATTACHMENTSPEC
{
public:
	ATTACHMENTSPEC()
	{
		hook=0;
		inx=0;
	}
	OBJHANDLE hook;
	DWORD     inx;
};
class RAMFD: public MFD {
public:
	RAMFD (DWORD w, DWORD h, VESSEL *vessel);
	~RAMFD ();
	bool ConsumeKeyBuffered (DWORD key);
	bool ConsumeButton (int bt, int event);
	char *ButtonLabel (int bt);
	int  ButtonMenu (const MFDBUTTONMENU **menu) const;
	void Update (HDC hDC);
	static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);
	MFDData * data;
	DWORD width, height;
};

#endif 
