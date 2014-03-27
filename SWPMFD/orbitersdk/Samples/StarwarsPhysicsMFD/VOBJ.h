#ifndef _H_VOBJ_STAR_WARS_PHYSICS_MFD_THREE_AM_COMPUTEREX_H_
#define _H_VOBJ_STAR_WARS_PHYSICS_MFD_THREE_AM_COMPUTEREX_H_

#include <orbitersdk.h>

class VOBJ
{
public:
	VOBJ(OBJHANDLE h)
	{
		hook     = h;
		wfac     = 10;
		active   = FALSE;
		constant = FALSE;
		range=3;
		velocity = 300;
		gravity  = true;
	}
	VOBJ()
	{
		hook=NULL;
		wfac=10;
		active=FALSE;
		constant=FALSE;
		range=3;
		velocity=300;
		gravity = true;
	}
	void Physics()
	{
		if (!oapiIsVessel(hook))
			return;
		VESSEL * v = oapiGetVesselInterface(hook);
		if (!gravity)
		{
			VECTOR3 frc;
			v->GetWeightVector(frc);
			v->AddForce(-frc, _V(0,0,0));
		}
		if (oapiGetTimeAcceleration() > 10)
			return;
		if (!active)
			return;
		VECTOR3 spdShipVector;
		v->GetShipAirspeedVector(spdShipVector);
		VECTOR3 ForceVectorNewton;
		VECTOR3 RadiusVectorMeter;
		int RadiusVectorMultiplier = v->GetMass();
		ForceVectorNewton.x = 0;
        RadiusVectorMeter.x = 0;
        ForceVectorNewton.y = 0;
        RadiusVectorMeter.y = 0;
        ForceVectorNewton.z = 0;
        RadiusVectorMeter.z = 0;
		if (v->GetAltitude() < 100)
        {
			ForceVectorNewton.x = 0;
            RadiusVectorMeter.x = 0;
            ForceVectorNewton.z = 0;
            RadiusVectorMeter.z = 0;
            ForceVectorNewton.y = 0;
            RadiusVectorMeter.y = 0;
        }
        else
        {
			if (spdShipVector.x != 0)
			{
				RadiusVectorMeter.x = ((spdShipVector.x - spdShipVector.x - spdShipVector.x) * 10);
                ForceVectorNewton.x = ((RadiusVectorMeter.x / 10) * RadiusVectorMultiplier);
            }
			if (spdShipVector.y != 0)
            {
				RadiusVectorMeter.y = ((spdShipVector.y - spdShipVector.y - spdShipVector.y) * 10);
                ForceVectorNewton.y = ((RadiusVectorMeter.y / 10) * RadiusVectorMultiplier);
			}
		}
		v->AddForce(ForceVectorNewton, RadiusVectorMeter);
	}
	void Propogate()
	{
		if (!oapiIsVessel(hook))
			return;
		VESSEL * v = oapiGetVesselInterface(hook);
		VESSELSTATUS vs;
		v->GetStatus(vs);
		VECTOR3 rofs;
		v->GlobalRot(_V(0,0,wfac),rofs);
		vs.rvel.x+=rofs.x;
		vs.rvel.y+=rofs.y;
		vs.rvel.z+=rofs.z;
		vs.status = 0;
		v->DefSetState(&vs);
	}
	double wfac;
	OBJHANDLE hook;
	BOOL active;
	BOOL constant;
	double range;
	double velocity;
	bool   gravity;
};
#endif
