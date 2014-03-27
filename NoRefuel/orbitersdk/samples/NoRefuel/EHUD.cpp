#include "EHUD.h"

ANNOTATION* EHUD::_D(double x, double xx, double y, double yy, double s, VECTOR3 c, char * msg)
{
	ANNOTATION *a = new ANNOTATION(x,xx,y,yy,s,c,msg);
	return a;
}
EHUD::EHUD()
{
}
void EHUD::Prune()
{
	if(events.empty())
		return;
	std::vector<HUDEVENT*>::iterator it = events.begin() + 1;
	HUDEVENT * ev;
	for (int i = 0; i < events.size(); i++)
	{
		it = events.begin() + i;
		ev = events[i];

		if (ev->LifeTime > 0 && ev->RegisterTime+ev->LifeTime < oapiGetSimTime())
		{
			oapiDelAnnotation(ev->hNote);
			delete ev;
			events.erase(it);
		}
	}
}
HUDEVENT* EHUD::Add(double lt, OBJHANDLE f, ANNOTATION *ann)
{
	HUDEVENT * ev = new HUDEVENT(lt, f, ann);
	ev->RegisterTime = oapiGetSimTime();
	ev->hNote=oapiCreateAnnotation(true,ann->size,ann->col);
	oapiAnnotationSetPos(ev->hNote,ann->x1,ann->y1,ann->x2,ann->y2);
	events.push_back(ev);
	return ev;
}
void EHUD::Extend(double lf, HUDEVENT *ev, char *msg)
{
	ev->LifeTime=lf;
	ev->RegisterTime=oapiGetSimTime();
	strcpy(ev->Ann->msg,msg);
}
void EHUD::TimeStep()
{
	HUDEVENT * ev;
	for (int i = 0; i < events.size(); i++)
	{
		if (!events[i])
			continue;
		ev = events[i];

		if (!isNull(ev->From))
		{
			if (!isFocus(ev->From))
				continue;
		}
		oapiAnnotationSetText(ev->hNote,ev->Ann->msg);
	}
	Prune();
}
bool EHUD::isFocus(OBJHANDLE h)
{
	if (!oapiIsVessel(h))
		return false;
	if (oapiGetFocusInterface()->GetHandle() == h)
		return true;
	return false;
}
bool EHUD::isNull(OBJHANDLE h)
{
	if (h == NULL)
		return true;
	return false;
}
DWORD EHUD::Count()
{
	return events.size();
}
