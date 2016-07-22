#include <windows.h>
#include "WMLC.h"

BOOL WINAPI DllMain(HMODULE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		return (WMLC::CheckResourceFile(hDllHandle) && WMLC::CheckGameVersion());
	}

	return TRUE;
}
