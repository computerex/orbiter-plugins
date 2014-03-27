#include <orbitersdk.h>
#include "VOBJ.h"


void VOBJ::Render(void)
{
	static PARTICLESTREAMSPEC dust = {
		0, 1.0, 5, 50, 0.3, 2.0, 3, 2.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_SQRT, 0, 1,
		PARTICLESTREAMSPEC::ATM_PLOG, -0.1, 0.1
	};

	if (INIT == FALSE)
	{
		INIT = TRUE;
	}
	if (pHandle)
		return;
	if (!th)
		return;
	pHandle = vessel->AddExhaustStream(th,&dust);
}
void VOBJ::Del(void)
{
	if (!pHandle)
		return;
	vessel->DelExhaustStream(pHandle);
	pHandle = 0;
}