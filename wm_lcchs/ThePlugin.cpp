#include "ThePlugin.h"
#include "CFont.h"
#include "CSprite2d.h"
#include "CTxdStore.h"
#include "rwFunc.h"
#include "CCharTable.h"
#include "../include/hooking/Hooking.Patterns.h"

#include <cstring>

#include "../include/injector/injector.hpp"
#include "../include/injector/hooking.hpp"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

namespace ThePlugin
{
	char texturePath[MAX_PATH];
	char slantTexturePath[MAX_PATH];
	char textPath[MAX_PATH];

	const wchar_t *MessageboxTitle = L"《侠盗猎车手3》汉化补丁 v1.0 Build2016XXXX";

	bool CheckResourceFile(HMODULE hPlugin)
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
			return false;
		}

		return true;
	}

	bool CheckGameVersion()
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

	void LoadCHSTexture()
	{
		CTxdStore::PopCurrentTxd();

		CFont::ChsSprite.SetRwTexture(rwFunc::LoadTextureFromPNG(texturePath));
		CFont::ChsSlantSprite.SetRwTexture(rwFunc::LoadTextureFromPNG(slantTexturePath));
	}

	void UnloadCHSTexture(int slot)
	{
		CTxdStore::RemoveTxdSlot(slot);

		CFont::ChsSprite.Delete();
		CFont::ChsSlantSprite.Delete();
	}

	void PatchGame()
	{
		injector::MakeJMP(hook::pattern("89 04 24 DB 04 24 D8 0D ? ? ? ? DD D9 83 C4 08 C3").get(0).get(-0x45), CFont::GetCharacterSize);
		injector::MakeJMP(hook::pattern("0F BF 15 ? ? ? ? 53 56 83 EC 08").get(0).get(), CFont::GetStringWidth);
		injector::MakeJMP(hook::pattern("8B 44 24 04 EB 1F").get(0).get(), CFont::GetNextSpace);
		injector::MakeJMP(hook::pattern("53 56 55 31 ED 83 EC 10 80 3D ? ? ? ? 01").get(0).get(), CFont::GetNumberLines);
		injector::MakeJMP(hook::pattern("53 56 57 55 31 ED 83 EC 20 31 F6 80").get(0).get(), CFont::GetTextRect);
		injector::MakeJMP(hook::pattern("8B 15 ? ? ? ? 53 83 EC 48").get(0).get(), CFont::PrintChar);
		injector::MakeJMP(hook::pattern("53 56 57 55 83 EC 38 8D 4C 24 28").get(0).get(), CFont::PrintString);
	}
}
