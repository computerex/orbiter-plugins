#ifndef _H_FUEL_TRANSFER_MFD_COMPUTEREX_H_WILL_GET_DUX_
#define _H_FUEL_TRANSFER_MFD_COMPUTEREX_H_WILL_GET_DUX_
#include <orbitersdk.h>
#include "VOBJ.h"

class MFDData
{
public:
	MFDData(OBJHANDLE h)
	{
		hook = new VOBJ(h);
		ShowHelp=FALSE;
	}
	MFDData()
	{
		hook=new VOBJ();
		ShowHelp=FALSE;
	}
	VOBJ * hook; 
	BOOL   ShowHelp;
};

class FTMFD: public MFD {
public:
	FTMFD (DWORD w, DWORD h, VESSEL *vessel);
	~FTMFD ();
	bool ConsumeKeyBuffered (DWORD key);
	bool ConsumeButton (int bt, int event);
	char *ButtonLabel (int bt);
	int  ButtonMenu (const MFDBUTTONMENU **menu) const;
	void Update (HDC hDC);
	static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);
	MFDData * data;
	DWORD height, width;
};
#endif
