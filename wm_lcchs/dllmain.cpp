#include <windows.h>
#include "ThePlugin.h"

BOOL WINAPI DllMain(HMODULE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		return (ThePlugin::CheckResourceFile(hDllHandle) && ThePlugin::CheckGameVersion());
	}

	return TRUE;
}
