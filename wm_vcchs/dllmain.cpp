#include <windows.h>
#include "WMVC.h"
#include "CCharTable.h"

#define GAME_ID_GTAVC_1_0 0x74FF5064
#define GAME_ID_GTAVC_1_1 0x00408DC0
#define GAME_ID_GTAVC_STEAM 0x00004824
#define GAME_ID_GTAVC_STEAMENC 0x24E58287

static bool WaitForDecrypt()
{
    while (true)
    {
        Sleep(0);

        switch ((*(unsigned int *)0x61C11C))
        {
        case GAME_ID_GTAVC_1_0:
        case GAME_ID_GTAVC_1_1:
        case GAME_ID_GTAVC_STEAM:
            return true;

        case GAME_ID_GTAVC_STEAMENC:
            continue;

        default:
            return false;
        }
    }
}

BOOL WINAPI DllMain(HMODULE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        if (WaitForDecrypt())
        {
            WMVC::MakeResourcePath(hDllHandle);
            CCharTable::ReadTable();
            WMVC::PatchGame();
        }
    }

    return TRUE;
}
