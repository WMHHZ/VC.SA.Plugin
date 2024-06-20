#pragma once
#include <Windows.h>

class WMVC
{
  public:
    static void MakeResourcePath(HMODULE module);
    static void PatchGame();
};
