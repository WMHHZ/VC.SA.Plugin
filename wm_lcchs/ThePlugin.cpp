#include "ThePlugin.h"
#include "CFont.h"
#include "CTxdStore.h"
#include "rwFunc.h"
#include "CCharTable.h"
#include "../include/hooking/Hooking.Patterns.h"

#include "../include/injector/injector.hpp"
#include "../include/injector/hooking.hpp"

#include <cstring>

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

char ThePlugin::texturePath[MAX_PATH];
char ThePlugin::slantTexturePath[MAX_PATH];
char ThePlugin::textPath[MAX_PATH];

const wchar_t *ThePlugin::MessageboxTitle = L"《侠盗猎车手3》汉化补丁 v1.0 Build2016XXXX";

bool ThePlugin::CheckResourceFile(HMODULE hPlugin)
{
	char pluginPath[MAX_PATH];

	GetModuleFileNameA(hPlugin, pluginPath, MAX_PATH);
	std::strcpy(texturePath, pluginPath);
	std::strcpy(slantTexturePath, pluginPath);
	std::strcpy(textPath, pluginPath);
	std::strcpy(std::strrchr(texturePath, '.'), "\\wm_lcchs.png");
	std::strcpy(std::strrchr(slantTexturePath, '.'), "\\wm_lcchs_slant.png");
	std::strcpy(std::strrchr(textPath, '.'), "\\wm_lcchs.gxt");

	if (!PathFileExistsA(texturePath) || !PathFileExistsA(slantTexturePath) || !PathFileExistsA(textPath))
	{
		MessageBoxW(NULL, L"afafaf", MessageboxTitle, MB_OK);
		//资源文件不见了
		return true;
	}

	return true;
}

bool ThePlugin::CheckGameVersion()
{
	auto &veref = injector::address_manager::singleton();

	if (veref.IsIII())
	{
		CFont::GetAddresses();
		CSprite2d::GetAddresses();
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

void ThePlugin::LoadCHSTexture()
{
	CTxdStore::PopCurrentTxd();

	CFont::ChsSprite.SetRwTexture(rwFunc::LoadTextureFromPNG(texturePath));
	CFont::ChsSlantSprite.SetRwTexture(rwFunc::LoadTextureFromPNG(slantTexturePath));
}

void ThePlugin::UnloadCHSTexture(int slot)
{
	CTxdStore::RemoveTxdSlot(slot);

	CFont::ChsSprite.Delete();
	CFont::ChsSlantSprite.Delete();
}

__declspec(naked) void LoadCHSGXT()
{
	__asm
	{
		pop eax;
		add eax, 2;
		push 0x40000;
		push esi;
		push offset ThePlugin::textPath;
		jmp eax;
	}
}

void ThePlugin::PatchGame()
{
	unsigned __int8 *FET_LAN_Entry = *hook::pattern("6B C0 61 80 3C 85 ? ? ? ? 00").get(0).get<unsigned __int8 *>(6) + 0x3E7C;
	std::memcpy(FET_LAN_Entry, FET_LAN_Entry + 0x14, 0x14);
	std::memcpy(FET_LAN_Entry + 0x14, FET_LAN_Entry + 0x28, 0x14);
	std::memset(FET_LAN_Entry + 0x28, 0, 0x14);

	hook::pattern pagerColorPattern("68 CD 00 00 00 6A 42 68 A2 00 00 00 6A 20");
	injector::WriteMemory<unsigned __int32>(pagerColorPattern.get(0).get(1), 255, true);
	injector::WriteMemory<unsigned __int8>(pagerColorPattern.get(0).get(6), 0, true);
	injector::WriteMemory<unsigned __int32>(pagerColorPattern.get(0).get(8), 0, true);
	injector::WriteMemory<unsigned __int8>(pagerColorPattern.get(0).get(13), 0, true);

	injector::WriteMemory<__int16>(hook::pattern("66 C7 01 08 00").get(0).get(3), 4, true);

	injector::MakeCALL(hook::pattern("68 00 00 04 00 56 50").get(0).get(), LoadCHSGXT);

	injector::MakeCALL(hook::pattern("53 83 EC 08 68 ? ? ? ?").get(0).get(0x147), LoadCHSTexture);
	injector::MakeCALL(hook::pattern("B9 ? ? ? ? E8 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 59 50 E8 ? ? ? ?").get(0).get(0x16), UnloadCHSTexture);

	injector::MakeJMP(hook::pattern("0F BF 15 ? ? ? ? 53 56 83 EC 08").get(0).get(), CFont::GetStringWidth);
	injector::MakeJMP(hook::pattern("53 56 55 31 ED 83 EC 10 80 3D ? ? ? ? 01").get(0).get(), CFont::GetNumberLines);
	injector::MakeJMP(hook::pattern("53 56 57 55 83 EC 38 8D 4C 24 28").get(0).get(), CFont::PrintString);
}
