#pragma once
#include <windows.h>

namespace ThePlugin
{
	extern char texturePath[];
	extern char slantTexturePath[];
	extern char textPath[];

	extern const wchar_t *MessageboxTitle;

	extern bool CheckResourceFile(HMODULE hPlugin);
	extern bool CheckGameVersion();

	extern void __cdecl LoadCHSTexture();
	extern void __cdecl UnloadCHSTexture(int slot);

	extern void PatchGame();
}
