#ifndef _H_COMPUTEREX_HUD_INTERFACE_
#define _H_COMPUTEREX_HUD_INTERFACE_

#include <orbitersdk.h>
#include <vector>

class HUDOPTIONS
{
public:
	HUDOPTIONS()
	{
		xofs=0.0;
		yofs=0.05;
		size=0.4;
		startx=0.07;
		starty=0.5;
		color=_V(1,0,0);
	}
	double xofs, yofs, size, startx, starty;
	VECTOR3 color;
};
class ANNOTATION
{
public:
	ANNOTATION(double x, double xx, double y, double yy, double s, VECTOR3 c, char * m)
	{
		x1   = x;
		x2   = xx;
		y1   = y;
		y2   = yy;
		size = s;
        col  = c;
		strcpy(msg, m);
	}
	ANNOTATION()
	{
		x1   =  0.3;
		x2   =  1.0;
		y1   =  0.3;
		y2   =  1.0;
		size =  1.0;
		col  =  _V(1,0,0);
		strcpy(msg, "HUD Interface");
	}
	double x1, x2, y1, y2, size;
	VECTOR3 col;
	char msg[255];
};

class HUDEVENT
{
public:
	HUDEVENT()
	{
		LifeTime = 3;
		From     = NULL;
	}
	HUDEVENT(double t, OBJHANDLE f, ANNOTATION *A)
	{
		LifeTime = t;
		From     = f;
		Ann      = A;
	}
	~HUDEVENT()
	{
		delete Ann;
		if (hNote)
			oapiDelAnnotation(hNote);
	}
	double     LifeTime;
	double     RegisterTime;
	OBJHANDLE  From;
	ANNOTATION *Ann;
	NOTEHANDLE hNote;
};
class EHUD
{
public:
	EHUD();
	~EHUD();
private:
	std::vector<HUDEVENT*> events;
	void      Prune();
	bool      isFocus(OBJHANDLE h);
	bool      isNull(OBJHANDLE h); 
public:
	DWORD     Count ();
	HUDEVENT* Add(double lt, OBJHANDLE f, ANNOTATION *ann); 
	void      Extend(double lf, HUDEVENT * ev, char * msg);
	void      TimeStep();
	ANNOTATION* _D(double x, double xx, double y, double yy, double s, VECTOR3 c, char * msg);
};

#endif
