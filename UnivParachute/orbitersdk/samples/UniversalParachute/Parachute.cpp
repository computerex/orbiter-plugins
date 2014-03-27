#include "Parachute.h"
#include <orbitersdk.h>

void PARACHUTE::deploy()
{
	if (state!=PACKED)
		return;
	state   = DEPLOYED;
	meshInx = VESSEL(hook).AddMesh(mesh, &ofs);
}
void PARACHUTE::jettison()
{
	if (state!=DEPLOYED)
		return;
	state = JETTISONED;
	VESSEL * v = oapiGetVesselInterface(hook);
	meshInx = v->DelMesh(meshInx, true);
	VESSELSTATUS vs; 
	v->GetStatus(vs);
	char name[255];
	sprintf(name, "%s_parachute", v->GetName());
	VECTOR3 Pos;
    oapiGetGlobalPos(v->GetHandle(), &Pos);
    v->Global2Local(Pos,Pos);
	Pos.x += ofs.x;
	Pos.y += ofs.y;
    Pos.z += ofs.z;
    v->Local2Rel(Pos,Pos);
	vs.rpos=Pos;
	vs.status = 0;
	OBJHANDLE hVes = oapiCreateVessel(name, "Parachute", vs);
	v = oapiGetVesselInterface(hVes);
	v->AddMesh(this->mesh);
	v->SetEnableFocus(false);
}
void PARACHUTE::Update()
{
	if (state!=DEPLOYED)
		return;
	VESSEL * v = oapiGetVesselInterface(hook);
	VECTOR3 airspeed;
	v->GetHorizonAirspeedVector(airspeed);
	double as = 0;
	if (type==CTYPE_PARACHUTE)
		as = airspeed.y;
	else if (type==CTYPE_DRAG)
		as = airspeed.z;
	else
		return;
	if (as<0)
		as*=-1;
	double Fd = 0.5*v->GetAtmDensity()*(as*as)*dcoef*area;
	if (type==CTYPE_PARACHUTE)
		v->AddForce(_V(0,Fd,0), ofs);
	else
		v->AddForce(_V(0,0,-Fd), ofs);
	force = Fd;
}