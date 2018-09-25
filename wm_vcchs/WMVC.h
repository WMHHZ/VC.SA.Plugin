#pragma once
#include "stdinc.h"

class WMVC
{
public:
    static bool WaitForDecrypt();
    static injector::auto_pointer SelectAddress(unsigned int addr10, unsigned int addr11, unsigned int addrsteam);
    static void MakeResourcePath(HMODULE module);
    static void PatchGame();
};
