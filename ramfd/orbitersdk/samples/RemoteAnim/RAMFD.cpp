#define STRICT
#define ORBITER_MODULE

#include <orbitersdk.h>
#include "RAMFD.h"
#include "Parser.h"
#include "LOGGER.h"

int mode=0; 
double animSpeedDefault = 0.01;
std::vector<VOBJ*> g_VobjVector;
std::vector<MFDData*> g_data;
PARSER g_parser;
BOOL StateFileFound=FALSE;

char * GetAttachmentStatusString(OBJHANDLE hVessel, ATTACHMENTHANDLE hAttach, char * buffer)
{
	VESSEL * v = oapiGetVesselInterface(hVessel);
	OBJHANDLE hTest = v->GetAttachmentStatus(hAttach);
	if (!oapiIsVessel(hTest))
		sprintf(buffer,"Free");
	else
		sprintf(buffer,"Attached to %s",VESSEL(hTest).GetName());
	return buffer;
}
void GetFileName(char * buffer)
{
	char cbuf[255];
	for (int i = 0; i < 1000; i++)
	{
		sprintf(cbuf,"Scenarios/RAMFD/RAMFDState%d.cfg",i+1);
		if (fopen(cbuf,"r")==0)
		{
			strcpy(buffer,cbuf);
			return;
		} 
	}
	strcpy(buffer,"Scenarios/RAMFD/RAMFDState1.cfg");
}
bool isNull(void * ptr)
{
	if (ptr==NULL)
		return true;
	return false;
}
double GetClosestAttachment(OBJHANDLE hVessel, ATTACHMENTHANDLE hAttach, ATTACHMENTSPEC * aspec, bool toparent)
{
	std::vector<OBJHANDLE> vList;
	OBJHANDLE hV;
	VESSEL * v;
	for (int i = 0; i < oapiGetVesselCount(); i++)
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
	for (int i = 0; i < vList.size(); i++)
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
		for (int j = 0; j < v->AttachmentCount(toparent); j++)
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
void Prune(int which)
{
	if (which==0)
	{	
		if (g_data.empty())
			return;
		std::vector<MFDData*>::iterator it = g_data.begin() + 1;
		for (int i = 0; i < g_data.size(); i++)
		{
			it = g_data.begin() + i;
			if (!oapiIsVessel(g_data[i]->hook))
			{
				delete g_data[i];
				g_data.erase(it);
			}
		}
	}else
	{
		if (g_VobjVector.empty())
			return;
		std::vector<VOBJ*>::iterator it = g_VobjVector.begin()+1;
		for (int i = 0; i < g_VobjVector.size(); i++)
		{
			it = g_VobjVector.begin()+1;
			if (!oapiIsVessel(g_VobjVector[i]->hook))
			{
				delete g_VobjVector[i];
				g_VobjVector.erase(it);
			}
		}
	}
}
bool GetDouble(void * id, char * str, void * data)
{
	*(double*)data=atof(str);
	return true;
}
bool isAttended(OBJHANDLE h)
{
	for (int i = 0; i < g_VobjVector.size(); i++)
	{
		if (g_VobjVector[i]->hook == h)
			return true;
	}
	return false;
}
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	Prune(0);
	Prune(1);
	OBJHANDLE hVes=NULL;
	static bool init = false;
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		hVes=oapiGetVesselByIndex(i);
		if (isAttended(hVes))
			continue;
		VOBJ * vbj = new VOBJ(hVes);
		if (StateFileFound==TRUE)
		{
			if (!vbj->init(g_parser))
			{
				delete vbj;
				continue;
			}else
				g_VobjVector.push_back(vbj);
		}else
		{
			if (!vbj->init())
			{
				delete vbj;
				continue;
			}else
				g_VobjVector.push_back(vbj);
		}
	}
	if (!init)
	{
		init=true;
		for (int i = 0; i < g_VobjVector.size(); i++)
	    {
			for (int j = 0; j < g_VobjVector[i]->av.size(); j++)
	  	    {
				g_VobjVector[i]->setstate(j,g_VobjVector[i]->state(j));
		    }
	    }
	}
}
DLLCLBK void opcDLLInit (HINSTANCE hDLL)
{
	static char *name = "Remote Animation MFD";
	MFDMODESPEC spec;
	spec.name    = name;
	spec.key     = OAPI_KEY_R;
	spec.msgproc = RAMFD::MsgProc;
	mode = oapiRegisterMFDMode (spec);
	PARSER g_Parser("Modules/Plugin/RAMFD.cfg");
	g_Parser.read();
	animSpeedDefault=g_Parser.Double("DefaultSpeed");
}

DLLCLBK void opcDLLExit (HINSTANCE hDLL)
{
	oapiUnregisterMFDMode (mode);
}
RAMFD::RAMFD (DWORD w, DWORD h, VESSEL *vessel)
: MFD (w, h, vessel)
{
	bool found = false;
	for (int i = 0; i < g_data.size(); i++)
	{
		if (g_data[i]->hook==vessel->GetHandle())
		{
			this->data=g_data[i];
			found=true;
			break;
		}
	}
	if (!found)
	{
		MFDData * d = new MFDData();
		d->hook=vessel->GetHandle();
		d->animSpeed=animSpeedDefault;
		g_data.push_back(d);
		this->data=d;
	}
	width=w/25;
	height=h/24;
}
RAMFD::~RAMFD ()
{
}
int RAMFD::MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case OAPI_MSG_MFD_OPENED:
		return (int)(new RAMFD (LOWORD(wparam), HIWORD(wparam), (VESSEL*)lparam));
	}
	return 0;
}

bool RAMFD::ConsumeKeyBuffered (DWORD key)
{
	if (g_VobjVector.size() < 1)
	{
		return false;
	}
	VOBJ * vbj = g_VobjVector[data->iSelected];
	VESSEL * v = oapiGetVesselInterface(vbj->hook);
	switch(key)
	{
	case OAPI_KEY_EQUALS:
		{
			if (data->iSelected+1 >= g_VobjVector.size())
				data->iSelected=0;
			else
				data->iSelected++;
			data->iSelectedAnimation=0;
			data->iSelectedAttachment=0;
			return true;
		}
	case OAPI_KEY_MINUS:
		{
			if (data->iSelectedAnimation+1 >= g_VobjVector[data->iSelected]->av.size())
				data->iSelectedAnimation=0;
			else
				data->iSelectedAnimation++;
			return true;
		}
	case OAPI_KEY_LBRACKET:
		{
			vbj->setstate(data->iSelectedAnimation,vbj->state(data->iSelectedAnimation)+data->animSpeed);
			return true;
		}
	case OAPI_KEY_RBRACKET:
		{
			vbj->setstate(data->iSelectedAnimation,vbj->state(data->iSelectedAnimation)-data->animSpeed);
			return true;
		}
	case OAPI_KEY_S:
		{
			oapiOpenInputBox("Enter animation speed: ", GetDouble, 0, 20, &data->animSpeed);
			return true;
		}
	case OAPI_KEY_R:
		{
			if (vbj->av.size() < 1)
				return false;
			vbj->reset(data->iSelectedAnimation);
			return true;
		}
	case OAPI_KEY_0:
		{
			if (data->iSelectedAnimation-1 >= 0)
				data->iSelectedAnimation--;
			else
				data->iSelectedAnimation=g_VobjVector[data->iSelected]->av.size()-1;
			return true;
		}
	case OAPI_KEY_T:
		{
			if (data->toparent)
				data->toparent=false;
			else
				data->toparent=true;
			return true;
		}
	case OAPI_KEY_E:
		{
			oapiOpenInputBox("Enter attachment range [meters]",GetDouble, 0, 20, &data->fAttachDistance);
			return true;
		}
	case OAPI_KEY_Q:
		{
			if (data->iSelectedAttachment+1 >= v->AttachmentCount(data->toparent))
				data->iSelectedAttachment=0;
			else
				data->iSelectedAttachment++;
			return true;
		}
	case OAPI_KEY_W:
		{
			if (!v->AttachmentCount(data->toparent))
				return false;
			if (data->iSelectedAttachment-1 >= 0)
				data->iSelectedAttachment--;
			else
				data->iSelectedAttachment=v->AttachmentCount(data->toparent)-1;
			return true;
		}
	case OAPI_KEY_A:
		{
			if (!v->AttachmentCount(data->toparent))
				return false;
			char status[255];
			ATTACHMENTHANDLE attach = v->GetAttachmentHandle(data->toparent, data->iSelectedAttachment);
			GetAttachmentStatusString(v->GetHandle(), attach, status);
			if (strcmp("Free", status))
			{
				if (data->toparent)
				{
					OBJHANDLE hParent = v->GetAttachmentStatus(attach);
					VESSEL * vessel = oapiGetVesselInterface(hParent);
					ATTACHMENTHANDLE hAttach;
					OBJHANDLE hTest = 0;
					for (int i = 0; i < vessel->AttachmentCount(false); i++)
					{
						hAttach=vessel->GetAttachmentHandle(false, i);
						hTest=vessel->GetAttachmentStatus(hAttach);
						if (!oapiIsVessel(hTest))
							continue;
						if (!strcmp(v->GetName(),VESSEL(hTest).GetName()))
						{
							vessel->DetachChild(hAttach,data->relVel);
							break;
						}
					}
				}
				else
				{
					v->DetachChild(attach,data->relVel);
				}
				return true;
			}
			ATTACHMENTSPEC * aspec = new ATTACHMENTSPEC();
			double distance = GetClosestAttachment(v->GetHandle(), attach, aspec, !data->toparent);
			if (!oapiIsVessel(aspec->hook))
			{
				delete aspec;
				return false;
			}
			if (distance>data->fAttachDistance)
			{
				delete aspec;
				return false;
			}
			VESSEL * vessel = oapiGetVesselInterface(aspec->hook);
			ATTACHMENTHANDLE attach2 = vessel->GetAttachmentHandle(!data->toparent, aspec->inx);
			if (data->toparent)
				vessel->AttachChild(v->GetHandle(), attach2, attach);
			else
				v->AttachChild(vessel->GetHandle(), attach, attach2);
			delete aspec;
			return true;
		}
	case OAPI_KEY_V:
		{
			oapiOpenInputBox("Release velocity: [m/s]", GetDouble, 0, 20, &data->relVel);
			return true;
		}
	}
	return false;
}
bool RAMFD::ConsumeButton (int bt, int event)
{
	switch(bt)
	{
	case 1:
		return ConsumeKeyBuffered(OAPI_KEY_LBRACKET);
	case 2:
		return ConsumeKeyBuffered(OAPI_KEY_RBRACKET);
	}
	if (!(event & PANEL_MOUSE_LBDOWN)) return false;
	
	switch(bt)
	{
	case 0:
		return ConsumeKeyBuffered(OAPI_KEY_EQUALS);
	case 3:
		return ConsumeKeyBuffered(OAPI_KEY_MINUS);
	case 4:
		return ConsumeKeyBuffered(OAPI_KEY_0);
	case 5:
		return ConsumeKeyBuffered(OAPI_KEY_R);
	case 6:
		return ConsumeKeyBuffered(OAPI_KEY_S);
	case 7:
		return ConsumeKeyBuffered(OAPI_KEY_T);
	case 8:
		return ConsumeKeyBuffered(OAPI_KEY_E);
	case 9:
		return ConsumeKeyBuffered(OAPI_KEY_Q);
	case 10:
		return ConsumeKeyBuffered(OAPI_KEY_W);
	case 11:
		return ConsumeKeyBuffered(OAPI_KEY_A);
	}
	return false;
}
char *RAMFD::ButtonLabel (int bt)
{
	char *label[12] = {"TGT", "A+", "A-", "AN+", "AN-", "RES", "SPD","TYP","RAN", "AT+", "AT-","ATT"};
	return (bt < 12 ? label[bt] : 0);
}
int RAMFD::ButtonMenu (const MFDBUTTONMENU **menu) const
{
	static const MFDBUTTONMENU mnu[12] = {
		{"Next Target", 0, '='},
		{"Animate forward", 0, '['},
		{"Animate backward", 0, ']'},
		{"Next animation", 0, '-'},
		{"Previous animation", 0, '0'},
		{"Reset", 0, 'R'},
		{"Change Speed", 0, 'S'},
		{"Attachment Type", 0, 'T'},
		{"Attachment Range", 0, 'E'},
		{"Next attachment", 0, 'Q'},
		{"Prev. attachment", 0, 'W'},
		{"Attach/Detach", 0, 'A'}
	};
	if (menu) *menu = mnu;
	return 12;
}
void RAMFD::Update (HDC hDC)
{
	Title (hDC, "Remote Animation MFD");

	double y = height;
	char cbuf[255];
	int len;

	if (g_VobjVector.size() < 1)
	{
		len = sprintf(cbuf,"No Registered Vessels");
		TextOut(hDC,width,y, cbuf,len);
		return;
	}
	if (data->iSelected >= g_VobjVector.size())
	{
		data->iSelected=0;
	}
	VOBJ * vbj = g_VobjVector[data->iSelected];
	VESSEL * v = oapiGetVesselInterface(g_VobjVector[data->iSelected]->hook);
	len = sprintf(cbuf,"Target: %s",v->GetName());
	TextOut(hDC,width,y,cbuf,len);
	y+=height*1;
	if (vbj->av.size() > 0)
	{
		len = sprintf(cbuf,"Selected Animation: %s %d",vbj->av[data->iSelectedAnimation].name, data->iSelectedAnimation+1);
		TextOut(hDC,width,y,cbuf,len);
		y+=height*1;
		len = sprintf(cbuf,"State: %.4f", vbj->av[data->iSelectedAnimation].state);
		TextOut(hDC,width, y, cbuf, len);
		y+=height*1;
	}
	len = sprintf(cbuf,"Speed: %.2f",data->animSpeed);
	TextOut(hDC,width,y,cbuf,len);
	y+=height*3;
	len = sprintf(cbuf,"Attachment Range: %.2f",data->fAttachDistance);
	TextOut(hDC,width,y, cbuf, len);
	y+=height*1;
	if (data->toparent)
		len = sprintf(cbuf,"Attachment Type: Child");
	else
		len = sprintf(cbuf,"Attachment Type: Parent");
	TextOut(hDC,width, y, cbuf, len);
	y+=height*1;
	DWORD aCount=v->AttachmentCount(data->toparent);
	if (aCount)
		len = sprintf(cbuf,"Selected Attachment: %d of %d",data->iSelectedAttachment+1,aCount);
	else
		len = sprintf(cbuf,"Selected Attachment: None");
	TextOut(hDC,width, y, cbuf,len);
	y+=height*1;

	if (aCount)
	{
		char status[100];
	    GetAttachmentStatusString(v->GetHandle(),v->GetAttachmentHandle(data->toparent,data->iSelectedAttachment), status);
	    len = sprintf(cbuf,"Status: %s",status);
	    TextOut(hDC,width, y, cbuf, len);
		y+=height*1;
	}
	len = sprintf(cbuf,"Release vel.: %.2f", data->relVel);
	TextOut(hDC,width, y, cbuf, len);
}
DLLCLBK void opcSaveState (FILEHANDLE scn)
{
	char fname[255];
	GetFileName(fname);
	char sep[] = ";-----------------------------------------;";
	char vname[255];
	VOBJ * vbj=NULL;
	VESSEL * v = NULL;
	LOGGER log(fname, "write");

	log.write(sep);
	sprintf(log.line(),";RAMFD - State File MJD: %.6f Sim: %.0f", oapiGetSimMJD(), oapiGetSimTime());
	log.write();
	log.write(sep);

	int vCount=0;
	for (int i = 0; i < g_VobjVector.size(); i++)
	{
		if (!g_VobjVector[i])
			continue;
		if (!oapiIsVessel(g_VobjVector[i]->hook))
			continue;
		vbj = g_VobjVector[i];
		v = oapiGetVesselInterface(vbj->hook);
		if (vbj->av.size() < 1)
			continue;
		sprintf(vname, "%s", v->GetName());
		log.write(sep);
		sprintf(log.line(),"Vessel_%d_Name = %s", vCount+1, vname);
		log.write();
		for (int j = 0; j < vbj->av.size(); j++)
		{
			sprintf(log.line(), "%s_Animation_%d_State = %.4f", vname, j+1, vbj->av[j].state);
			log.write();
		}
		vCount++;
	}
	sprintf(log.line(), "VesselCount = %d", vCount);
	log.write();
	oapiWriteScenario_string(scn, "STATEFILE", fname);
}
DLLCLBK void opcLoadState (FILEHANDLE scn)
{
	char *line;
	char fname[255];
	while (oapiReadScenario_nextline (scn, line)) {
		if (!strnicmp (line, "STATEFILE", 9)) {
			sscanf (line+9, "%s", &fname);
			StateFileFound=TRUE;
			g_parser.read(fname);
		}
	}
}