#ifndef _H_COMPUTEREX_ORB_REENTRY_STREAM_DNR
#define _H_COMPUTEREX_ORB_REENTRY_STREAM_DNR

#include <orbitersdk.h>
#include <vector>

class PVISUAL
{
public:
	PVISUAL() : stream(0), pos(_V(0,0,0)) {}
	PVISUAL(PARTICLESTREAMSPEC * p, VECTOR3 po) : stream(p), pos(po){}

	VECTOR3 pos;
	PARTICLESTREAMSPEC * stream;
};
#endif
