#include "WMVC.h"
#include "CFont.h"

#include <cstring>

#include "../include/injector/gvm/gvm.hpp"
#include "../include/injector/hooking.hpp"
#include "../include/selector/AddressSelector.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

bool WMVC::CheckResourceFile(HMODULE hPlugin)
{
	char pluginPath[MAX_PATH];

	GetModuleFileNameA(hPlugin, pluginPath, MAX_PATH);
	strcpy(CFont::texturePath, pluginPath);
	strcpy(CFont::textPath, pluginPath);
	strcpy(strrchr(CFont::texturePath, '.'), "\\wm_vcchs.txd");
	strcpy(strrchr(CFont::textPath, '.'), "\\wm_vcchs.gxt");

	if (!PathFileExistsA(CFont::texturePath) || !PathFileExistsA(CFont::textPath))
	{
		MessageBoxW(NULL, L"afafaf", WMVERSIONWSTRING, MB_OK);
		//资源文件不见了
		return false;
	}

	return true;
}

bool WMVC::CheckGameVersion()
{
	auto &veref = injector::address_manager::singleton();

	if (veref.IsVC() && (veref.GetMinorVersion() != 1))
	{
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
		add eax, 3;
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

	injector::MakeCALL(AddressSelectorVC::SelectAddress<0x5852A0, 0x0, 0x5850D0>(), Hook_LoadGxt1);
	injector::MakeCALL(AddressSelectorVC::SelectAddress<0x5855EE, 0x0, 0x58541E>(), Hook_LoadGxt2);

	injector::MakeCALL(AddressSelectorVC::SelectAddress<0x552461, 0x0, 0x552351>(), CFont::LoadCHSTexture);
	injector::MakeCALL(AddressSelectorVC::SelectAddress<0x552300, 0x0, 0x5521F0>(), CFont::UnloadCHSTexture);

	injector::MakeJMP(AddressSelectorVC::SelectAddress<0x550650, 0x0, 0x550540>(), CFont::GetStringWidth);
	injector::MakeJMP(AddressSelectorVC::SelectAddress<0x550720, 0x0, 0x550610>(), CFont::GetTextRect);
	injector::MakeJMP(AddressSelectorVC::SelectAddress<0x550C70, 0x0, 0x550B60>(), CFont::GetNumberLines);

	injector::MakeJMP(AddressSelectorVC::SelectAddress<0x551040, 0x0, 0x550F30>(), CFont::PrintString);
	injector::MakeJMP(AddressSelectorVC::SelectAddress<0x551A30, 0x0, 0x551920>(), CFont::RenderFontBuffer);

	injector::WriteMemory(AddressSelectorVC::SelectAddress<0x68FD58, 0x0, 0x68ED60, float>(), 999.0f);

	unsigned __int8 *SpaceAddInstr = AddressSelectorVC::SelectAddress<0x6161BB, 0x0, 0x615D50, unsigned __int8>();
	injector::MakeNOP(SpaceAddInstr, 6);
	injector::MakeNOP(SpaceAddInstr + 8);
	injector::MakeNOP(SpaceAddInstr + 0x22);
	injector::MakeNOP(SpaceAddInstr + 0x3D);
	injector::MakeNOP(SpaceAddInstr + 0x42, 6);
	injector::WriteMemory<unsigned __int8>(SpaceAddInstr + 0x65, 1, true);
	injector::MakeNOP(SpaceAddInstr + 0x72, 6);
}
 