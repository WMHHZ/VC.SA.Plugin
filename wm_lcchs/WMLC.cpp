#include "WMLC.h"
#include "CFont.h"
#include "CTxdStore.h"
#include "rwFunc.h"
#include "CCharTable.h"

#include "../include/injector/injector.hpp"
#include "../include/injector/hooking.hpp"

#include "../include/selector/AddressSelector.h"

#include <cstring>

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

char WMLC::texturePath[MAX_PATH];
char WMLC::textPath[MAX_PATH];

bool WMLC::CheckResourceFile(HMODULE hPlugin)
{
	char pluginPath[MAX_PATH];

	GetModuleFileNameA(hPlugin, pluginPath, MAX_PATH);
	std::strcpy(texturePath, pluginPath);
	std::strcpy(textPath, pluginPath);
	std::strcpy(std::strrchr(texturePath, '.'), "\\wm_lcchs.txd");
	std::strcpy(std::strrchr(textPath, '.'), "\\wm_lcchs.gxt");

	if (!PathFileExistsA(texturePath) || !PathFileExistsA(textPath))
	{
		MessageBoxW(nullptr, L"找不到资源文件，请确认是否带上了wm_lcchs文件夹！", WMVERSIONWSTRING, MB_ICONWARNING);
		//资源文件不见了
		return false;
	}

	return true;
}

bool WMLC::CheckGameVersion()
{
	auto &veref = injector::address_manager::singleton();

	if (veref.IsIII() && (veref.GetMinorVersion() == 0 || veref.IsSteam()))
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
		MessageBoxW(nullptr, L"你正在使用的游戏版本不被支持！请确保你的游戏主程序为以下之一：\n1.0美版：2383872字节", WMVERSIONWSTRING, MB_ICONWARNING);
		//不支持的游戏版本
		return false;
	}

	return true;
}

void WMLC::LoadCHSTexture()
{
	CTxdStore::PopCurrentTxd();
	int slot = CTxdStore::AddTxdSlot("wm_lcchs");
	CTxdStore::LoadTxd(slot, texturePath);
	CTxdStore::AddRef(slot);
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(slot);
	CFont::ChsSprite.SetTexture("chs_normal", "chs_normal_mask");
	CFont::ChsSlantSprite.SetTexture("chs_slant", "chs_slant_mask");
	CTxdStore::PopCurrentTxd();
}

void WMLC::UnloadCHSTexture(int dummy)
{
	CTxdStore::RemoveTxdSlot(dummy);
	CTxdStore::RemoveTxdSlot(CTxdStore::FindTxdSlot("wm_lcchs"));
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
		push offset WMLC::textPath;
		jmp eax;
	}
}
struct
{
	void *retAddr1, *retAddr2, *retAddr3;
}jmpBack1;

__declspec(naked) void Hook_PrintString()
{
	__asm
	{
		mov ax, [edi];
		test ax, ax;
		jz ret1;
		cmp ax, 0x20;
		jz ret2;
		xor bl, bl;
		push 0;
		push esi;
		call CFont::GetStringWidth;
		add esp, 8;
		fadd dword ptr[esp + 8];
		fst dword ptr[esp + 8];
		fstp dword ptr[esp + 0x14];
		mov esi, edi;
		jmp jmpBack1.retAddr3;

	ret1:
		jmp jmpBack1.retAddr1;

	ret2:
		jmp jmpBack1.retAddr2;
	}
}

void WMLC::PatchGame()
{
	AddressSelectorLC selector;

	unsigned __int8 *FET_LAN_Entry = selector.SelectAddress<0x6157AC, 0x621F64, unsigned __int8>();
	std::memcpy(FET_LAN_Entry, FET_LAN_Entry + 0x14, 0x14);
	std::memcpy(FET_LAN_Entry + 0x14, FET_LAN_Entry + 0x28, 0x14);
	std::memset(FET_LAN_Entry + 0x28, 0, 0x14);

	injector::WriteMemory<unsigned __int32>(selector.SelectAddress<0x5082CF, 0x50833F>(), 255, true);
	injector::WriteMemory<unsigned __int8>(selector.SelectAddress<0x5082D4, 0x508344>(), 0, true);
	injector::WriteMemory<unsigned __int32>(selector.SelectAddress<0x5082D6, 0x508346>(), 0, true);
	injector::WriteMemory<unsigned __int8>(selector.SelectAddress<0x5082DB, 0x50834B>(), 0, true);

	injector::WriteMemory<__int16>(selector.SelectAddress<0x52B73A, 0x52B90A>(), 4, true);

	injector::MakeCALL(selector.SelectAddress<0x52C42F, 0x52C5FF>(), LoadCHSGXT);

	injector::MakeCALL(selector.SelectAddress<0x500B87, 0x500BF7>(), LoadCHSTexture);
	injector::MakeCALL(selector.SelectAddress<0x500BCA, 0x500C3A>(), UnloadCHSTexture);

	injector::MakeJMP(selector.SelectAddress<0x5018A0, 0x501910>(), CFont::GetStringWidth);
	injector::MakeJMP(selector.SelectAddress<0x501260, 0x5012D0>(), CFont::GetNumberLines);
	injector::MakeJMP(selector.SelectAddress<0x5013B0, 0x501420>(), CFont::GetTextRect);
	injector::MakeJMP(selector.SelectAddress<0x501960, 0x5019D0>(), CFont::GetNextSpace);
	injector::MakeJMP(selector.SelectAddress<0x501840, 0x5018B0>(), CFont::GetCharacterSize);

	injector::MakeCALL(selector.SelectAddress<0x50179F, 0x50180F>(), CFont::PrintCharDispatcher);

	jmpBack1.retAddr1 = selector.SelectAddress<0x50117C, 0x5011EC>();
	jmpBack1.retAddr2 = selector.SelectAddress<0x501167, 0x5011D7>();
	jmpBack1.retAddr3 = selector.SelectAddress<0x50123B, 0x5012AB>();
	injector::MakeJMP(selector.SelectAddress<0x50115F, 0x5011CF>(), Hook_PrintString);
}
