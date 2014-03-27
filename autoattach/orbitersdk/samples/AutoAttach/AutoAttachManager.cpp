#include "AutoAttachManager.h"
#include <orbitersdk.h>
#include <vector>

bool AAMANAGER::isChild(OBJHANDLE hVessel)
{
	VESSEL * v = oapiGetVesselInterface(hVessel);
	ATTACHMENTHANDLE hAttach;
	OBJHANDLE hVes;
	for (DWORD i = 0; i < v->AttachmentCount(true); i++) // scan all the child attachments
	{
		hAttach=v->GetAttachmentHandle(true, i);
		hVes = v->GetAttachmentStatus(hAttach);
		if (oapiIsVessel(hVes))
			return true;
	}
	return false;
}
bool AAMANAGER::isChild(OBJHANDLE hVessel, ATTACHMENTHANDLE hAttach)
{
	VESSEL * v = oapiGetVesselInterface(hVessel);
	ATTACHMENTHANDLE hAtt;
	for (DWORD i = 0; i < v->AttachmentCount(true); i++)
	{
		hAtt=v->GetAttachmentHandle(true, i);
		if (hAtt==hAttach)
			return true;
	}
	return false;
}
bool AAMANAGER::isParent(OBJHANDLE hVessel)
{
	if (isChild(hVessel))
		return false;
	return true;
}
bool AAMANAGER::isParent(OBJHANDLE hVessel, ATTACHMENTHANDLE hAttach)
{
	VESSEL * v = oapiGetVesselInterface(hVessel);
	ATTACHMENTHANDLE hAtt;
	for (DWORD i = 0; i < v->AttachmentCount(false); i++)
	{
		hAtt=v->GetAttachmentHandle(false, i);
		if (hAtt==hAttach)
			return true;
	}
	return false;
}
bool AAMANAGER::isAttached(OBJHANDLE hVessel)
{
	VESSEL * v = oapiGetVesselInterface(hVessel);
	OBJHANDLE hVes;
	ATTACHMENTHANDLE hAttach;
	for (DWORD i = 0; i < v->AttachmentCount(true); i++) // first scan all the child attachments
	{
		hAttach=v->GetAttachmentHandle(true, i);
		hVes = v->GetAttachmentStatus(hAttach);
		if (oapiIsVessel(hVes))
			return true;
	}
	for (DWORD i = 0; i < v->AttachmentCount(false); i++) // now scan the parents
	{
		hAttach=v->GetAttachmentHandle(false, i);
		hVes = v->GetAttachmentStatus(hAttach);
		if (oapiIsVessel(hVes))
			return true;
	}
	return false;
}
bool AAMANAGER::isAttachmentBusy(OBJHANDLE hVessel, DWORD attachment, bool toparent)
{
	VESSEL * v = oapiGetVesselInterface(hVessel);
	if (v->AttachmentCount(toparent) <= attachment)
		return false;
	ATTACHMENTHANDLE hAttach = v->GetAttachmentHandle(toparent, attachment);
	return (oapiIsVessel(hAttach));
}
int AAMANAGER::hasAttachments(OBJHANDLE hVessel)
{
	VESSEL * v = oapiGetVesselInterface(hVessel);
	int count = v->AttachmentCount(true); // child attachments
	count+=v->AttachmentCount(false);     // parent attachments
	return count;
}
OBJHANDLE AAMANAGER::AttachedTo(OBJHANDLE hVessel, int attachment, bool toparent)
{
	VESSEL * v = oapiGetVesselInterface(hVessel);
	ATTACHMENTHANDLE hAttach = v->GetAttachmentHandle(toparent, attachment);
	OBJHANDLE hVobj = v->GetAttachmentStatus(hAttach);
	if (oapiIsVessel(hVobj))
		return hVobj;
	return NULL;
}
double AAMANAGER::getDistance(OBJHANDLE hV1, OBJHANDLE hV2)
{
	VESSEL * v = oapiGetVesselInterface(hV1);
	VECTOR3 rpos;
	v->GetRelativePos(hV2, rpos);
	return length(rpos);
}
OBJHANDLE AAMANAGER::getClosestVessel(OBJHANDLE hVessel)
{
	double distance = 0, dT = 0;
	OBJHANDLE hVes, hClosest;
	for (DWORD i = 0; i < oapiGetVesselCount(); i++)
	{
		hVes = oapiGetVesselByIndex(i);
		if (hVes == hVessel)
			continue;
		if (i==0)
		{
			hClosest = hVes;
			distance = getDistance(hVessel, hVes);
			continue;
		}
		dT = getDistance(hVessel, hVes);
		if (distance>dT)
		{
			distance=dT;
			hClosest=hVes;
		}
	}
	return hClosest;
}
double AAMANAGER::getAttachmentDistance(OBJHANDLE hV1, OBJHANDLE hV2, ATTACHMENTHANDLE attachment1, ATTACHMENTHANDLE attachment2)
{
	VESSEL * v1 = oapiGetVesselInterface(hV1);
	VESSEL * v2 = oapiGetVesselInterface(hV2);
	VECTOR3 pos, dir, rot, gpos;
	v1->GetAttachmentParams(attachment1, pos, dir, rot);
	v1->Local2Global(pos, gpos);
	v2->GetAttachmentParams(attachment2, pos, dir, rot);
	v2->Local2Global(pos, pos);
	return dist(pos, gpos);
}
double AAMANAGER::GetClosestAttachment(OBJHANDLE hVessel, ATTACHMENTHANDLE hAttach, ATTACHMENTSPEC * aspec, bool toparent)
{
	std::vector<OBJHANDLE> vList;
	OBJHANDLE hV;
	VESSEL * v;
	for (DWORD i = 0; i < oapiGetVesselCount(); i++)
	{
		hV = oapiGetVesselByIndex(i);
		if (hV == hVessel)
			continue;
		v=oapiGetVesselInterface(hV);
		if (!v->AttachmentCount(toparent))
			continue;
		vList.push_back(hV);
	}
	if (vList.size() < 1)
	{
		aspec->hook=NULL;
		aspec->inx=0;
		return 0;
	}
	VECTOR3 gpos, pos, rot, dir;
	v=oapiGetVesselInterface(hVessel);
	v->GetAttachmentParams(hAttach, pos, dir, rot);
	v->Local2Global(pos, gpos);
	double distance=0;
	ATTACHMENTHANDLE attach;
	for (DWORD i = 0; i < vList.size(); i++)
	{
		v=oapiGetVesselInterface(vList[i]);
		if (i==0)
		{
			attach=v->GetAttachmentHandle(toparent, 0);
			v->GetAttachmentParams(attach, pos, dir, rot);
			v->Local2Global(pos, pos);
			distance=dist(gpos, pos);
			aspec->hook=v->GetHandle();
			aspec->inx=0;
		}
		for (DWORD j = 0; j < v->AttachmentCount(toparent); j++)
		{
			attach=v->GetAttachmentHandle(toparent, j);
			v->GetAttachmentParams(attach, pos, dir, rot);
			v->Local2Global(pos, pos);
			if (distance > dist(gpos, pos))
			{
				aspec->hook=v->GetHandle();
				aspec->inx=j;
				distance=dist(gpos, pos);
			}
		}
	}
	return distance;
}

OBJHANDLE AAMANAGER::FindMatch(OBJHANDLE hVessel, ATTACHMENTHANDLE * hvAtt, ATTACHMENTHANDLE * htAtt, double * dist)
{
	VESSEL *               v=oapiGetVesselInterface(hVessel);
	double                 distance = 0, dT = 0;
	ATTACHMENTHANDLE       hAttach;
	OBJHANDLE              hVRet;
	ATTACHMENTSPEC         spec;
	OBJHANDLE              hVes;
	std::vector<OBJHANDLE> ChildList;
	std::vector<OBJHANDLE> ParentList;
	for (DWORD i = 0; i < oapiGetVesselCount(); i++)
	{
		hVes = oapiGetVesselByIndex(i);
		if (hVes == hVessel)
			continue;
		v = oapiGetVesselInterface(hVes);
		if (v->AttachmentCount(true))
			ChildList.push_back(hVes);
		if (v->AttachmentCount(false))
			ParentList.push_back(hVes);
	}
	if (VESSEL(hVessel).AttachmentCount(true))
	{
		if (ParentList.size() < 1){
			if (oapiGetFocusInterface()->GetHandle()==hVessel)
		sprintf(oapiDebugString(), "no parents");
		    return NULL;
		}
	}else
	{
		if (ChildList.size() < 1){
			if (oapiGetFocusInterface()->GetHandle()==hVessel)
		sprintf(oapiDebugString(), "no children");
			return NULL;
		}
	}
	v=oapiGetVesselInterface(hVessel);
	for (DWORD i = 0; i < v->AttachmentCount(true); i++)
	{
		hAttach=v->GetAttachmentHandle(true, i);
		if (i==0)
		{
			distance = GetClosestAttachment(hVessel, hAttach, &spec, false);
			hVRet=spec.hook;
			*hvAtt=hAttach;
			*htAtt=VESSEL(hVRet).GetAttachmentHandle(false, spec.inx);
		}
		dT = GetClosestAttachment(hVessel, hAttach, &spec, false);
		if (dT < distance)
		{
			distance = dT;
			hVRet=spec.hook;
			*hvAtt=hAttach;
			*htAtt=VESSEL(hVRet).GetAttachmentHandle(false, spec.inx);
		}
	}
	for (DWORD i = 0; i < v->AttachmentCount(false); i++)
	{
		hAttach=v->GetAttachmentHandle(false, i);
		if (i==0)
		{
			distance = GetClosestAttachment(hVessel, hAttach, &spec, true);
			hVRet=spec.hook;
			*hvAtt=hAttach;
			*htAtt=VESSEL(hVRet).GetAttachmentHandle(true, spec.inx);
		}
		dT = GetClosestAttachment(hVessel, hAttach, &spec, true);
		if (dT < distance)
		{
			distance = dT;
			hVRet=spec.hook;
			*hvAtt=hAttach;
			*htAtt=VESSEL(hVRet).GetAttachmentHandle(true, spec.inx);
		}
	}
	*dist=distance;
	if (oapiIsVessel(hVRet))
		return hVRet;
	return NULL;
}
OBJHANDLE AAMANAGER::FindAttachmentParent(ATTACHMENTHANDLE hAttach)
{
	OBJHANDLE hVes = NULL;
	ATTACHMENTHANDLE hAtt;
	VESSEL * v = NULL;
	for (DWORD i = 0; i < oapiGetVesselCount(); i++)
	{
		hVes = oapiGetVesselByIndex(i);
		v = oapiGetVesselInterface(hVes);
		for (DWORD j = 0; j < v->AttachmentCount(true); j++)
		{
			hAtt = v->GetAttachmentHandle(true, j);
			if (hAttach==hAtt)
				return hVes;
		}
		for (DWORD j = 0; j < v->AttachmentCount(false); j++)
		{
			hAtt = v->GetAttachmentHandle(false, j);
			if (hAttach==hAtt)
				return hVes;
		}
	}
	return NULL;
}
void AAMANAGER::ProcessVessel(OBJHANDLE hVessel)
{
	if (!hasAttachments(hVessel))
		return;
	if (isChild(hVessel))
		return;
	if (getDistance(hVessel, getClosestVessel(hVessel))>50) // scan in a 50 meter radius
		return;
	ATTACHMENTHANDLE hAttach1, hAttach2;
	double distance = 0;
	OBJHANDLE hTarget = FindMatch(hVessel, &hAttach1, &hAttach2, &distance);
	if (!oapiIsVessel(hTarget)){
		return;
	}
	if (distance>1) // attachment distance must be within 1 meters
		return;
	OBJHANDLE HV1, HV2;
	HV1 = FindAttachmentParent(hAttach1);
	HV2 = FindAttachmentParent(hAttach2);
	if (!oapiIsVessel(HV1) || !oapiIsVessel(HV2)){
		return;
	}
	if (isChild(HV1, hAttach1))
	{
		OBJHANDLE hChild = HV1;
		OBJHANDLE hParent = HV2;
		VESSEL * v = oapiGetVesselInterface(hParent);
		v->AttachChild(hChild, hAttach2, hAttach1);
	}else
	{
		OBJHANDLE hChild = HV2;
		OBJHANDLE hParent = HV1;
		VESSEL * v = oapiGetVesselInterface(hParent);
		v->AttachChild(hChild, hAttach1, hAttach2);
	}
}
void AAMANAGER::TimeStep()
{
	DWORD inx = oapiGetVesselCount();
	if (inx<2)
		return;
	OBJHANDLE hVessel = NULL;
	for (DWORD i = 0; i < inx; i++)
	{
		hVessel=oapiGetVesselByIndex(i);
		ProcessVessel(hVessel);
	}
}