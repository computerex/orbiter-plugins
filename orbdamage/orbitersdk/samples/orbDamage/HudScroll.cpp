#include "HudScroll.h"

void HUDSCROLL::insert(char *msg)
{
	for (int i = 0; i < m.size(); i++)
	{
		if (i == m.size()-1)
		{
			m[i]=msg;
			break;
		}
		m[i]=m[i+1];
	}
}
