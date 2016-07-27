#pragma once
#include <windows.h>

class ThePlugin
{
public:


	static const wchar_t *MessageboxTitle;

	static bool CheckResourceFile(HMODULE hPlugin);
	static bool CheckGameVersion();



	static void PatchGame();
};
