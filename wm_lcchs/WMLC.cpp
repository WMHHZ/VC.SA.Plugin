#include "WMLC.h"
#include "CFont.h"
#include "CTxdStore.h"
#include "rwFunc.h"
#include "CCharTable.h"
#include "CFont.h"
#include "../include/injector/injector.hpp"
#include "../include/injector/hooking.hpp"

#include "../include/selector/AddressSelector.h"

#include <cstring>

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")


bool WMLC::CheckResourceFile(HMODULE hPlugin)
{
	char pluginPath[MAX_PATH];

	GetModuleFileNameA(hPlugin, pluginPath, MAX_PATH);
	std::strcpy(CFont::texturePath, pluginPath);
	std::strcpy(CFont::textPath, pluginPath);
	std::strcpy(std::strrchr(CFont::texturePath, '.'), "\\wm_lcchs.txd");
	std::strcpy(std::strrchr(CFont::textPath, '.'), "\\wm_lcchs.gxt");

	if (!PathFileExistsA(CFont::texturePath) || !PathFileExistsA(CFont::textPath))
	{
		MessageBoxW(nullptr, L"找不到资源文件，请确认是否带上了wm_lcchs文件夹！", WMVERSIONWSTRING, MB_ICONWARNING);
		return false;
	}

	return true;
}

bool WMLC::CheckGameVersion()
{
	auto &veref = injector::address_manager::singleton();

	if (veref.IsIII() && (veref.GetMinorVersion() == 0 || veref.IsSteam()))
	{
		PatchGame();
	}
	else
	{
		MessageBoxW(nullptr, L"你正在使用的游戏版本不被支持！请确保你的游戏主程序为以下之一：\n1.0：2383872字节\nSteam：2801664字节", WMVERSIONWSTRING, MB_ICONWARNING);
		return false;
	}

	return true;
}

__declspec(naked) void LoadCHSGXT()
{
	__asm
	{
		pop eax;
		add eax, 2;
		push 0x40000;
		push esi;
		push offset CFont::textPath;
		jmp eax;
	}
}
struct
{
	void *retAddr1, *retAddr2, *retAddr3;
}jmpBack1;

__declspec(naked) void PrintStringFix()
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

unsigned __int16 *ContinousTokensFix(unsigned __int16 *arg_text, unsigned __int16 *useless)
{
	while (*arg_text == '~')
	{
		arg_text = CFont::ParseToken(arg_text, useless);
	}

	return arg_text;
}

void WMLC::PatchGame()
{
	unsigned __int8 *FET_LAN_Entry = AddressSelectorLC::SelectAddress<0x6157AC, 0x0, 0x621F64, unsigned __int8>();
	std::memcpy(FET_LAN_Entry, FET_LAN_Entry + 0x14, 0x14);
	std::memcpy(FET_LAN_Entry + 0x14, FET_LAN_Entry + 0x28, 0x14);
	std::memset(FET_LAN_Entry + 0x28, 0, 0x14);

	injector::WriteMemory<unsigned __int32>(AddressSelectorLC::SelectAddress<0x5082CF, 0x0, 0x50833F>(), 255, true);
	injector::WriteMemory<unsigned __int8>(AddressSelectorLC::SelectAddress<0x5082D4, 0x0, 0x508344>(), 0, true);
	injector::WriteMemory<unsigned __int32>(AddressSelectorLC::SelectAddress<0x5082D6, 0x0, 0x508346>(), 0, true);
	injector::WriteMemory<unsigned __int8>(AddressSelectorLC::SelectAddress<0x5082DB, 0x0, 0x50834B>(), 0, true);

	injector::WriteMemory<__int16>(AddressSelectorLC::SelectAddress<0x52B73A, 0x0, 0x52B90A>(), 5, true);

	injector::MakeCALL(AddressSelectorLC::SelectAddress<0x52C42F, 0x0, 0x52C5FF>(), LoadCHSGXT);

	injector::MakeCALL(AddressSelectorLC::SelectAddress<0x500B87, 0x0, 0x500BF7>(), CFont::LoadCHSTexture);
	injector::MakeCALL(AddressSelectorLC::SelectAddress<0x500BCA, 0x0, 0x500C3A>(), CFont::UnloadCHSTexture);

	injector::MakeJMP(AddressSelectorLC::SelectAddress<0x5018A0, 0x0, 0x501910>(), CFont::GetStringWidth);
	injector::MakeJMP(AddressSelectorLC::SelectAddress<0x501260, 0x0, 0x5012D0>(), CFont::GetNumberLines);
	injector::MakeJMP(AddressSelectorLC::SelectAddress<0x5013B0, 0x0, 0x501420>(), CFont::GetTextRect);
	injector::MakeJMP(AddressSelectorLC::SelectAddress<0x501960, 0x0, 0x5019D0>(), CFont::GetNextSpace);
	injector::MakeJMP(AddressSelectorLC::SelectAddress<0x501840, 0x0, 0x5018B0>(), CFont::GetCharacterSize);

	injector::MakeCALL(AddressSelectorLC::SelectAddress<0x50179F, 0x0, 0x50180F>(), CFont::PrintCharDispatcher);

	jmpBack1.retAddr1 = AddressSelectorLC::SelectAddress<0x50117C, 0x0, 0x5011EC>();
	jmpBack1.retAddr2 = AddressSelectorLC::SelectAddress<0x501167, 0x0, 0x5011D7>();
	jmpBack1.retAddr3 = AddressSelectorLC::SelectAddress<0x50123B, 0x0, 0x5012AB>();
	injector::MakeJMP(AddressSelectorLC::SelectAddress<0x50115F, 0x0, 0x5011CF>(), PrintStringFix);

	injector::MakeCALL(AddressSelectorLC::SelectAddress<0x501751, 0x0, 0x5017C1>(), ContinousTokensFix);

	injector::MakeNOP(AddressSelectorLC::SelectAddress<0x58F4EB, 0x0, 0x58F6CB>(), 6);
	injector::MakeNOP(AddressSelectorLC::SelectAddress<0x58F4F3, 0x0, 0x58F6D3>(), 1);
	injector::MakeNOP(AddressSelectorLC::SelectAddress<0x58F50D, 0x0, 0x58F6ED>(), 1);
	injector::MakeNOP(AddressSelectorLC::SelectAddress<0x58F528, 0x0, 0x58F708>(), 1);
	injector::MakeNOP(AddressSelectorLC::SelectAddress<0x58F52D, 0x0, 0x58F70D>(), 6);
	injector::MakeNOP(AddressSelectorLC::SelectAddress<0x58F55D, 0x0, 0x58F73D>(), 6);
}
