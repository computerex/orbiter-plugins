#ifndef  _H_COMPUTEREX_DAMAGE_MODULE_REDONE_H_
#define  _H_COMPUTEREX_DAMAGE_MODULE_REDONE_H_
#define  DAMAGE_DELAY 3 // seconds
#define  SAMPLING_STEP 5 //ten time steps per sample
#include <orbitersdk.h>
#include <vector>
#include "HudScroll.h"
#include "EHUD.h"
#include "LOGGER.h"

// crappy low pass filter
class LPFILTER
{
public:
	LPFILTER()
	{
		fcThresh = 20;
	}
	LPFILTER(unsigned long fc)
	{
		fcThresh=fc;
	}
	int input(double d)
	{
		if (data.size()>fcThresh){
			data.clear();
		}
		data.push_back(d);
		return data.size();
	}
	double avg()
	{
		double sum = 0;
		for (int i = 0; i < data.size(); i++)
		{
			sum+=data[i];
		}
		return (sum/data.size());
	}
	std::vector<double> data;
	unsigned long       fcThresh;
};
// user defined limits for the vessel
class VLIMIT
{
public:
	VLIMIT()
	{
		maxVel=250;
		maxFlux=45000;
		maxPressure=30;
		maxAccel=20;
		maxCollision=3;
		maxG=20;
		minG=-10;
	}
	double maxVel;
	double maxFlux;
	double maxPressure;
	double maxAccel;
	double maxCollision;
	double maxG;
	double minG;
};
// elapsed time 
class VFTIME
{
public:
	VFTIME()
	{
		Flux=0;
		Pressure=0;
		Accel=0;
		maxG=0;
		minG=0;
	}
	double Flux;
	double Pressure;
	double Accel;
	double maxG;
	double minG;
};
// flags for a damage event
class VFLAGS
{
public:
	VFLAGS()
	{
		Flux=FALSE;
		Pressure=FALSE;
		Accel=FALSE;
		maxG=FALSE;
		minG=FALSE;
	}
	BOOL Flux;
	BOOL Pressure;
	BOOL Accel;
	BOOL maxG;
	BOOL minG;
	VFTIME time;
};
// internal system state, mostly calculation values
class IVSTATE
{
public:
	IVSTATE()
	{
		hVel=0;
		Flux=0;
		Pressure=0;
		gContact=false;
		Vel=0;
		Altitude=0;
		Mass=0;
		defaultAltitude=1;
		Pitch=0;
		Bank=0;
	}
	LPFILTER Gs;
	LPFILTER Accel;
	double   hVel;
	double   Flux;
	double   Pressure;
	double   Vel;
	bool     gContact;
	double   Altitude;
	VFLAGS   flags;
	double   Mass;
	double   defaultAltitude;
	double   Pitch;
	double   Bank;
};
// vessel state - Damaged? 
class VSTATE
{
public:
	VSTATE()
	{
		Vel=FALSE;
		Flux=FALSE;
		Pressure=FALSE;
		Accel=FALSE;
		Collision=FALSE;
		maxG=FALSE;
		minG=FALSE;
		dead=FALSE;
	}
	BOOL Vel;
	BOOL Flux;
	BOOL Pressure;
	BOOL Accel;
	BOOL Collision;
	BOOL maxG;
	BOOL minG;
	BOOL dead;
};
class MISC
{
public:
	MISC()
	{
		effect=NULL;
		fire=NULL;
	}
	THRUSTER_HANDLE effect, fire;
};
// "extended" vessel class
class VOBJ
{
public:
	VOBJ(OBJHANDLE hook, VLIMIT limit)
	{
		this->hook=hook;
		this->limit=limit;
		Init=FALSE;
	}
	VOBJ()
	{
		hook=NULL;
		Init=FALSE;
	}
	OBJHANDLE Hook() 
	{
		return hook;
	}
	VLIMIT Limits()
	{
		return limit;
	}
	VSTATE State()
	{
		return state;
	}
	void             PostStep();
	void             PreStep(HUDOPTIONS hudop);
	void             PreFlagCheck();
	void             PostFlagCheck();
	void             PrintMessages(HUDOPTIONS hudop);
	THRUSTER_HANDLE  PaintFire(void);
	THRUSTER_HANDLE  PaintSmoke(void);
	void             PaintDamage(void);
	void             UpdateIVState();
	void             CheckDamage();
	void             DisableSpacecraft();
	void             DamageStructural();
	OBJHANDLE        hook;
private:
	VLIMIT           limit;
	VSTATE           state;
	IVSTATE          ivstate;
	IVSTATE          LastState;
	EHUD             HUD;
	HUDSCROLL        hudp;
	BOOL             Init;
	MISC             misc;
};
#endif

