#define STRICT
#define ORBITER_MODULE

#include "orbitersdk.h"
#include <time.h>
#include <vector>
#include "FileReader.h"

int GetRand(int from, int to)
{
	return rand() % to + from;
}
double GDRand(double from, double  span)
{
	return (double)rand() / (double)RAND_MAX * span + from;
}
int _OBJ_COUNT = 1000;
#define KM 1000
std::vector<OBJHANDLE> hVes;
std::vector<std::string> meshes;
BOOL Init = FALSE;
FILEREADER g_FR;
VECTOR3 ofs = {1000,500,500};
VECTOR3 d_ofs = {1000,1000,500};
VECTOR3 relV = {0,0,-100};
double A_ofs = 250*KM;
void init(void)
{
	char ini[255];
	char cbuf[255];
	char str[255];
	sprintf(ini,"./Modules/Plugin/roid.ini");
    g_FR.LoadVector(ini,"SETTINGS","destroy_ofs",d_ofs);
	g_FR.LoadVector(ini,"SETTINGS","create_ofs",ofs);
	g_FR.LoadInt(ini,"SETTINGS","count",_OBJ_COUNT);
	g_FR.LoadVector(ini,"SETTINGS","relvel",relV);
	g_FR.LoadDouble(ini,"SETTINGS","renderalt",A_ofs);
	for (int i = 0; i < _OBJ_COUNT; i++)
	{
		OBJHANDLE h = 0;
		hVes.push_back(h);
	}
	int mCount = 0; 
	g_FR.LoadInt(ini,"SETTINGS","mesh_count",mCount);
	for (int i = 0; i < mCount; i++)
	{
		sprintf(cbuf,"mesh_%d_name",i+1);
		g_FR.LoadString(ini,"SETTINGS",cbuf,str);
		meshes.push_back(std::string(str));
	}
}
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	if (Init == FALSE)
	{
		srand(time(NULL));
		Init = TRUE;
		OBJHANDLE h = 0;
		for (int i = 0; i < oapiGetVesselCount(); i++)
		{
			h = oapiGetVesselByIndex(i);
			if (!strnicmp(VESSEL(h).GetName(),"obj_",4))
			{
				oapiDeleteVessel(h);
			}
		}
		init();
	}
	VESSEL * v = oapiGetFocusInterface();
	VECTOR3 Pos, relpos;
	VESSELSTATUS vs;
	char name[255];
	double x = 0;
	if (v->GroundContact())
		return;
	if (v->GetAltitude()/KM < A_ofs)
		return;
	if (FindWindow(NULL,"Orbiter: Select spacecraft"))
		return;
	v->GetStatus(vs);
	oapiGetGlobalPos(v->GetHandle(),&Pos);
	v->Global2Local(Pos,Pos);	    
	for (int i = 0; i < hVes.size(); i++)
	{
		if (!hVes[i])
		{
			Pos.z+=GDRand(1, ofs.z);
			x = GDRand(-ofs.x,ofs.x);
			if (GetRand(1,10) > 5)
				x=-x;
			Pos.x+=x;
			Pos.y+=GDRand(0,ofs.y);
			if (GetRand(1,10) > 5)
				Pos.y=-Pos.y;
		    v->Local2Rel(Pos,Pos);
		    VECTOR3 rofs;
	        v->GlobalRot(relV,rofs);
	        vs.rpos = Pos;
            vs.rvel+=rofs;
			sprintf(name,"obj_%d",i+1);
		    hVes[i] = oapiCreateVessel(name,"roid",vs);
			VESSEL(hVes[i]).AddMesh(meshes[GetRand(0,meshes.size())].c_str());
		} 
	}
	for (int i = 0; i < hVes.size(); i++)
	{
		oapiGetGlobalPos(hVes[i],&Pos);
		v->Global2Local(Pos,Pos);
		if ((fabs(Pos.z) > d_ofs.z) || (fabs(Pos.y) > d_ofs.y) || (fabs(Pos.x) > d_ofs.x))
		{
			oapiDeleteVessel(hVes[i]);
			hVes[i]=0;
		}
	}
}