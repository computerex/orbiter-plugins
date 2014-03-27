#ifndef _H_FLYBYMFD_H
#define _H_FLYBYMFD_H

#include <orbitersdk.h>
#include <vector>
#include <string>

#define CAM_FLYBY  0
#define CAM_NORMAL 1
class MFDData
{
public:
	MFDData(OBJHANDLE hV, int hC, VECTOR3 CPos, VECTOR3 CSpeed, BOOL trk) : 
	     hVessel(hV), CamRelV(hC), CamPos(CPos), CamSpeed(CSpeed), Tracking(trk){};
	MFDData() : hVessel(0), CamRelV(0)
	{
		CamPos    = _V(0,0,0);
		CamSpeed  = _V(0,0,0);
		Tracking  = FALSE;
	}
	OBJHANDLE  hVessel;
	int        CamRelV;
	VECTOR3    CamPos;
	VECTOR3    CamSpeed;
	BOOL       Tracking;
};

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

class FlyByMFD: public MFD
{
public:

	FlyByMFD(DWORD w, DWORD h, VESSEL * vessel);
	~FlyByMFD();
	bool ConsumeKeyBuffered(DWORD Key);
	void ActivateFlyBy(void);
	void Revert(void);
	void Update(HDC hDC);
	bool ConsumeButton (int bt, int event);
	char *ButtonLabel (int bt);
	static int MsgProc (UINT msg, UINT mfd, WPARAM wParam, LPARAM lParam);
	MFDData * data;
};
#endif