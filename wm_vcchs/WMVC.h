#pragma once
#include <windows.h>

#define WMVERSIONWSTRING L"《侠盗猎车手：罪恶都市》汉化补丁 Koishi(1.0) Build20180920 by 无名汉化组"

class WMVC
{
public:
    static void MakeResourcePath(HMODULE module);
    static void PatchGame();
};
