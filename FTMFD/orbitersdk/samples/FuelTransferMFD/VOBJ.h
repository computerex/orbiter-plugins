#ifndef _H_COMPUTEREX_FUEL_TRANSFER_MFD_H_VOBJ_
#define _H_COMPUTEREX_FUEL_TRANSFER_MFD_H_VOBJ_ 
#include <orbitersdk.h>
#include <MFDSoundSDK35.h>

enum VFSTATUS { IDLE = 0, FUELING };
enum VFMODE { VTOV = 0, MAGIC, DUMPING };

class VOBJ
{
public:
	VOBJ()
	{
		hook = NULL;
		iSelectedSource = iTargetSource = 0;
		strcpy(msg,"");
		status = IDLE;
		mode = MAGIC;
		dTransferRate=2;
		iTarget=0;
	}
	VOBJ (OBJHANDLE hook)
	{
		this->hook=hook;
		iSelectedSource = iTargetSource = 0;
		strcpy(msg,"");
		status = IDLE;
		mode = MAGIC;
		dTransferRate=2;
		iTarget=0;
	}
	void      TimeStep(int sID);
	int       getSelectedSource() { return iSelectedSource; }
	int       getTargetSource() { return iTargetSource; }
	double    getTransferRate() { return dTransferRate; }

	OBJHANDLE getHook() { return hook; }
	OBJHANDLE getTarget() { return oapiGetVesselByIndex(iTarget); }
	VFMODE    getMode() { return mode; }
	VFSTATUS  getStatus() { return status; }

	void      setHook(OBJHANDLE h) { hook = h; }
	void      setSource(int s) { iSelectedSource = s; }
	void      setTargetSource(int s) { iTargetSource = s; }
	void      setMode(VFMODE m) { mode = m; }
	void      setStatus(VFSTATUS s) { status = s; }
	void      setTransferRate(double tr) { dTransferRate = tr; }
	char *    message() { return msg; }
	VESSEL *  vhook() { return oapiGetVesselInterface(hook); }
	VESSEL *  vtarget() { return oapiGetVesselInterface(oapiGetVesselByIndex(iTarget)); }

	VFSTATUS  toggleStatus(int sID)
	{
		if (status==FUELING){
			status=IDLE;
			StopMFDWave(sID, 0);
			PlayMFDWave(sID, 1);
		}
		else{
			StopMFDWave(sID, 1);
			status=FUELING;
		}
		return status;
	}
	VFMODE    toggleMode();
	int       GetIndexByHandle(OBJHANDLE h);
	OBJHANDLE nextTarget();
	OBJHANDLE prevTarget();
	int       nextTargetSource();
	int       nextSelectSource();
private:
	int       iSelectedSource, iTargetSource, iTarget;
	OBJHANDLE hook;
	char      msg[255];
	VFSTATUS  status;
	VFMODE    mode;
	double    dTransferRate;
};

#endif