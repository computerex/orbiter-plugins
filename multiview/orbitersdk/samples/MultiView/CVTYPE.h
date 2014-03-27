#ifndef _H_MULTI_VIEW_COMPUTER_EX_FOUR_AM_PROJECT_H_
#define _H_MULTI_VIEW_COMPUTER_EX_FOUR_AM_PROJECT_H_

#define _CRT_SECURE_NO_DEPRECATE 

#include <vector>
#include <orbitersdk.h>

class CAM
{
public:
	CAM()
	{
		pos=_V(0,0,0);
		dir=_V(0,0,0);
	}
	VECTOR3 pos, dir;
};

/*
class VOBJ
{
public:
	VOBJ(std::vector<CAM> c, OBJHANDLE h)
	{
		cameras = c;
		hook = h;
	}
	VOBJ()
	{
		cameras.clear();
		hook = NULL;
	}
	std::vector<CAM> cameras;
	OBJHANDLE hook;
};
*/
class CVTYPE
{
public:
	CVTYPE(char * typ, std::vector<CAM> vec)
	{
		strcpy(type,typ);
		cameras=vec;
	}
	CVTYPE()
	{
		strcpy(type,"");
		cameras.clear();
	}
	std::vector<CAM> cameras;
	char type[255];
};
#endif
