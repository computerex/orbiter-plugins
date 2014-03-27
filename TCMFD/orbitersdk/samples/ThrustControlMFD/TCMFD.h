#ifndef _H_THRUSTER_CONTROL_COMPUTEREX_H_
#define _H_THRUSTER_CONTROL_COMPUTEREX_H_
#pragma once
#define  NUM_OF_PAGES 2
#define  PAGE_MAIN    0
#define  PAGE_AP      1

#include <orbitersdk.h>
#include "VOBJ.h"
#include <vector>
#include <string>
#include "REMT.h"
#include <MfdSoundSDK35.h>

class TCPOINT
{
public:
	TCPOINT()
	{
		x = y = 0;
	}
	double x, y;
};
class MFDData
{
public:
	MFDData(VOBJ * v, DWORD w, DWORD h, int id)
	{
		this->v=v;
		Page = PAGE_MAIN;
		SelectedAP=0;
		lSampleTime=0;
		width=w/35;
		height=h/24;
		remoteThrust = new RTHRUST(v->getHookHandle(), id, this);
	}
	MFDData()
	{
		Page = PAGE_MAIN;
		SelectedAP=0;
		lSampleTime=0;
		width = height = 0;
		remoteThrust = new RTHRUST();
	}
	VOBJ *  v;
	int     Page;
	int     SelectedAP;
	TCPOINT points[17];
	double  lSampleTime;
	DWORD   width, height;
	RTHRUST *remoteThrust;
};
class TCMFD: public MFD {
public:
	TCMFD (DWORD w, DWORD h, VESSEL *vessel);
	~TCMFD ();
	bool ConsumeKeyBuffered (DWORD key);
	bool ConsumeButton (int bt, int event);
	char *ButtonLabel (int bt);
	int  ButtonMenu (const MFDBUTTONMENU **menu) const;
	void Update (HDC hDC);
	int  nextPage()
	{
		if (data->Page+1 >= NUM_OF_PAGES)
			data->Page=0;
		else
			data->Page++;
		return data->Page;
	}
	int prevPage()
	{
		if (data->Page-1 < 0)
			data->Page=NUM_OF_PAGES-1;
		else
			data->Page--;
		return data->Page;
	}
	int nextAP()
	{
		if (data->SelectedAP+1 >= NUM_OF_AP)
			data->SelectedAP=0;
		else
			data->SelectedAP++;
		return data->SelectedAP;
	}
	void PrintMain(HDC hDC);
	void PrintAP(HDC hDC);
	void PrintRT(HDC hDC);
	static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);
	MFDData * data;
	DWORD width, height;
};
#endif