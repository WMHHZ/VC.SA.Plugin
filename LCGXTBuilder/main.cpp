#include "LCGXT.h"

int main()
{
	LCGXT temp;

	if (temp.LoadText("GTA3.txt"))
	{
		temp.SaveAsGXT("wm_lcchs.gxt");
		temp.GenerateWMHHZStuff();
	}
	return 0;
}
