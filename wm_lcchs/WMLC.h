#pragma once
#include <windows.h>

#define WMVERSIONSTRING u8"《侠盗猎车手3》汉化补丁 v1.0 Build2016XXXX"
#define WMVERSIONWSTRING L"《侠盗猎车手3》汉化补丁 v1.0 Build2016XXXX"

class WMLC
{
public:
	static char texturePath[];
	static char textPath[];

	static bool CheckResourceFile(HMODULE hPlugin);
	static bool CheckGameVersion();

	static void __cdecl LoadCHSTexture();
	static void __cdecl UnloadCHSTexture(int slot);

	static void PatchGame();
};
