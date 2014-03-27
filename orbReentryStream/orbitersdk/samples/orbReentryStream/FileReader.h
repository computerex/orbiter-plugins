#ifndef _H_COMPUTEREX_FILEREADER
#define _H_COMPUTEREX_FILEREADER

#include <windows.h>
#include "orbitersdk.h"

class FILEREADER
{
public:
	void LoadInt(char *inifile, char *inisection, char *inikey, int &inivalue);
    void LoadDouble(char *inifile, char *inisection, char *inikey, double &inivalue);
    void LoadVector(char *inifile, char *inisection, char *inikey, VECTOR3 &inivalue);
    void LoadString(char *inifile, char *inisection, char *inikey, char *inivalue);
};

#endif
