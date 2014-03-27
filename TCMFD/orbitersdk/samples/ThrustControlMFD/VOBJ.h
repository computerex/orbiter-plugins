#ifndef _H_COMPUTEREX_THRUST_CONTROL_VOBJ_H_
#define _H_COMPUTEREX_THRUST_CONTROL_VOBJ_H_

#include <orbitersdk.h>
#include <vector>
#define  AP_VEL 0
#define  AP_HOV 1
#define  NUM_OF_AP 2

void    Tokenize(const std::string& str, std::vector<std::string>& tokens,
			     const std::string& delimiters = " ");
VECTOR3 RotateVectorY(const VECTOR3 &v, double angle);
VECTOR3 RotateVectorX(const VECTOR3 &v, double angle);
VECTOR3 RotateVectorZ(const VECTOR3 &v, double angle);
bool    VectorInputRef (void *id, char *str, void *user_data);
bool    VectorInputDir (void *id, char *str, void *user_data);
bool    ResetThrustRating (void*id, char* str, void *data);
bool    SetISP (void*id, char * str, void *data);
bool    SetThrusterLevel(void*id, char * str, void * data);
VECTOR3 nrmvec3(VECTOR3 vec);

class VAP
{
public:
	VAP(int thg, double sp, bool active)
	{
		this->thg=thg;
		this->sp=sp;
		this->active=active;
	}
	VAP()
	{
		sp=0;
		active=false;
		thg=THGROUP_MAIN;
	}
	double sp;
	int    thg;
	bool   active;
};
class VOBJ

{
public:
	VOBJ(OBJHANDLE hook)
	{
		this->hook = hook;
		iTarget=0;
		GimbleGroup=THGROUP_MAIN;
		gimble=false;
	}
	VOBJ()
	{
		iTarget=0;
		GimbleGroup=THGROUP_MAIN;
		gimble=false;
	}
	int nextThruster()
	{
		VESSEL * v = oapiGetVesselInterface(hook);
		if (iTarget+1 >= v->GetThrusterCount())
			iTarget=0;
		else
			iTarget++;
		return iTarget;
	}
	int prevThruster()
	{
		VESSEL * v = oapiGetVesselInterface(hook);
		if (iTarget-1 < 0)
			iTarget = v->GetThrusterCount()-1;
		else
			iTarget--;
		return iTarget;
	}
	OBJHANDLE getHookHandle()
	{
		return hook;
	}
	int       getTargetThruster()
	{
		return iTarget;
	}
	THRUSTER_HANDLE getTargetHandle()
	{
		VESSEL * v = oapiGetVesselInterface(hook);
		return v->GetThrusterHandleByIndex(iTarget);
	}
	VESSEL * getHook()
	{
		VESSEL * v = oapiGetVesselInterface(hook);
		return v;
	}
	void resetRef(VECTOR3 vec)
	{
		getHook()->SetThrusterRef(getTargetHandle(), vec);
	}
	void resetDir(VECTOR3 vec)
	{
		double length = sqrt(vec.x*vec.x+vec.y*vec.y+vec.z*vec.z);
		vec /= length;
		getHook()->SetThrusterDir(getTargetHandle(), vec);
	}
	void resetRating(double rating)
	{
		getHook()->SetThrusterMax0(getTargetHandle(), rating);
	}
	void resetISP(double isp)
	{
		getHook()->SetThrusterIsp(getTargetHandle(), isp);
	}
	void resetLevel(double lvl)
	{
		getHook()->SetThrusterLevel(getTargetHandle(), lvl);
	}
	void TimeStep();
	bool gimbling() { return gimble; }
	void toggleGimbling() { if (gimble) gimble=false; else gimble = true; }
	int  gimbleGroup() { return GimbleGroup; }
	void setGimbleGroup(int i) { GimbleGroup = i; }
	VAP  ap[NUM_OF_AP];
private:
	OBJHANDLE hook;
	int       iTarget;
	bool      gimble;
	int       GimbleGroup;
};
#endif

