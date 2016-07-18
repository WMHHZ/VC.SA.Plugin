#pragma once
#include <windows.h>

class ThePlugin
{
public:
	static char texturePath[];
	static char textPath[];

	static const wchar_t *MessageboxTitle;

	static bool CheckResourceFile(HMODULE hPlugin);
	static bool CheckGameVersion();

	static void __cdecl LoadCHSTexture();
	static void __cdecl UnloadCHSTexture(int slot);

	static void PatchGame();
};
