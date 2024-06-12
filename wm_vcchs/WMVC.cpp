#include "WMVC.h"
#include "CFontPatch.h"
#include "CCharTable.h"

void WMVC::MakeResourcePath(HMODULE hPlugin)
{
    char pluginPath[260];

    GetModuleFileNameA(hPlugin, pluginPath, 260);
    std::strcpy(CFontPatch::fontPath, pluginPath);
    std::strcpy(CFontPatch::textPath, pluginPath);
    std::strcpy(CCharTable::datPath, pluginPath);

    std::strcpy(std::strrchr(CFontPatch::fontPath, '.'), "\\wm_vcchs.txd");
    std::strcpy(std::strrchr(CFontPatch::textPath, '.'), "\\wm_vcchs.gxt");
    std::strcpy(std::strrchr(CCharTable::datPath, '.'), "\\wm_vcchs.dat");
}

__declspec(naked) void hook_load_gxt_mission()
{
    __asm
    {
        pop eax;
        add eax, 3;
        push offset CFontPatch::textPath;
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
        push offset CFontPatch::textPath;
        jmp eax;
    }
}

void WMVC::PatchGame()
{
    //初始化内存地址
    CFontPatch::Size = (tFontTable*)GLOBAL_ADDRESS_BY_VERSION(0x696BD8, 0x0, 0x695BE0);
    CFontPatch::FontBufferIter = (FontBufferPointer*)GLOBAL_ADDRESS_BY_VERSION(0x70975C, 0x0, 0x70875C);
    CFontPatch::FontBuffer.pdata = (CFontRenderState*)GLOBAL_ADDRESS_BY_VERSION(0x70935C, 0x0, 0x70835C);

    //Patch
    unsigned char* FEO_LAN_Entry = (unsigned char *)GLOBAL_ADDRESS_BY_VERSION(0x6DA386, 0x0, 0x6D9356);
    memcpy(FEO_LAN_Entry, FEO_LAN_Entry + 0x12, 0x12);
    memcpy(FEO_LAN_Entry + 0x12, FEO_LAN_Entry + 0x24, 0x12);
    memset(FEO_LAN_Entry + 0x24, 0, 0x12);

    injector::MakeCALL(GLOBAL_ADDRESS_BY_VERSION(0x5852A0, 0x0, 0x5850D0), hook_load_gxt_mission);
    injector::MakeCALL(GLOBAL_ADDRESS_BY_VERSION(0x5855EE, 0x0, 0x58541E), hook_load_gxt);

    injector::MakeCALL(GLOBAL_ADDRESS_BY_VERSION(0x552461, 0x0, 0x552351), CFontPatch::LoadCHSFont);
    injector::MakeCALL(GLOBAL_ADDRESS_BY_VERSION(0x552300, 0x0, 0x5521F0), CFontPatch::UnloadCHSFont);

    injector::MakeJMP(GLOBAL_ADDRESS_BY_VERSION(0x550650, 0x0, 0x550540), CFontPatch::GetStringWidth);
    injector::MakeJMP(GLOBAL_ADDRESS_BY_VERSION(0x550720, 0x0, 0x550610), CFontPatch::GetTextRect);
    injector::MakeJMP(GLOBAL_ADDRESS_BY_VERSION(0x550C70, 0x0, 0x550B60), CFontPatch::GetNumberLines);

    injector::MakeJMP(GLOBAL_ADDRESS_BY_VERSION(0x551040, 0x0, 0x550F30), CFontPatch::PrintString);
    injector::MakeJMP(GLOBAL_ADDRESS_BY_VERSION(0x551A30, 0x0, 0x551920), CFontPatch::RenderFontBuffer);

    injector::WriteMemory(GLOBAL_ADDRESS_BY_VERSION(0x68FD58, 0x0, 0x68ED60), 999.0f);

    unsigned char* SpaceAddInstr = (unsigned char*)GLOBAL_ADDRESS_BY_VERSION(0x6161BB, 0x0, 0x615DDB);
    injector::MakeNOP(SpaceAddInstr, 6);
    injector::MakeNOP(SpaceAddInstr + 8);
    injector::MakeNOP(SpaceAddInstr + 0x22);
    injector::MakeNOP(SpaceAddInstr + 0x3D);
    injector::MakeNOP(SpaceAddInstr + 0x42, 6);
    injector::WriteMemory<unsigned char>(SpaceAddInstr + 0x65, 1, true);
    injector::MakeNOP(SpaceAddInstr + 0x72, 6);
}
