#ifndef _NAME_CHANGE_MFD_H_
#define _NAME_CHANGE_MFD_H_

class NameChangeMFD: MFD {
public:
	NameChangeMFD (DWORD w, DWORD h, VESSEL *vessel);
	bool ConsumeKeyBuffered (DWORD key);
	bool ConsumeButton (int bt, int event);
	char *ButtonLabel (int bt);
	int  ButtonMenu (const MFDBUTTONMENU **menu) const;
	void Update (HDC hDC);
	static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);
};

#endif 