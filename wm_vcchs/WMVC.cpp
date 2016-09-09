#include "WMVC.h"
#include "CFont.h"
#include "CCharTable.h"

#include <cstring>

#include "../deps/injector/gvm/gvm.hpp"
#include "../deps/injector/hooking.hpp"
#include "../deps/selector/AddressSelector.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

bool WMVC::CheckResourceFile(HMODULE hPlugin)
{
	char pluginPath[260];

	GetModuleFileNameA(hPlugin, pluginPath, 260);
	std::strcpy(CFont::fontPath, pluginPath);
	std::strcpy(CFont::textPath, pluginPath);
	std::strcpy(CCharTable::datPath, pluginPath);

	std::strcpy(std::strrchr(CFont::fontPath, '.'), "\\wm_vcchs.txd");
	std::strcpy(std::strrchr(CFont::textPath, '.'), "\\wm_vcchs.gxt");
	std::strcpy(std::strrchr(CCharTable::datPath, '.'), "\\wm_vcchs.dat");

	if (!PathFileExistsA(CFont::fontPath) || !PathFileExistsA(CFont::textPath) || !PathFileExistsA(CCharTable::datPath))
	{
		MessageBoxW(nullptr, L"找不到资源文件，请确认是否带上了wm_vcchs文件夹！", WMVERSIONWSTRING, MB_ICONERROR);
		return false;
	}

	return true;
}

bool WMVC::CheckGameVersion()
{
	auto &veref = injector::address_manager::singleton();

	if (veref.IsVC() && (veref.IsSteam() || veref.GetMinorVersion() == 0))
	{
		CCharTable::ReadTable();
		PatchGame();
	}
	else
	{
		MessageBoxW(nullptr, L"你正在使用的游戏版本不被支持！请确保你的游戏主程序为以下之一：\n1.0：3088896字节\nSteam：3615744字节", WMVERSIONWSTRING, MB_ICONERROR);
		return false;
	}

	return true;
}

__declspec(naked) void hook_load_gxt_mission()
{
	__asm
	{
		pop eax;
		add eax, 3;
		push offset CFont::textPath;
		jmp eax;
	}
}

char aRb[] = "rb";
__declspec(naked) void hook_load_gxt()
{
	__asm
	{
		pop eax;
		add eax, 5;
		push offset aRb;
		push offset CFont::textPath;
		jmp eax;
	}
}

void WMVC::PatchGame()
{
	unsigned __int8 *FEO_LAN_Entry = AddressSelectorVC::SelectAddress<0x6DA386, 0x0, 0x6D9356, unsigned __int8>();
	memcpy(FEO_LAN_Entry, FEO_LAN_Entry + 0x12, 0x12);
	memcpy(FEO_LAN_Entry + 0x12, FEO_LAN_Entry + 0x24, 0x12);
	memset(FEO_LAN_Entry + 0x24, 0, 0x12);

	injector::MakeCALL(AddressSelectorVC::SelectAddress<0x5852A0, 0x0, 0x5850D0>(), hook_load_gxt_mission);
	injector::MakeCALL(AddressSelectorVC::SelectAddress<0x5855EE, 0x0, 0x58541E>(), hook_load_gxt);

	injector::MakeCALL(AddressSelectorVC::SelectAddress<0x552461, 0x0, 0x552351>(), CFont::LoadCHSFont);
	injector::MakeCALL(AddressSelectorVC::SelectAddress<0x552300, 0x0, 0x5521F0>(), CFont::UnloadCHSFont);

	injector::MakeJMP(AddressSelectorVC::SelectAddress<0x550650, 0x0, 0x550540>(), CFont::GetStringWidth);
	injector::MakeJMP(AddressSelectorVC::SelectAddress<0x550720, 0x0, 0x550610>(), CFont::GetTextRect);
	injector::MakeJMP(AddressSelectorVC::SelectAddress<0x550C70, 0x0, 0x550B60>(), CFont::GetNumberLines);

	injector::MakeJMP(AddressSelectorVC::SelectAddress<0x551040, 0x0, 0x550F30>(), CFont::PrintString);
	injector::MakeJMP(AddressSelectorVC::SelectAddress<0x551A30, 0x0, 0x551920>(), CFont::RenderFontBuffer);

	injector::WriteMemory(AddressSelectorVC::SelectAddress<0x68FD58, 0x0, 0x68ED60, float>(), 999.0f);

	unsigned __int8 *SpaceAddInstr = AddressSelectorVC::SelectAddress<0x6161BB, 0x0, 0x615DDB, unsigned __int8>();
	injector::MakeNOP(SpaceAddInstr, 6);
	injector::MakeNOP(SpaceAddInstr + 8);
	injector::MakeNOP(SpaceAddInstr + 0x22);
	injector::MakeNOP(SpaceAddInstr + 0x3D);
	injector::MakeNOP(SpaceAddInstr + 0x42, 6);
	injector::WriteMemory<unsigned __int8>(SpaceAddInstr + 0x65, 1, true);
	injector::MakeNOP(SpaceAddInstr + 0x72, 6);
}
 