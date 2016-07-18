#pragma warning(disable:4996)

#include "CPlugin.h"
#include "CFont.h"
#include "CSprite2d.h"
#include "CTimer.h"
#include "CGame.h"
#include "CTxdStore.h"
#include "CCharTable.h"
#include "rwFunc.h"

#include <cstring>

#include "../include/injector/injector.hpp"
#include "../include/injector/hooking.hpp"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

char CPlugin::texturePath[MAX_PATH];
char CPlugin::textPath[MAX_PATH];

const wchar_t *CPlugin::MessageboxTitle = L"《侠盗猎车手：罪恶都市》汉化补丁 Koishi(1.0) Build160820 by 无名汉化组";

bool CPlugin::CheckResourceFile(HMODULE hPlugin)
{
	char pluginPath[MAX_PATH];

	GetModuleFileNameA(hPlugin, pluginPath, 260);
	strcpy(texturePath, pluginPath);
	strcpy(textPath, pluginPath);
	strcpy(strrchr(texturePath, '.'), "\\wm_vcchs.png");
	strcpy(strrchr(textPath, '.'), "\\wm_vcchs.gxt");

	if (!PathFileExistsA(texturePath) || !PathFileExistsA(textPath))
	{
		MessageBoxW(NULL, L"afafaf", MessageboxTitle, MB_OK);
		//资源文件不见了
		return false;
	}

	return true;
}

bool CPlugin::CheckGameVersion()
{
	auto &veref = injector::address_manager::singleton();

	if (veref.IsVC())
	{
		if (veref.IsSteam())
		{
			CFont::InitSteam();
			CSprite2d::InitSteam();
			CTimer::InitSteam();
			CGame::InitSteam();
			CTxdStore::InitSteam();
			rwFunc::InitSteam();

			CreateThread(0, 0, SteamPatchThread, NULL, 0, NULL);
		}
		else
		{
			CFont::Init10();
			CSprite2d::Init10();
			CTimer::Init10();
			CGame::Init10();
			CTxdStore::Init10();
			rwFunc::Init10();

			Patch10();
		}

		CCharTable::InitTable();
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
		push offset CPlugin::textPath;
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
		push offset CPlugin::textPath;
		jmp eax;
	}
}

void CPlugin::LoadCHSTexture()
{
	CTxdStore::PopCurrentTxd();

	CFont::m_Sprites[2].m_pRwTexture = rwFunc::LoadTextureFromPNG(texturePath);
}

void CPlugin::UnloadCHSTexture(int dummy)
{
	CTxdStore::RemoveTxdSlot(dummy);

	CFont::m_Sprites[2].Delete();
}

void CPlugin::Patch10()
{
	memcpy((void *)(0x6DA386), (void *)(0x6DA386 + 0x12), 0x12);
	memcpy((void *)(0x6DA386 + 0x12), (void *)(0x6DA386 + 0x24), 0x12);
	memset((void *)(0x6DA386 + 0x24), 0, 0x12);

	injector::MakeNOP(0x5852A0, 8);
	injector::MakeCALL(0x5852A0, Hook_LoadGxt1);
	injector::MakeNOP(0x5855EE, 10);
	injector::MakeCALL(0x5855EE, Hook_LoadGxt2);

	injector::MakeCALL(0x552461, LoadCHSTexture);
	injector::MakeCALL(0x552300, UnloadCHSTexture);

	injector::MakeJMP(0x550650, CFont::GetStringWidth);
	injector::MakeJMP(0x550720, CFont::GetTextRect);
	injector::MakeJMP(0x550C70, CFont::GetNumberLines);
	injector::MakeJMP(0x551040, CFont::PrintString);
	injector::MakeJMP(0x551A30, CFont::RenderFontBuffer);
}

DWORD CPlugin::SteamPatchThread(LPVOID)
{
	do
	{
		Sleep(0);
	} while (injector::ReadMemory<unsigned __int32>(0x550542, true) != 0x6961D0);
	
	memcpy((void *)(0x6DA386 - 0x1030), (void *)(0x6DA386 + 0x12 - 0x1030), 0x12);
	memcpy((void *)(0x6DA386 + 0x12 - 0x1030), (void *)(0x6DA386 + 0x24 - 0x1030), 0x12);
	memset((void *)(0x6DA386 + 0x24 - 0x1030), 0, 0x12);

	injector::MakeNOP(0x5852A0 - 0x1D0, 8);
	injector::MakeCALL(0x5852A0 - 0x1D0, Hook_LoadGxt1);
	injector::MakeNOP(0x5855EE - 0x1D0, 10);
	injector::MakeCALL(0x5855EE - 0x1D0, Hook_LoadGxt2);

	injector::MakeCALL(0x552461 - 0x110, LoadCHSTexture);
	injector::MakeCALL(0x552300 - 0x110, UnloadCHSTexture);

	injector::MakeJMP(0x550650 - 0x110, CFont::GetStringWidth);
	injector::MakeJMP(0x550720 - 0x110, CFont::GetTextRect);
	injector::MakeJMP(0x550C70 - 0x110, CFont::GetNumberLines);
	injector::MakeJMP(0x551040 - 0x110, CFont::PrintString);
	injector::MakeJMP(0x551A30 - 0x110, CFont::RenderFontBuffer);

	return 0;
}
