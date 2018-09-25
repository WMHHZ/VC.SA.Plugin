#include <windows.h>
#include "WMVC.h"
#include "CCharTable.h"

BOOL WINAPI DllMain(HMODULE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        if (WMVC::WaitForDecrypt())
        {
            WMVC::MakeResourcePath(hDllHandle);
            CCharTable::ReadTable();
            WMVC::PatchGame();
        }
    }

    return TRUE;
}
