#ifndef _H_COMPUTEREX_NO_RE_FUEL_H_
#define _H_COMPUTEREX_NO_RE_FUEL_H_

#include <orbitersdk.h>
#include <vector>

class P_RES
{
public:
	P_RES()
	{
		lastmass=0;
	}
	PROPELLANT_HANDLE hpr;
	double lastmass;
};
class VOBJ
{
public:
       VOBJ(OBJHANDLE h)
       {
           hook = h;
		   DWORD pcount = VESSEL(h).GetPropellantCount();
		   for (int i = 0; i < pcount; i++)
		   {
			   P_RES pr;
			   pr.hpr = VESSEL(h).GetPropellantHandleByIndex(i);
			   pr.lastmass=VESSEL(h).GetPropellantMass(pr.hpr);
			   hprs.push_back(pr);
		   }
       }
       VOBJ()
       {
       }
       void TimeStep(void)
	   {
		   VESSEL * vessel = oapiGetVesselInterface(hook);
		   double cpm = 0;
		   for (int i = 0; i < hprs.size(); i++)
		   {
			   cpm = vessel->GetPropellantMass(hprs[i].hpr);
			   if (cpm < hprs[i].lastmass){
				   hprs[i].lastmass=cpm;
				   continue;
			   }
			   double diff = cpm - hprs[i].lastmass;
			   if (diff>1)
			   {
				   vessel->SetPropellantMass(hprs[i].hpr, hprs[i].lastmass);
			   }
			   hprs[i].lastmass=cpm;
		   }
	   }
       static double GetTotalFuelMass(OBJHANDLE handle)
       {
              if (!oapiIsVessel(handle))
                  return 0;
              VESSEL * v = oapiGetVesselInterface(handle);
              double sum=0;
              for (int i = 0; i < v->GetPropellantCount(); i++)
			  {
				  PROPELLANT_HANDLE hpr = v->GetPropellantHandleByIndex(i);
				  sum+=v->GetPropellantMass(hpr);
			  }
			  return sum;
	   }
       OBJHANDLE hook;
	   std::vector<P_RES> hprs;
};
#endif
             
   