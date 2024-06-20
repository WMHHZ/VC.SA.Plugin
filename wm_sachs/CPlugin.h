#pragma once

#include <windows.h>

struct RwTexture;

class CPlugin
{
  public:
    static char           texturePath[];
    static char           textPath[];
    static const wchar_t *MessageboxTitle;

    static bool Init(HMODULE hPlugin);

    static void Patch10U();

    static void __cdecl LoadCHSTexture();
    static void __cdecl UnloadCHSTexture(int dummy);
};
