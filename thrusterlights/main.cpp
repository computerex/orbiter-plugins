#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include <vector>

class Thruster
{
public:
	Thruster(OBJHANDLE hook, THRUSTER_HANDLE th)
	{
		this->hook=hook;
		this->th=th;
		VESSEL * v = oapiGetVesselInterface(hook);
		VECTOR3 pos;
		v->GetThrusterRef(th, pos);
		double m0=v->GetThrusterMax0(th);
		COLOUR4 col_d = {0.9f,0.8f,1.0f,0.0f};
		COLOUR4 col_s = {1.9f,0.8f,1.0f,0.0f};
		COLOUR4 col_a = {0.0f,0.0f,0.0f,0.0f};
		light=v->AddPointLight(pos, m0*0.01, 0.0, 0.0, 2e-3, col_d, col_s, col_a ); 
	}
	void timestep()
	{
		if ( th == 0 ) return;
		if ( !oapiIsVessel(hook) ) return;
		VESSEL * v = oapiGetVesselInterface(hook);
		double lvl = v->GetThrusterLevel(th);
		light->SetIntensity(lvl);
	}
	OBJHANDLE hook;
	THRUSTER_HANDLE th;
	LightEmitter * light;
};

class VOBJ
{
public:
	VOBJ(OBJHANDLE hook)
	{
		this->hook=hook;
	}
	~VOBJ()
	{
		for ( unsigned int i = 0; i < thrusters.size(); i++ )
		{
			if ( thrusters[i] )
				delete thrusters[i];
			thrusters[i]=0;
		}
		thrusters.clear();
	}
	void timestep()
	{
		if ( !oapiIsVessel(hook) ) return;
		prune();
		VESSEL * v = oapiGetVesselInterface(hook);
		THRUSTER_HANDLE th;
		DWORD tcnt = v->GetThrusterCount();
		bool found=false;
		for ( unsigned int i = 0; i < tcnt; i++ )
		{
			th=v->GetThrusterHandleByIndex(i);
			found=false;
			for ( unsigned int j = 0; j < thrusters.size(); j++ )
			{
				if ( th == thrusters[j]->th ){
					found=true;
					break;
				}
			}
			if ( found ) continue;
			thrusters.push_back(new Thruster(hook, th));
		}
		for ( unsigned int i = 0; i < thrusters.size(); i++ )
			thrusters[i]->timestep();
	}
	void prune()
	{
		std::vector<Thruster*>::iterator it;
		for ( unsigned int i = 0; i < thrusters.size(); i++ )
		{
			if ( thrusters[i] == 0 ){
				it=thrusters.begin()+i;
				thrusters.erase(it);
				break;
			}
			_ASSERT(oapiIsVessel(thrusters[i]->hook));
			if ( thrusters[i]->th == 0 ){
				delete thrusters[i];
				thrusters[i]=0;
				thrusters.erase(it);
			}
		}
	}
	std::vector<Thruster*> thrusters;
	OBJHANDLE hook;
};

std::vector<VOBJ*> vlist;
DLLCLBK void opcFocusChanged(OBJHANDLE hgains, OBJHANDLE hlose)
{
}

DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	OBJHANDLE hv;
	bool found;
	for ( unsigned int i = 0; i < oapiGetVesselCount(); i++ )
	{
		hv = oapiGetVesselByIndex(i);
		found=false;
		for ( unsigned int j = 0; j < vlist.size(); j++ )
		{
			if ( vlist[j]->hook == hv ) { found = true; break; }
		}
		if ( !found )
			vlist.push_back(new VOBJ(hv));
	}
	std::vector<VOBJ*>::iterator it;
	for ( unsigned int i = 0; i < vlist.size(); i++ )
	{
		if ( vlist[i] == 0 ){
			it=vlist.begin()+i;
			vlist.erase(it);
			continue;
		}
		if ( !oapiIsVessel(vlist[i]->hook) ){
			delete vlist[i];
			vlist[i]=0;
			it=vlist.begin()+i;
			vlist.erase(it);
			continue;
		}
		vlist[i]->timestep();
	}
}

DLLCLBK void opcOpenRenderViewport(HWND hwnd, DWORD width, DWORD height, BOOL fc)
{
}

DLLCLBK void opcCloseRenderViewport()
{
	for ( unsigned int i = 0; i < vlist.size(); i++ )
	{
		if ( vlist[i] != 0 ){
			delete vlist[i];
			vlist[i]=0;
		}
	}
	vlist.clear();
}
