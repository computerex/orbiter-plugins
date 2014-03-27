#include "REMT.h"
#include <orbitersdk.h>
#include "MfdSoundSDK35.h"
#include "TCMFD.h"

void RTHRUST::TimeStep()
{
	if ((!CountdownStarted && !burning) || !oapiIsVessel(hook)){
		return;
	}
	VESSEL * v = oapiGetVesselInterface(hook);
	VOBJ * vbj = data->v;
	double simt = oapiGetSimTime();
	double tto = CountdownStartTime-simt;
	int thg = vbj->ap[data->SelectedAP].thg;
	if (tto < 10 && tto > 9.9)
	{
		PlayMFDWave(id, 0);
	}
	else
	{
		if (tto<0.1 && tto > 0 && !burning) // Start burn
		{
			burning=true;
			CountdownStarted=false;
			Countdown=0;
			BurnStartTime=simt+BurnDuration;
			v->SetThrusterGroupLevel((THGROUP_TYPE)thg, 1);
			//if (oapiGetFocusInterface()->GetHandle() == hook)
				//StopMFDWave(id, 0);
		}
		else
		{
			if (simt > BurnStartTime && burning) // ending the burn
			{
				burning=false;
				BurnDuration=0;
				v->SetThrusterGroupLevel((THGROUP_TYPE)thg, 0);
			}
		}
	}
	if (burning)
	{
		if (BurnDuration<20) 
			oapiSetTimeAcceleration(1);
		v->SetThrusterGroupLevel((THGROUP_TYPE)thg, 1);
	}
}
void RTHRUST::Abort()
{
	CountdownStarted=false;
	burning=false;
	CountdownStartTime=0;
	Countdown=0;
	BurnDuration=0;
	if (oapiGetFocusInterface()->GetHandle()==hook)
		StopMFDWave(id, 0);
	int thg = data->v->ap[data->SelectedAP].thg;
	data->v->getHook()->SetThrusterGroupLevel((THGROUP_TYPE)thg, 0);
}