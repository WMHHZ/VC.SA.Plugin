#include "VCGXT.h"

int main()
{
	VCGXT temp;

	if (temp.LoadText("GTAVC.txt"))
	{
		temp.SaveAsGXT("wm_vcchs.gxt");
		temp.GenerateWMHHZStuff();
	}

	return 0;
}
