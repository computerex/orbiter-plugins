#include "VOBJ.h"

int  rint(int from, int to)
{
	return rand() % to+from;
}
void VOBJ::UpdateIVState()
{
	if (state.dead==TRUE)
		return;
	VESSEL * v = oapiGetVesselInterface(hook);
	VECTOR3 W, F, A, V;
	v->GetForceVector(F);
	v->GetWeightVector(W);
	A=(F-W)/v->GetMass();
	v->GetHorizonAirspeedVector(V);
	ivstate.Accel.input(A.z);
	ivstate.Gs.input(A.y/9.8);
	ivstate.gContact=v->GroundContact();
	ivstate.Pressure=v->GetDynPressure();
	ivstate.Vel=v->GetAirspeed();
	ivstate.Altitude=v->GetAltitude();
	ivstate.hVel=V.y;
	ivstate.Flux=0.5*(ivstate.Vel*ivstate.Vel*ivstate.Vel*v->GetAtmDensity());
	ivstate.Mass=v->GetMass();
	ivstate.Pitch=v->GetPitch();
	ivstate.Bank=v->GetBank();
}
void VOBJ::PreStep(HUDOPTIONS hudop)
{
	if (!oapiIsVessel(hook) || state.dead==TRUE){
		PrintMessages(hudop);
		return;
	}
	if (Init==FALSE)
	{
		Init=TRUE;
		VESSEL * v = oapiGetVesselInterface(hook);
		if (v->GroundContact())
			ivstate.defaultAltitude=v->GetAltitude();
	}
	PreFlagCheck();
	CheckDamage();
	PrintMessages(hudop);
}
void VOBJ::PostStep()
{
	if (!oapiIsVessel(hook) || state.dead){
		PaintDamage();
		return;
	}
	PostFlagCheck();
	LastState=ivstate;
	UpdateIVState();
	PaintDamage();
}

void VOBJ::PaintDamage()
{
	VESSEL * v = oapiGetVesselInterface(hook);
	if (misc.effect || misc.fire)
	{
		if (oapiGetTimeAcceleration()>1){
			if (misc.effect)
				v->SetThrusterLevel(misc.effect,0);
			if (misc.fire)
				v->SetThrusterLevel(misc.fire,0);
		}
		else{
			if (misc.effect)
				v->SetThrusterLevel(misc.effect,1);
			if (misc.fire)
				v->SetThrusterLevel(misc.fire,1);
		}
	}
	if (state.dead==TRUE)
		return;
	if (state.Accel==TRUE)
	{
		DamageStructural();
		if (!misc.effect)
			misc.effect=PaintSmoke();
	}
	if (state.Collision==TRUE){
		misc.fire=PaintFire();
		state.dead=TRUE;
	}
	if (state.Flux==TRUE)
	{
		v->ClearMeshes();
		state.dead=TRUE;
	}
	if (state.maxG==TRUE)
	{
		v->ClearMeshes();
		state.dead=TRUE;
	}
	if (state.minG==TRUE)
	{
		v->ClearMeshes();
		state.dead=TRUE;
	}
	if (state.Pressure)
	{
		state.dead=TRUE;
		if (!misc.effect)
			misc.effect=PaintSmoke();
	}
	if (state.Vel)
	{
		if (state.Accel==FALSE)
			DamageStructural();
		if (!misc.effect)
			misc.effect=PaintSmoke();
	}
}
void VOBJ::CheckDamage()
{
	char cbuf[255];
	if (state.Accel==FALSE)
	{
		if (ivstate.flags.Accel==TRUE)
		{
			if (oapiGetSimTime()-ivstate.flags.time.Accel > DAMAGE_DELAY)
			{
				state.Accel=TRUE;
				sprintf(cbuf,"Acceleration excess, spacecraft destroyed! %.2f m/s", ivstate.Accel.avg());
				hudp.insert(cbuf);
				return;
			}
		}
	}
	if (state.Collision==FALSE)
	{
		if (ivstate.gContact)
		{
			if (ivstate.hVel < -limit.maxCollision)
			{
				state.Collision=TRUE;
				sprintf(cbuf,"Collision at: %.2f m/s Spacecraft destroyed!",ivstate.hVel);
				hudp.insert(cbuf);
				DisableSpacecraft();
				return;
			}
		}
	}
	if (state.Flux==FALSE)
	{
		if (ivstate.flags.Flux==TRUE)
		{
			if (oapiGetSimTime()-ivstate.flags.time.Flux > DAMAGE_DELAY)
			{
				state.Flux=TRUE;
				sprintf(cbuf,"Spacecraft got disintegrated! HF %.2f", ivstate.Flux);
				hudp.insert(cbuf);
				DisableSpacecraft();
				return;
			}
		}
	}
	if (state.maxG==FALSE)
	{
		if (ivstate.flags.maxG==TRUE)
		{
			if (oapiGetSimTime()-ivstate.flags.time.maxG > DAMAGE_DELAY)
			{
				state.maxG=TRUE;
				sprintf(cbuf,"G stress! Spacecraft destroyed! %.2f",ivstate.Gs.avg());
				hudp.insert(cbuf);
				DisableSpacecraft();
				return;
			}
		}
	}
	if (state.minG==FALSE)
	{
		if (ivstate.flags.minG==TRUE)
		{
			if (oapiGetSimTime()-ivstate.flags.time.minG > DAMAGE_DELAY)
			{
				state.minG=TRUE;
				sprintf(cbuf,"G stress! Spacecraft destroyed! %.2f",ivstate.Gs.avg());
				hudp.insert(cbuf);
				DisableSpacecraft();
				return;
			}
		}
	}
	if (state.Pressure==FALSE)
	{
		if (ivstate.flags.Pressure==TRUE)
		{
			if (oapiGetSimTime()-ivstate.flags.time.Pressure > DAMAGE_DELAY)
			{
				state.Pressure=TRUE;
				sprintf(cbuf,"Dynamic pressure too high! Spacecraft crushed! %.2f PA", ivstate.Pressure);
				hudp.insert(cbuf);
				DisableSpacecraft();
				return;
			}
		}
	}
	if (state.Vel==FALSE)
	{
		if (ivstate.gContact && fabs(ivstate.Vel) > limit.maxVel)
		{
			state.Vel=TRUE;
		    sprintf(cbuf,"Ground airspeed too high! Spacecraft damaged! %.2f m/s", ivstate.Vel);
		    hudp.insert(cbuf);
		    return;
		}
	}
	if (GetKeyState(VK_ESCAPE) & 0x800)
		hudp.clear();
}
void VOBJ::PostFlagCheck()
{
	if (state.Accel==FALSE)
	{
		if (fabs(ivstate.Accel.avg()) < limit.maxAccel)
		{
			ivstate.flags.Accel=FALSE;
		}
	}
	if (state.Flux==FALSE)
	{
		if (ivstate.Flux < limit.maxFlux)
		{
			ivstate.flags.Flux=FALSE;
		}
	}
	if (state.maxG==FALSE)
	{
		if (ivstate.Gs.avg() < limit.maxG)
		{
			ivstate.flags.maxG=FALSE;
		}
	}
	if (state.minG==FALSE)
	{
		if (ivstate.Gs.avg() > limit.minG)
		{
			ivstate.flags.minG=FALSE;
		}
	}
	if (state.Pressure==FALSE)
	{
		if (ivstate.Pressure < limit.maxPressure)
		{
			ivstate.flags.Pressure=FALSE;
		}
	}
}
void VOBJ::PreFlagCheck()
{
	if (state.Accel==FALSE)
	{
		if (fabs(ivstate.Accel.avg()) > limit.maxAccel && ivstate.flags.Accel==FALSE)
		{
			ivstate.flags.Accel=TRUE;
			ivstate.flags.time.Accel=oapiGetSimTime();
			hudp.insert("Warning! Acceleration too high!");
		}
	}
	if (state.Flux==FALSE)
	{
		if (ivstate.Flux > limit.maxFlux && ivstate.flags.Flux==FALSE)
		{
			ivstate.flags.Flux=TRUE;
			ivstate.flags.time.Flux=oapiGetSimTime();
			hudp.insert("Warning! Spacecraft temperature too high!");
		}
	}
	if (state.maxG==FALSE)
	{
		if (ivstate.Gs.avg() > limit.maxG && ivstate.flags.maxG==FALSE)
		{
			ivstate.flags.maxG=TRUE;
			ivstate.flags.time.maxG=oapiGetSimTime();
			hudp.insert("Warning! G excess!");
		}
	}
	if (state.minG==FALSE)
	{
		if (ivstate.Gs.avg() < limit.minG && ivstate.flags.minG==FALSE)
		{
			ivstate.flags.minG=TRUE;
			ivstate.flags.time.minG=oapiGetSimTime();
			hudp.insert("Warning! G Stress!");
		}
	}
	if (state.Pressure==FALSE)
	{
		if (ivstate.Pressure > limit.maxPressure && ivstate.flags.Pressure==FALSE)
		{
			ivstate.flags.Pressure=TRUE;
			ivstate.flags.time.Pressure=oapiGetSimTime();
			hudp.insert("Warning! Dynamic pressure too high!");
		}
	}
}
void VOBJ::PrintMessages(HUDOPTIONS hudop)
{
	char cbuf[255];
	double yofs = hudop.starty, xofs = hudop.startx;
	for (int i = 0; i < hudp.m.size(); i++)
	{
		sprintf(cbuf,"%s",hudp.m[i].c_str());
		HUD.Add(0.001,hook,HUD._D(xofs,1,yofs,1,hudop.size,hudop.color,cbuf));
		yofs+=hudop.yofs;
		xofs+=hudop.xofs;
	}
	HUD.TimeStep();
}
void VOBJ::DisableSpacecraft()
{
	VESSEL * v = oapiGetVesselInterface(hook);
	v->ClearAirfoilDefinitions();
	v->ClearThrusterDefinitions();
	v->ClearPropellantResources();
	v->SetWheelbrakeLevel(1,0,true);
}
THRUSTER_HANDLE VOBJ::PaintSmoke()
{
	VESSEL * v = oapiGetVesselInterface(hook);
	PARTICLESTREAMSPEC rps = {
		0, v->GetSize()*2, 8, 200, 0.15, 1.0, 5, 3.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 2,
		PARTICLESTREAMSPEC::ATM_PLOG, 1e-4, 1
	};
	rps.atmslowdown=1000;
	PROPELLANT_HANDLE hpr = v->CreatePropellantResource(1);
	THRUSTER_HANDLE th = v->CreateThruster(_V(0,0,0),_V(0,0,-1),0e1, hpr, 5000e300);
	v->AddExhaustStream(th, _V(0,0,0), &rps);
	v->SetThrusterLevel(th, 1);
	return th;
}
THRUSTER_HANDLE VOBJ::PaintFire()
{
	VESSEL * v = oapiGetVesselInterface(hook);
	VESSELSTATUS vs;
	v->GetStatus(vs);
	vs.vrot=_V(0,0,0);
	v->DefSetState(&vs);
	SURFHANDLE hFire = oapiRegisterExhaustTexture("DamFire");
	PARTICLESTREAMSPEC rps = {
		0, v->GetSize()*2, 33, 33, 0.11, 0.33, 1.9, 1.0, PARTICLESTREAMSPEC::EMISSIVE,
		PARTICLESTREAMSPEC::LVL_SQRT, 0, 1,
		PARTICLESTREAMSPEC::ATM_PLOG, 1e-5, 0.1,hFire
	};
	rps.lifetime=5;
	rps.srcspread=0;
	rps.atmslowdown=0;
	VECTOR3 arot;
	v->GetGlobalOrientation(arot);
	THRUSTER_HANDLE thRet;
	PROPELLANT_HANDLE hpr = v->CreatePropellantResource(1);
	THRUSTER_HANDLE th = v->CreateThruster(_V(0,0,0),_V(0,-1,0),0e100, hpr, 5000e300);
	v->AddExhaustStream(th, _V(0,v->GetSize(),0),&rps);
	v->SetThrusterLevel(th,1);
	thRet=th;
	hpr = v->CreatePropellantResource(1);
	th = v->CreateThruster(_V(0,0,0),_V(0,-1,0),50e1, hpr, 5e2);
	rps.srcsize=v->GetSize()*2;
	rps.atmslowdown=5;
	rps.growthrate=v->GetSize()*20;
	rps.lifetime=10;
	rps.srcrate=10;
	rps.srcspread=0;
	v->AddExhaustStream(th, _V(0,0,0),&rps);
	v->SetThrusterLevel(th,1);
	return thRet;
}
void VOBJ::DamageStructural()
{
	VESSEL * v = oapiGetVesselInterface(hook);
	int r = rint(1,4);
	if (r>2)
		r=-1;
	else
		r=1;
	if (oapiGetTimeAcceleration()<=1)
	{
		v->AddForce(_V(rint(100, v->GetMass()*10)*r,rint(100, v->GetMass()*10)*r,rint(100, v->GetMass()*10)*-1),
			_V(rint(1,v->GetSize())*r, rint(1,v->GetSize())*r, rint(1,v->GetSize())*r));
	}
	v->SetWheelbrakeLevel(1,true);
}