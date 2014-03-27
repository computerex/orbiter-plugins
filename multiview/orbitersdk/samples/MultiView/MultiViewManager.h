#ifndef _H_COMPUTEREX_FOUR_AM_PROJECT_MANAGER_H_
#define _H_COMPUTEREX_FOUR_AM_PROJECT_MANAGER_H_

#include "CVTYPE.h"
#include <orbitersdk.h>

class MULTIVIEWMANAGER
{
public:
	MULTIVIEWMANAGER()
	{
	}
	bool    hasConfig(OBJHANDLE h);
	bool    isSpacecraft(OBJHANDLE h);
	bool    isRegistered(char * typ);
	CVTYPE  Register(char * typ);
	bool    FocusIsRegistered();
	CVTYPE  GetFocus();
	void    TimeStep(void);

	std::vector<CVTYPE> profiles;
	CAM     defaultCam;
};
#endif
