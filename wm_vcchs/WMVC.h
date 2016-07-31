#pragma once
#include <windows.h>

#define WMVERSIONWSTRING L"《侠盗猎车手：罪恶都市》汉化补丁 Koishi(1.0) Build20160730 by 无名汉化组"

class WMVC
{
public:
	static bool CheckResourceFile(HMODULE hPlugin);
	static bool CheckGameVersion();

	static void PatchGame();
};
