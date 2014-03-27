#ifndef _H_UNI_PARA_MFD_H_COMPUTEREX_H_
#define _H_UNI_PARA_MFD_H_COMPUTEREX_H_
#pragma once
#include <orbitersdk.h>
#include "UniversalParachuteManager.h"
#include <string>
#include <vector>

void Tokenize(const std::string& str,
			  std::vector<std::string>& tokens,
			  const std::string& delimiters = " ")
{
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
    {
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}

class MFDData
{
public:
	MFDData(VOBJ * vbj)
	{
		v=vbj;
		SelectedChute=0;
	}
	MFDData()
	{
		v=NULL;
		SelectedChute=0;
	}
	int SelectedChute;
	VOBJ * v;
};
class UNVPAMFD: public MFD {
public:
	UNVPAMFD (DWORD w, DWORD h, VESSEL *vessel);
	~UNVPAMFD ();
	bool ConsumeKeyBuffered (DWORD key);
	bool ConsumeButton (int bt, int event);
	char *ButtonLabel (int bt);
	int  ButtonMenu (const MFDBUTTONMENU **menu) const;
	void Update (HDC hDC);
	int  nextChute()
	{
		if (data->SelectedChute+1 >= data->v->chutes.size())
			data->SelectedChute=0;
		else
			data->SelectedChute++;
		return data->SelectedChute;
	}
	int prevChute()
	{
		if (data->SelectedChute-1 < 0)
			data->SelectedChute = data->v->chutes.size()-1;
		else
			data->SelectedChute--;
		return data->SelectedChute;
	}
	static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);
	DWORD width, height;
	MFDData * data;
};
#endif