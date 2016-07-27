#include "ThePlugin.h"
#include "CFont.h"
#include "CSprite2d.h"
#include "CTimer.h"
#include "CTxdStore.h"
#include "CCharTable.h"
#include "rwFunc.h"
#include "../include/hooking/Hooking.Patterns.h"

#include <cstring>

#include "../include/injector/injector.hpp"
#include "../include/injector/hooking.hpp"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

const wchar_t *ThePlugin::MessageboxTitle = L"《侠盗猎车手：罪恶都市》汉化补丁 Koishi(1.0) Build20160730 by 无名汉化组";

bool ThePlugin::CheckResourceFile(HMODULE hPlugin)
{
	char pluginPath[MAX_PATH];

	GetModuleFileNameA(hPlugin, pluginPath, MAX_PATH);
	strcpy(CFont::texturePath, pluginPath);
	strcpy(CFont::textPath, pluginPath);
	strcpy(strrchr(CFont::texturePath, '.'), "\\wm_vcchs.png");
	strcpy(strrchr(CFont::textPath, '.'), "\\wm_vcchs.gxt");

	if (!PathFileExistsA(CFont::texturePath) || !PathFileExistsA(CFont::textPath))
	{
		MessageBoxW(NULL, L"afafaf", MessageboxTitle, MB_OK);
		//资源文件不见了
		return false;
	}

	return true;
}

bool ThePlugin::CheckGameVersion()
{
	auto &veref = injector::address_manager::singleton();

	if (veref.IsVC())
	{
		CFont::GetAddresses();
		CSprite2d::GetAddresses();
		CTimer::GetAddresses();
		CTxdStore::GetAddresses();
		rwFunc::GetAddresses();

		CCharTable::InitTable();
		PatchGame();
	}
	else
	{
		//不支持的游戏版本
		return false;
	}

	return true;
}

__declspec(naked) void Hook_LoadGxt1()
{
	__asm
	{
		pop eax;
		push offset CFont::textPath;
		jmp eax;
	}
}

char aRb[] = "rb";
__declspec(naked) void Hook_LoadGxt2()
{
	__asm
	{
		pop eax;
		push offset aRb;
		push offset CFont::textPath;
		jmp eax;
	}
}


void ThePlugin::PatchGame()
{
	unsigned __int8 *FEO_LAN_Entry = *hook::pattern("B8 ? ? ? ? 01 C8 B9 ? ? ? ?").get(0).get<unsigned __int8 *>(1) + 0x1816;
	memcpy(FEO_LAN_Entry, FEO_LAN_Entry + 0x12, 0x12);
	memcpy(FEO_LAN_Entry + 0x12, FEO_LAN_Entry + 0x24, 0x12);
	memset(FEO_LAN_Entry + 0x24, 0, 0x12);

	void *LoadGXTPattern1 = hook::pattern("8D 04 40 8B 5C 85 34 8D 84 24 80 00 00 00 50").get(0).get(7);
	void *LoadGXTPattern2 = hook::pattern("8D 44 24 14 68 ? ? ? ? 50").get(0).get();
	injector::MakeNOP(LoadGXTPattern1, 8);
	injector::MakeCALL(LoadGXTPattern1, Hook_LoadGxt1);
	injector::MakeNOP(LoadGXTPattern2, 10);
	injector::MakeCALL(LoadGXTPattern2, Hook_LoadGxt2);

	injector::MakeCALL(hook::pattern("DB 05 ? ? ? ? 8D 4C 24 08").get(0).get(0xF8), CFont::LoadCHSTexture);
	injector::MakeCALL(hook::pattern("B9 ? ? ? ? E8 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 59 50 E8 ? ? ? ? 59 C3").get(0).get(0x20), CFont::UnloadCHSTexture);

	injector::MakeJMP(hook::pattern("D9 05 ? ? ? ? 53 56 83 EC 10").get(0).get(), CFont::GetStringWidth);
	injector::MakeJMP(hook::pattern("D9 05 ? ? ? ? 53 56 57 55 31 ED").get(0).get(), CFont::GetTextRect);
	injector::MakeJMP(hook::pattern("53 56 55 31 ED 83 EC 20").get(0).get(), CFont::GetNumberLines);
	injector::MakeJMP(hook::pattern("53 56 57 55 83 EC 38 8B 74 24 54").get(0).get(), CFont::PrintString);
	injector::MakeJMP(hook::pattern("53 56 55 83 EC 18 81 3D ? ? ? ? ? ? ? ?").get(0).get(), CFont::RenderFontBuffer);

	injector::WriteMemory(*hook::pattern("D9 05 ? ? ? ? D8 44 24 10 50").get(0).get<float *>(2), 999.0f);
}
