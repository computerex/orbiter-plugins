#include "VOBJ.h"
#include <vector>
#include <MFDSoundSDK35.h>

#define  CUT_OFF 0.001
void VOBJ::TimeStep(int SoundID)
{
	if (!oapiIsVessel(hook)){
		sprintf(msg, "Unknown Error");
		return;
	}
	if (!oapiGetVesselByIndex(iTarget) && mode == VTOV){
		mode=MAGIC;
		iTargetSource=0;
		iTarget=0;
	}
	if (VESSEL(hook).GetPropellantCount() < 1)
	{
		sprintf(msg, "No fuel sources available");
		return;
	}
	if (status==IDLE)
	{
		sprintf(msg, "IDLE: Waiting for action");
		return;
	}
	PlayMFDWave(SoundID, 0);
	if (mode==VTOV)
	{
		VESSEL * vHook = oapiGetVesselInterface(hook);
		VESSEL * vTarget = oapiGetVesselInterface(oapiGetVesselByIndex(iTarget));
		if (vTarget->GetPropellantCount() < 1)
		{
			sprintf(msg, "Target has no fuel sources");
			return;
		}
		PROPELLANT_HANDLE hpTarget = vTarget->GetPropellantHandleByIndex(iTargetSource);
		PROPELLANT_HANDLE hpHook   = vHook->GetPropellantHandleByIndex(iSelectedSource);
		if (!hpHook||!hpTarget){
			sprintf(msg, "Unknown Error!");
			return;
		}
		double maxMasstarget = vTarget->GetPropellantMaxMass(hpTarget);
		double massTarget = vTarget->GetPropellantMass(hpTarget);
		double maxMasshook = vHook->GetPropellantMaxMass(hpHook);
		double massHook = vHook->GetPropellantMass(hpHook);

		if (massTarget<CUT_OFF)
		{
			sprintf(msg, "No fuel left to transfer!");
			return;
		}
		if (fabs(maxMasshook-massHook) < CUT_OFF)
		{
			sprintf(msg, "Tank full");
			return;
		}
		if (hook == oapiGetVesselByIndex(iTarget))
		{
			if (iSelectedSource == iTargetSource)
			{
				sprintf(msg, "Cannot transfer fuel to the same source");
				return;
			}
		}
		double dFuelTrans = 0;
		if (massTarget-dTransferRate >= 0 && massHook+dTransferRate <= maxMasshook)
		{
			dFuelTrans = dTransferRate;
		}else
		{
			if (maxMasshook-massHook < massTarget)
			{
				dFuelTrans = maxMasshook-massHook;
			}else if (maxMasshook-massHook > massTarget)
			{
				dFuelTrans = massTarget;
			}else
				dFuelTrans = massTarget;
		}
		dFuelTrans*=oapiGetSimStep();
		vTarget->SetPropellantMass(hpTarget, massTarget-dFuelTrans);
		vHook->SetPropellantMass(hpHook, massHook+dFuelTrans);
		sprintf(msg, "Transfering %.2f%%", ((massHook+dFuelTrans)/maxMasshook)*100, 
			vTarget->GetName(), ((massTarget-dFuelTrans)/maxMasstarget)*100);
	}
	else if (mode == MAGIC)
	{
		VESSEL * v = oapiGetVesselInterface(hook);
		PROPELLANT_HANDLE hpr = v->GetPropellantHandleByIndex(iSelectedSource);
		if (!hpr)
		{
			sprintf(msg, "Unknown error!");
			return;
		}
		double mass = v->GetPropellantMass(hpr);
		double maxmass = v->GetPropellantMaxMass(hpr);
		if (fabs(maxmass-mass) < CUT_OFF){
			sprintf(msg, "Tank full");
			return;
		}
		double dFuelTrans = 0;
		if (mass+dTransferRate > maxmass)
			dFuelTrans = maxmass-mass;
		else
			dFuelTrans = dTransferRate;
		dFuelTrans*=oapiGetSimStep();
		v->SetPropellantMass(hpr, dFuelTrans+mass);
		sprintf(msg, "Transfering %.2f%%", ((mass+dFuelTrans)/maxmass)*100);
	}else if (mode == DUMPING)
	{
		VESSEL * v = oapiGetVesselInterface(hook);
		PROPELLANT_HANDLE hpr = v->GetPropellantHandleByIndex(iSelectedSource);
		if (!hpr)
		{
			sprintf(msg, "Unknown error!");
			return;
		}
		double mass = v->GetPropellantMass(hpr);
		double maxmass = v->GetPropellantMaxMass(hpr);
		if (mass <CUT_OFF)
		{
			sprintf(msg, "No more fuel left");
			return;
		}
		double dFuelTrans=0;
		if (mass-dTransferRate < 0)
			dFuelTrans = mass;
		else
			dFuelTrans = dTransferRate;
		dFuelTrans*=oapiGetSimStep();
		v->SetPropellantMass(hpr, mass-dFuelTrans);
		sprintf(msg, "Dumping %.2f%%", ((mass-dFuelTrans)/maxmass)*100);
	}
}
int VOBJ::GetIndexByHandle(OBJHANDLE h)
{
	if (!oapiIsVessel(h))
		return 0;
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		if (h == oapiGetVesselByIndex(i))
			return i;
	}
	return 0;
}
VFMODE VOBJ::toggleMode()
{
	status=IDLE;
	if (mode==DUMPING)
	{
		mode = VTOV;
		iTarget=0;
	}else if (mode == VTOV)
	{
		mode = MAGIC;
		iTarget=0;
	}else if (mode == MAGIC)
	{
		mode = DUMPING;
		iTarget=0;
	}
	return mode;
}
OBJHANDLE VOBJ::nextTarget()
{
	/*if (iTarget+1 < oapiGetVesselCount())
		iTarget++;
	else
		iTarget=0;*/
	OBJHANDLE hVes=NULL;
	OBJHANDLE hTarget=oapiGetVesselByIndex(iTarget);
	for (int i = iTarget; i < oapiGetVesselCount(); i++)
	{
		hVes = oapiGetVesselByIndex(i);
		if (hTarget==hVes)
			continue;
		if (VESSEL(hVes).GetPropellantCount() < 1)
			continue;
		iTarget=i;
		return hVes;
	}
	for (int i = 0; i < iTarget; i++)
	{
		hVes = oapiGetVesselByIndex(i);
		if (hTarget==hVes)
			continue;
		if (VESSEL(hVes).GetPropellantCount() < 1)
			continue;
		iTarget=i;
		return hVes;
	}
	iTarget=0;
	return oapiGetVesselByIndex(iTarget);
}
OBJHANDLE VOBJ::prevTarget()
{
	/*if (iTarget-1 < 0)
		iTarget=oapiGetVesselCount()-1;
	else
		iTarget--;*/
	OBJHANDLE hVes = NULL;
	OBJHANDLE hTarget = oapiGetVesselByIndex(iTarget);
	for (int i = iTarget; i > 0; i--)
	{
		hVes = oapiGetVesselByIndex(i);
		if (hTarget==hVes)
			continue;
		if (VESSEL(hVes).GetPropellantCount() < 1)
			continue;
		iTarget=i;
		return hVes;
	}
	for (int i = oapiGetVesselCount()-1; i > iTarget; i--)
	{
		hVes = oapiGetVesselByIndex(i);
		if (hTarget==hVes)
			continue;
		if (VESSEL(hVes).GetPropellantCount() < 1)
			continue;
		iTarget=i;
		return hVes;
	}
	iTarget=0;
	return oapiGetVesselByIndex(iTarget);
}
int VOBJ::nextTargetSource()
{
	if (!oapiIsVessel(oapiGetVesselByIndex(iTarget)))
		return -1;
	int count = vtarget()->GetPropellantCount();
	if (iTargetSource+1 < count)
		iTargetSource++;
	else
		iTargetSource=0;
	return iTargetSource;
}
int VOBJ::nextSelectSource()
{
	int count = vhook()->GetPropellantCount();
	if (iSelectedSource+1 < count)
		iSelectedSource++;
	else
		iSelectedSource=0;
	return iSelectedSource;
}