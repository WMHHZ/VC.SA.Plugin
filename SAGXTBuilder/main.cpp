#include "SAGXT.h"

int main()
{
	SAGXT temp;

	if (temp.LoadText("GTASA.txt"))
	{
		temp.SaveAsGXT("wm_sachs.gxt");
		temp.GenerateWMHHZStuff();
	}

	return 0;
}
