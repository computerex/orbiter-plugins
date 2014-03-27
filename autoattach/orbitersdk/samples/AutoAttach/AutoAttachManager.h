#ifndef _H_COMPUTER_EX_AUTO_ATTACH_MANAGER_H_
#define _H_COMPUTER_EX_AUTO_ATTACH_MANAGER_H_
#pragma once
#include <orbitersdk.h>

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


class AAMANAGER
{
public:
	bool        isChild(OBJHANDLE hVessel);
	bool        isChild(OBJHANDLE hVessel, ATTACHMENTHANDLE hAttach);
	bool        isParent(OBJHANDLE hVessel);
	bool        isParent(OBJHANDLE hVessel, ATTACHMENTHANDLE hAttach);
	bool        isAttached(OBJHANDLE hVessel);
	bool        isAttachmentBusy(OBJHANDLE hVessel, DWORD attachment, bool toparent);
	int         hasAttachments(OBJHANDLE hVessel);
	OBJHANDLE   AttachedTo(OBJHANDLE hVessel, int attachment, bool toparent);
	OBJHANDLE   getClosestVessel(OBJHANDLE hVessel);
	OBJHANDLE   FindMatch(OBJHANDLE hVessel, ATTACHMENTHANDLE * hvAtt, ATTACHMENTHANDLE * htAtt, double * distance);
	OBJHANDLE   FindAttachmentParent(ATTACHMENTHANDLE hAttach);
	double      getDistance(OBJHANDLE hV1, OBJHANDLE hV2);
	double      getAttachmentDistance(OBJHANDLE hV1, OBJHANDLE hV2, ATTACHMENTHANDLE attachment1, ATTACHMENTHANDLE attachment2);
	double      GetClosestAttachment(OBJHANDLE hVessel, ATTACHMENTHANDLE hAttach, ATTACHMENTSPEC * aspec, bool toparent);
	void        TimeStep();
	void        ProcessVessel(OBJHANDLE hVessel);
};
#endif
