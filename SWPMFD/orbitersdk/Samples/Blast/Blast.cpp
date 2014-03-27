#define STRICT
#define ORBITER_MODULE

#include "Blast.h"
#include <vector>

VECTOR3 negv(VECTOR3 a)
{
	return _V(-a.x,-a.y,-a.z);
}
VECTOR3 norv(VECTOR3 &a)
{
	double l = length(a);
	return _V(a.x/l, a.y/l, a.z/l);
}
BLAST::BLAST(OBJHANDLE hVessel, int flightmodel) : VESSEL2(hVessel, flightmodel)
{
	m_DRange=1; 
	m_Blown =false;
	m_CreationTime=oapiGetSimTime();
	m_ExplosionTime=m_DRange;
}
int BLAST::clbkConsumeBufferedKey(DWORD key, bool down, char *kstate)
{
	if (!down) return 0;

	if (key==EVENT_RANGE_SET)
	{
		m_DRange = *(double*)kstate;
		ClearThrusterDefinitions();
		PROPELLANT_HANDLE hpr = CreatePropellantResource(1);
		THRUSTER_HANDLE    th = CreateThruster(_V(0,0,0),_V(0,1,0),1e0,hpr,500e50);
		AddExhaust(th, 1, m_DRange/2);
		CreateThrusterGroup(&th, 1, THGROUP_MAIN);
		SetThrusterGroupLevel(THGROUP_MAIN,1);
		return 1;
	}
	return 0;
}
void BLAST::clbkPreStep(double simt, double simdt, double mjd)
{
	VECTOR3 fVector;
	GetWeightVector(fVector);
	AddForce(negv(fVector),_V(0,0,0));

	if (Blow())
	{
		m_Blown=true;
		m_ExplosionTime=oapiGetSimTime();
		SetThrusterGroupLevel(THGROUP_MAIN,0);
		Damage();
	}
	if (m_Blown)
	{
		if (m_ExplosionTime+5 < oapiGetSimTime())
		{
			oapiDeleteVessel(GetHandle());
		}
	}
}
void BLAST::clbkSetClassCaps(FILEHANDLE cfg)
{
}
bool BLAST::Blow()
{
	if (m_Blown)
		return false;
	if (GroundContact())
		return true;
	if (ClosestVessel() <= m_DRange)
		return true;
	if (m_CreationTime+20 < oapiGetSimTime())
		return true;
	return false;
}
double BLAST::ClosestVessel()
{
	std::vector<double> dist;
	OBJHANDLE hVes;
	VECTOR3 pos;
	VESSEL * vessel;
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		hVes=oapiGetVesselByIndex(i);
		vessel=oapiGetVesselInterface(hVes);
		if (oapiGetFocusInterface()->GetHandle() == hVes)
			continue;
		if (vessel->GetClassName())
			if (!strcmp(vessel->GetClassName(),"Blast"))
				continue;
		GetRelativePos(hVes,pos);
		dist.push_back(length(pos));
	}
	while(true)
	{
		bool done = true;
		for (int i = 1; i < dist.size(); i++)
		{
			if (dist[i] < dist[i-1])
			{
				double temp = dist[i];
				dist[i]=dist[i-1];
				dist[i-1]=temp;
				done=false;
			}
		}
		if (done)
			break;
	}
	return dist[0];
}
void BLAST::Damage(OBJHANDLE hTarget)
{
	if (hTarget==GetHandle() || hTarget==oapiGetFocusInterface()->GetHandle())
		return;
	VESSEL * target=oapiGetVesselInterface(hTarget);
	VESSELSTATUS vs;
	target->GetStatus(vs);
	vs.status=0;
	target->DefSetState(&vs);
	VECTOR3 rpos;
	GetRelativePos(GetHandle(),rpos);
	VECTOR3 angvel;
	target->GetAngularVel(angvel);
	angvel.x+=5;
	angvel.z-=5;
	target->SetAngularVel(angvel);
}
void BLAST::Damage()
{
	double dist = 0;
	OBJHANDLE hVes=NULL;
	VECTOR3 pos;
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		hVes=oapiGetVesselByIndex(i);
		GetRelativePos(hVes, pos);
		if (length(pos) <= m_DRange)
			Damage(hVes);
	}
}
DLLCLBK VESSEL *ovcInit (OBJHANDLE hvessel, int flightmodel)
{
	return new BLAST (hvessel, flightmodel);
}
DLLCLBK void ovcExit (VESSEL *vessel)
{
	if (vessel) delete (BLAST*)vessel;
}