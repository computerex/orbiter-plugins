#include <windows.h>
#include "FileReader.h"

void FILEREADER::LoadInt(char *inifile, char *inisection, char *inikey, int &inivalue)
{
	inivalue = GetPrivateProfileInt(inisection,inikey,inivalue,inifile);
}
void FILEREADER::LoadDouble(char *inifile, char *inisection, char *inikey, double &inivalue)
{
	const DWORD IniSize = 255;
	DWORD IniReturn;
	char IniBuffer[IniSize+1];
	char dblBuffer[20];

	IniReturn = GetPrivateProfileString(inisection,inikey,"",IniBuffer,IniSize,inifile);
	if (IniReturn > 0)
	{
		sscanf(IniBuffer,"%s",&dblBuffer);
		inivalue = atof(dblBuffer);
	}
}
void FILEREADER::LoadVector(char *inifile, char *inisection, char *inikey, VECTOR3 &inivalue)
{
	const DWORD IniSize = 255;
	DWORD IniReturn;
	char IniBuffer[IniSize+1];
	char dblBufferIni[60];
	char dblBufferX[20];
	char dblBufferY[20];
	char dblBufferZ[20];

	try
	{
		sprintf(dblBufferIni,"(%.8E,%.8E,%.8E)", inivalue.x,inivalue.y,inivalue.z);
	}
	catch (...)
	{
		dblBufferIni[0] = 0;
	}

	IniReturn = GetPrivateProfileString(inisection,inikey,dblBufferIni,IniBuffer,IniSize,inifile);
	sscanf(IniBuffer,"(%[^','],%[^','],%[^')']",&dblBufferX,&dblBufferY,&dblBufferZ);
	inivalue.x = atof(dblBufferX);
	inivalue.y = atof(dblBufferY);
	inivalue.z = atof(dblBufferZ);
}

void FILEREADER::LoadString(char *inifile, char *inisection, char *inikey, char *inivalue)
{
	const DWORD IniSize = 255;
	DWORD IniReturn;
	char IniBuffer[IniSize+1];

	IniReturn = GetPrivateProfileString(inisection,inikey,inivalue,IniBuffer,IniSize,inifile);
	strcpy(inivalue,IniBuffer);
}
