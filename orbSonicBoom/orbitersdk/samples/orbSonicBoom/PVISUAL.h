#ifndef _H_VISUA_PARTICLE_SONIC_BOOM_COMPUTEREX_POWER
#define _H_VISUA_PARTICLE_SONIC_BOOM_COMPUTEREX_POWER

#include <orbitersdk.h>

class PVISUAL
{
public:
	PVISUAL(PARTICLESTREAMSPEC *ps, VECTOR3 p) : stream(ps), pos(p) {}
	PVISUAL() :  pos(_V(0,0,0)){}
	PARTICLESTREAMSPEC *stream;
	VECTOR3 pos;
};
#endif