#ifndef _H_COMPUTEREX_PARACHUTE_UNIVERSAL_PARACHUTE_H_
#define _H_COMPUTEREX_PARACHUTE_UNIVERSAL_PARACHUTE_H_
#pragma once
#include <orbitersdk.h>

enum PARASTATE { PACKED = 0, DEPLOYED, JETTISONED };
enum CHUTETYPE { CTYPE_DRAG = 0, CTYPE_PARACHUTE };

class PARACHUTE
{
public:
	PARACHUTE()
	{
		hook  = NULL;
		state = PACKED;
		strcpy(mesh, "");
		ofs   = _V(0,0,0);
		meshInx=0;
		force = 0;
		dcoef = 1.5;
		area  = 10;
		type  = CTYPE_PARACHUTE;
	}
	PARACHUTE(OBJHANDLE hook, CHUTETYPE type, double dcoef, double area, char * meshName, PARASTATE state, VECTOR3 ofs)
	{
		this->hook=hook;
		this->dcoef = dcoef;
		this->area = area;
		this->state=state;
		strcpy(mesh, meshName);
		meshInx=0;
		this->ofs=ofs;
		this->type=type;
		force = 0;
	}
	bool       deployed() { return (state==DEPLOYED); }
	bool       jettisoned() { return (state==JETTISONED); }
	bool       packed() { return (state==PACKED); };

	void       deploy();
	void       jettison();
	void       Update();

	void       setHook(OBJHANDLE hook) { this->hook = hook; }
	void       setDragCoef(double dcoef) { this->dcoef = dcoef; }
	void       setArea(double area) { this->area = area; }
	void       setMeshName(char * meshName) { strcpy(mesh, meshName); }
	void       setOfs(VECTOR3 ofs) { this->ofs = ofs; }
	void       setState(PARASTATE state) { this->state=state; }
	void       setType(CHUTETYPE type) { this->type=type; }

	PARASTATE  getState() { return state; }
	OBJHANDLE  getHook() { return hook; }
	void       getMesh(char * mName) { strcpy(mName, mesh); }
	double     getDragCoef() { return dcoef; }
	double     getArea() { return area; }
	VECTOR3    getOfs() { return ofs; }
	double     getForce() { return force; } 
	CHUTETYPE  getType() { return type; }
private:
	PARASTATE  state;
	OBJHANDLE  hook;
	char       mesh[255];
	double     dcoef;
	double     area;
	UINT       meshInx;
	VECTOR3    ofs;
	CHUTETYPE  type;
	double     force; 
};
#endif

