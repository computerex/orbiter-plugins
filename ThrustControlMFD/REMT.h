#ifndef _H_COMPUTEREX_REMOTE_THRUST_H_
#define _H_COMPUTEREX_REMOTE_THRUST_H_
#include <orbitersdk.h>
#include "VOBJ.h"
#include "MfdSoundSDK35.h"

class MFDData;
class RTHRUST
{
public:
	RTHRUST(OBJHANDLE hook, int id, MFDData * data)
	{
		this->hook=hook;
		this->id=id;
		BurnDuration = CountdownStartTime = Countdown = BurnStartTime = 0;
		burning  = false;
		CountdownStarted = false;
		this->data=data;
	}
	RTHRUST()
	{
		this->hook=NULL;
		this->id=-1;
		BurnDuration = CountdownStartTime = Countdown = BurnStartTime = 0;
		burning  = false;
		CountdownStarted = false;
		data=NULL;
	}
	void       TimeStep(void);
	void       setBurnDuration(double time) { BurnDuration=time; }
	void       setCountdown(double countdown) { Countdown = countdown; }
	void       StartCountdown()
	{
		if (CountdownStarted)
			return;
		CountdownStarted=true;
		CountdownStartTime=oapiGetSimTime()+Countdown;
	}
	void       Abort();
	void       setHook(OBJHANDLE hook) { this->hook=hook; }
	void       setID(int id) { this->id=id; }
	bool       isBurning() { return burning; }
	bool       isCountStarted() { return CountdownStarted; }
	OBJHANDLE  getHook() { return hook; }
	double     getBurnDuration() { return BurnDuration; }
	double     getCountdown() { return Countdown; }
	double     getCountdownStartTime() { return CountdownStartTime; }
	double     getBurnStartTime() { return BurnStartTime; }
private:
	bool   burning, CountdownStarted;
	double BurnDuration, CountdownStartTime, Countdown, BurnStartTime;
	OBJHANDLE hook;
	int id;
	MFDData * data;
};
#endif