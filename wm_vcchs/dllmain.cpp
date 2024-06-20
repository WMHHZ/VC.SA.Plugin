#include "CCharTable.h"
#include "WMVC.h"
#include <windows.h>


BOOL WINAPI DllMain(HMODULE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        WMVC::MakeResourcePath(hDllHandle);
        CCharTable::ReadTable();
        WMVC::PatchGame();
    }

    return TRUE;
}
