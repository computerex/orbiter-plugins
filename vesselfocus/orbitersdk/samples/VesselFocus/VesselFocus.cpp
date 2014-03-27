#define  STRICT
#define  ORBITER_MODULE

#include <orbitersdk.h>
#include <vector>
#include <fstream>
#include <string>

struct FPARAM {
	BOOL hasFocus;
	char className[64];
};
BOOL Init = FALSE;
std::vector<OBJHANDLE>    handles;
std::vector<FPARAM>       classes;


bool isSpacecraft(OBJHANDLE hVes)
{
	if (!oapiIsVessel(hVes))
		return false;
	VESSEL * v = oapiGetVesselInterface(hVes);
	if (v->GetClassName() == NULL)
		return true;
	if (!strnicmp(VESSEL(hVes).GetClassName(), "Spacecraft",10))
		return true;
	return false;
}
bool isRegistered(OBJHANDLE hVes)
{
	if (!oapiIsVessel(hVes))
		return false;
	for (int i = 0; i < handles.size(); i++)
	{
		if (!oapiIsVessel(handles[i]))
			continue;
		if (handles[i]==hVes)
			return true;
	}
	return false;
}
void Prune()
{
    if (handles.empty())
        return;
    std::vector<OBJHANDLE>::iterator it = handles.begin() + 1;
    for (int i = 0; i < handles.size(); i++)
    {
        it = handles.begin() + i;
        if (!oapiIsVessel(handles[i]))
        {
            handles.erase(it);
        }
    }
}
void Initialize()
{
	Init=TRUE;
	FPARAM param;
	std::string line;
	std::ifstream myfile ("Config/Vessels/FocusVessel.cfg");
	const char * pLine = NULL;
    if (myfile.is_open())
    {
        while (! myfile.eof() )
        {
			getline (myfile,line);
			if (line.length() < 1)
				continue;
			if (!strnicmp("FOCUS", line.c_str(), 5))
			{
				pLine = line.c_str()+6;
				sprintf(param.className,"%s", pLine);
				param.hasFocus=TRUE;
				classes.push_back(param);
			}
			else if (!strnicmp("UNFOCUS", line.c_str(), 7))
			{
				pLine = line.c_str()+8;
				sprintf(param.className,"%s", pLine);
				param.hasFocus=FALSE;
				classes.push_back(param);
			}
		}
        myfile.close();
    }
}
DLLCLBK void opcPreStep(double simt, double simdt, double mjd)
{
	if (Init==FALSE)
	{
		Initialize();
	}
	OBJHANDLE hVessel = NULL;
	VESSEL *  vessel  = NULL;
	for (int i = 0; i < oapiGetVesselCount(); i++)
	{
		hVessel=oapiGetVesselByIndex(i);
		if (!oapiIsVessel(hVessel))
			continue;
		if (isRegistered(hVessel))
			continue;
		vessel = oapiGetVesselInterface(hVessel);
		for (int j = 0; j < classes.size(); j++)
		{
			if (isSpacecraft(hVessel))
			{
				if (!strcmp(classes[j].className, vessel->GetName()))
				{
					handles.push_back(hVessel);
					vessel->SetEnableFocus(classes[j].hasFocus);
					break;
				}
			}else
			{
				if (!strcmp(classes[j].className, vessel->GetClassName()))
				{
					handles.push_back(hVessel);
					vessel->SetEnableFocus(classes[j].hasFocus);
					break;
				}
			}
		}
	}
	Prune();
	for (int i = 0; i < handles.size(); i++)
	{
		vessel = oapiGetVesselInterface(handles[i]);
		for (int j = 0; j < classes.size(); j++)
		{
			if (isSpacecraft(handles[i]))
			{
				if (!strcmp(classes[j].className,vessel->GetName())){
					vessel->SetEnableFocus(classes[j].hasFocus);
					break;
				}
			}else
			{
				if (!strcmp(classes[j].className,vessel->GetClassName())){
					vessel->SetEnableFocus(classes[j].hasFocus);
					break;
				}
			}
		}
	}
}