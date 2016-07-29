#pragma once
#include <windows.h>

#define WMVERSIONWSTRING L"《侠盗猎车手3》汉化补丁 Satori(1.0) Build20160729 by 无名汉化组"

class WMLC
{
public:
	static bool CheckResourceFile(HMODULE hPlugin);
	static bool CheckGameVersion();

	static void PatchGame();
};
