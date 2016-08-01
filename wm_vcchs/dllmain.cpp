#include <windows.h>

#include "WMVC.h"

BOOL WINAPI DllMain(HMODULE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		return (WMVC::CheckResourceFile(hDllHandle) && WMVC::CheckGameVersion());
	}

	return TRUE;
}
