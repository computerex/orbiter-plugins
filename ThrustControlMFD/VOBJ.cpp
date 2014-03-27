#include "VOBJ.h"

void VOBJ::TimeStep()
{
	if (gimble)
	{
		VESSEL * vessel = getHook();
		VECTOR3 dir = _V(0,0,1);
		OBJHANDLE thruster;
		double cLevel = vessel->GetControlSurfaceLevel(AIRCTRL_ELEVATOR);
		double dLevel = vessel->GetControlSurfaceLevel(AIRCTRL_RUDDER);
		if (GimbleGroup==THGROUP_HOVER)
		{
			dir = _V(0,1,0);
			dir = RotateVectorZ(dir, dLevel*90);
			dir = RotateVectorX(dir, -cLevel*90);
			dir.y*=-1;
		}else if (GimbleGroup==THGROUP_MAIN)
		{
			dir = RotateVectorX(dir,-cLevel*90);
			dir = RotateVectorY(dir,dLevel*90);
		}
			dir  = nrmvec3(dir);
		for (DWORD i = 0; i < vessel->GetGroupThrusterCount((THGROUP_TYPE)GimbleGroup); i++)
		{
			thruster = vessel->GetGroupThruster((THGROUP_TYPE)GimbleGroup, i);
			vessel->SetThrusterDir(thruster,dir);
        }
	}
	VESSEL * vessel = getHook();
	double dt = oapiGetSimStep();
	if (ap[AP_HOV].active)
	{
		DWORD inx = vessel->GetGroupThrusterCount((THGROUP_TYPE)ap[AP_HOV].thg);
		VECTOR3 fv, tv;
	    vessel->GetForceVector(fv);
	    vessel->GetThrustVector(tv);
	    THRUSTER_HANDLE th;
	    fv-=tv;
	    VECTOR3 hVel;
	    vessel->GetHorizonAirspeedVector(hVel);
		double e = hVel.y-ap[AP_HOV].sp;
	    double lvl;
	    for (DWORD i = 0; i < inx; i++)
     	{
			th = vessel->GetGroupThruster((THGROUP_TYPE)ap[AP_HOV].thg, i);
			lvl = (-fv.y-e*vessel->GetMass()*70)/vessel->GetThrusterMax0(th);
			double b0 = fabs(vessel->GetBank()*(180/PI));
			if (ap[AP_HOV].thg==THGROUP_MAIN)
			{
				b0 = vessel->GetPitch()*(180/PI);
				if (lvl < 0)
					if (b0 >= 0)
						lvl=0;
					else
						if (lvl<-1)
							lvl=1;
						else
							lvl = fabs(lvl);
				else
					if (b0 <= 0)
						lvl = 0;
					else
						if (lvl>1)
							lvl = 1;
			}
			else
			{
				if (lvl<0)
					if (b0 <= 90)
						lvl=0;
					else
						if (lvl<-1)
							lvl=1;
						else
							lvl = fabs(lvl);
				else 
					if (b0 >= 90)
						lvl = 0;
					else
						if (lvl > 1)
							lvl = 1;
			}
			vessel->SetThrusterLevel(th, lvl);
		}
	}
	if (ap[AP_VEL].active)
	{
		DWORD inx = vessel->GetGroupThrusterCount((THGROUP_TYPE)ap[AP_VEL].thg);
		VECTOR3 dv;
		vessel->GetDragVector(dv);
		THRUSTER_HANDLE th;
		double e = ap[AP_VEL].sp-vessel->GetAirspeed();
		double lvl = 0;
		for (DWORD i = 0; i < inx; i++)
		{
			th = vessel->GetGroupThruster((THGROUP_TYPE)ap[AP_VEL].thg, i);
			lvl = (-dv.z+e*vessel->GetMass()*70)/vessel->GetThrusterMax0(th);
			if (lvl<0)
				lvl=0;
			else if (lvl>1)
				lvl=1;
			vessel->SetThrusterLevel(th, lvl);
		}
	}
}
bool VectorInputRef (void *id, char *str, void *user_data)
{
	VOBJ * v = (VOBJ*)user_data;
	VECTOR3 ref = _V(0,0,1);
	std::string svector = str;
	std::vector<std::string> tokens;
	Tokenize(svector,tokens);
	if (tokens.size() < 3)
		return true;
	ref.x=atof(tokens[0].c_str());
	ref.y=atof(tokens[1].c_str());
	ref.z=atof(tokens[2].c_str());
	v->resetRef(ref);;
	return true;
}
bool VectorInputDir (void *id, char *str, void *user_data)
{
	VOBJ * v = (VOBJ*)user_data;
	VECTOR3 dir = _V(0,0,1);
	std::string svector = str;
	std::vector<std::string> tokens;
	Tokenize(svector,tokens);
	if (tokens.size() < 3)
		return true;
	dir.x=atof(tokens[0].c_str());
	dir.y=atof(tokens[1].c_str());
	dir.z=atof(tokens[2].c_str());
	v->resetDir(dir);;
	return true;
}
bool ResetThrustRating (void*id, char* str, void *data)
{
	VOBJ * v = (VOBJ*)data;
	if (strlen(str)==0)
		return false;
	v->resetRating(atof(str));
	return true;
}
bool SetISP (void*id, char * str, void *data)
{
	VOBJ * v = (VOBJ*)data;
	if (strlen(str)==0)
		return false;
	v->resetISP(atof(str));
	return true;
}
bool SetThrusterLevel(void*id, char * str, void * data)
{
	VOBJ * v = (VOBJ*)data;
	if (strlen(str)==0)
		return false;
	double lvl = atof(str);
	if (lvl > 100)
		lvl = 100;
	else if (lvl < 0)
		lvl = 0;
	lvl /= 100;
	v->resetLevel(lvl);
	return true;
}
void Tokenize(const std::string& str,
			  std::vector<std::string>& tokens,
			  const std::string& delimiters)
{
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
    {
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}
VECTOR3 RotateVectorY(const VECTOR3 &v, double angle)
{
    VECTOR3 Output;
    Output.y=v.y;
    Output.x=v.x*cos(angle*RAD)-v.z*sin(angle*RAD);
    Output.z=v.z*cos(angle*RAD)+v.x*sin(angle*RAD);
    return Output;
}

VECTOR3 RotateVectorX(const VECTOR3 &v, double angle)
{
    VECTOR3 Output;
    Output.x=v.x;
    Output.z=v.z*cos(angle*RAD)-v.y*sin(angle*RAD);
    Output.y=v.y*cos(angle*RAD)+v.z*sin(angle*RAD);
    return Output;
}
VECTOR3 RotateVectorZ(const VECTOR3 &v, double angle)
{
    VECTOR3 Output;
    Output.z=v.z;
    Output.x=v.x*cos(angle*RAD)+v.y*sin(angle*RAD);
    Output.y=v.x*sin(angle*RAD)-v.y*cos(angle*RAD);
    return Output;
} 
VECTOR3 nrmvec3(VECTOR3 vec)
{
	double len = length(vec);
	return _V(vec.x/len, vec.y/len, vec.z/len);
}