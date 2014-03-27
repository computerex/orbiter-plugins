#ifndef _H_COMPUTEREX_EXPERIMENTING_WITH_PHYSICS_H_
#define _H_COMPUTEREX_EXPERIMENTING_WITH_PHYSICS_H_

#include <orbitersdk.h>
#include <vector>

class VOBJ
{
public:
	VOBJ()
	{
		hook=0;
		dThresh=0.2;
		dAngThresh=0.5;
	}
	VOBJ(OBJHANDLE hVessel, double thr, double athresh)
	{
		hook = hVessel;
		dThresh=thr;
		dAngThresh=athresh;
	}
	void   TimeStep(void)
	{
		if (!oapiIsVessel(hook))
			return;
		OBJHANDLE hVes = CheckCollision();
		if (oapiIsVessel(hVes))
			Collide(hVes);
	}
	VECTOR3 Acceleration(OBJHANDLE hVes)
	{
		VESSEL * vessel = oapiGetVesselInterface(hVes);
		VECTOR3  fv, wv;
		vessel->GetForceVector(fv);
		vessel->GetWeightVector(wv);
		fv-=wv;
		double  mass = oapiGetMass(hVes);
		return _V(fv.x/mass, fv.y/mass, fv.z/mass);
	}
	void   Collide(OBJHANDLE hVes)
	{
		VECTOR3 rvel, rpos;
		VESSELSTATUS vs;
		double size1, size2, pos, vel;
		oapiGetRelativeVel(hook, hVes, &rvel);
	    vel = length(rvel);
		oapiGetRelativePos(hook, hVes, &rpos);
	    pos = length(rpos);
		
		VESSEL * vessel = oapiGetVesselInterface(hook);
		vessel->GetStatus(vs);
	    size2 = oapiGetSize(hVes);
		size1   = oapiGetSize(hook);
		vs.rvel.x -= rvel.x*1.25;
	    vs.rvel.y -= rvel.y*1.25;
	    vs.rvel.z -= rvel.z*1.25;
		//vs.rpos.x += rpos.x * ((size2 + size1)/2 - pos)/pos;
	    //vs.rpos.y += rpos.y * ((size2 + size1)/2 - pos)/pos;
	    //vs.rpos.z += rpos.z * ((size2 + size1)/2 - pos)/pos;
		double a = rpos.x*((size2+size1)/2-pos);
		double b = rpos.y*((size2+size1)/2-pos);
		double c = rpos.z*((size2+size1)/2-pos);
		vs.rpos.x+=a/pos;
		vs.rpos.y+=b/pos;
		vs.rpos.z+=c/pos;
		vs.vrot.x =vel*50/(oapiGetMass(hook)/4);
	    vs.vrot.y =vel*50/(oapiGetMass(hook)/4);
	    vs.vrot.z =vel*50/(oapiGetMass(hook)/4);
		vessel->DefSetState(&vs);
	}
	OBJHANDLE   CheckCollision(void)
	{
		if (!oapiIsVessel(hook))
			return NULL;
		OBJHANDLE unfVessel=NULL;
		double    dist = DistClosestVessel(unfVessel);
		if (!oapiIsVessel(unfVessel))
			return unfVessel;
		VECTOR3 rvel;
		oapiGetRelativeVel(unfVessel, hook, &rvel);
		double vel = length(rvel);
		VECTOR3 ang;
		VESSEL(hook).GetAngularVel(ang);
		if (vel < dThresh && length(ang) < dAngThresh)
			return NULL;
		double size1  = oapiGetSize(hook);
		double size2  = oapiGetSize(unfVessel);
		if (dist < (size1+size2)/2)
			return unfVessel;
		else
			return NULL;
	}
	double DistClosestVessel(OBJHANDLE & hVessel)
	{
		if (!oapiIsVessel(hook))
			return 0;
		DWORD  inx  = oapiGetVesselCount();
		hVessel=NULL;
		if (inx<2)
			return 0;
		double dist = 0;
		OBJHANDLE * hObjs = new OBJHANDLE[inx-1];
		for (int i = 0, x = 0; i < inx; i++)
		{
			hVessel = oapiGetVesselByIndex(i);
			if (hVessel == hook)
				continue;
			hObjs[x]=hVessel;
			x++;
		}
		VECTOR3 rpos;
		VESSEL  * vessel = oapiGetVesselInterface(hook);
		OBJHANDLE retVessel=NULL;
		for (int i = 0; i < inx-1; i++)
		{
			hVessel=hObjs[i];
			vessel->GetRelativePos(hVessel, rpos);
			if (i==0){
				dist=length(rpos);
				retVessel=hVessel;
				continue;
			}
			if (dist > length(rpos)){
				retVessel=hVessel;
				dist=length(rpos);
			}
		}
		hVessel=retVessel;
		delete [] hObjs;
		return dist;
	}
	OBJHANDLE hook;
	double    dThresh, dAngThresh;
};
#endif