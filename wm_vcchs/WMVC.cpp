#include "WMVC.h"
#include "CFont.h"
#include "CCharTable.h"
#include "CTimer.h"
#include "CTxdStore.h"
#include "rwFunc.h"
#include "CSprite2d.h"

#define GAME_ID_GTAVC_1_0 0x74FF5064
#define GAME_ID_GTAVC_1_1 0x00408DC0
#define GAME_ID_GTAVC_STEAM 0x00004824
#define GAME_ID_GTAVC_STEAMENC 0x24E58287

bool WMVC::WaitForDecrypt()
{
    while (true)
    {
        Sleep(0);

        switch (injector::ReadMemory<unsigned int>(0x61C11C))
        {
        case GAME_ID_GTAVC_1_0:
        case GAME_ID_GTAVC_1_1:
        case GAME_ID_GTAVC_STEAM:
            return true;

        case GAME_ID_GTAVC_STEAMENC:
            continue;

        default:
            return false;
        }
    }
}

injector::auto_pointer WMVC::SelectAddress(unsigned int addr10, unsigned int addr11, unsigned int addrsteam)
{
    switch (injector::ReadMemory<unsigned int>(0x61C11C))
    {
    case GAME_ID_GTAVC_1_0:
        return injector::raw_ptr(addr10).get();

    case GAME_ID_GTAVC_1_1:
        return injector::raw_ptr(addr11).get();

    case GAME_ID_GTAVC_STEAM:
        return injector::raw_ptr(addrsteam).get();

    default:
        return injector::raw_ptr(0).get();
    }
}

void WMVC::MakeResourcePath(HMODULE hPlugin)
{
    char pluginPath[260];

    GetModuleFileNameA(hPlugin, pluginPath, 260);
    std::strcpy(CFont::fontPath, pluginPath);
    std::strcpy(CFont::textPath, pluginPath);
    std::strcpy(CCharTable::datPath, pluginPath);

    std::strcpy(std::strrchr(CFont::fontPath, '.'), "\\wm_vcchs.txd");
    std::strcpy(std::strrchr(CFont::textPath, '.'), "\\wm_vcchs.gxt");
    std::strcpy(std::strrchr(CCharTable::datPath, '.'), "\\wm_vcchs.dat");
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
    //初始化内存地址
    CFont::Size = SelectAddress(0x696BD8, 0x0, 0x695BE0);
    CFont::FontBufferIter = SelectAddress(0x70975C, 0x0, 0x70875C);
    CFont::FontBuffer.pdata = SelectAddress(0x70935C, 0x0, 0x70835C);
    CFont::RenderState = SelectAddress(0x94B8F8, 0x0, 0x94A900);
    CFont::Sprite = SelectAddress(0xA108B4, 0x0, 0xA0F8BC);
    CFont::Details = SelectAddress(0x97F820, 0x0, 0x97E828);
    CFont::fpFindNewCharacter.fun = SelectAddress(0x54FE70, 0x0, 0x54FD60);
    CFont::fpParseTokenEPt.fun = SelectAddress(0x5502D0, 0x0, 0x5501C0);
    CFont::fpParseTokenEPtR5CRGBARbRb.fun = SelectAddress(0x550510, 0x0, 0x550400);
    CFont::fpPrintStringPart.fun = SelectAddress(0x5516C0, 0x0, 0x5515B0);
    CFont::fpPrintChar.fun = SelectAddress(0x551E70, 0x0, 0x551D60);

    CSprite2d::fpAddToBuffer.fun = SelectAddress(0x578830, 0x0, 0x578720);
    CSprite2d::fpDelete.fun = SelectAddress(0x578A20, 0x0, 0x578910);
    CSprite2d::fpDraw.fun = SelectAddress(0x578710, 0x0, 0x0);
    CSprite2d::fpDrawRect.fun = SelectAddress(0x577B00, 0x0, 0x5779F0);
    CSprite2d::fpRenderVertexBuffer.fun = SelectAddress(0x5787E0, 0x0, 0x5786D0);
    CSprite2d::fpSetRenderState.fun = SelectAddress(0x577B90, 0x0, 0x577A80);
    CSprite2d::fpSetTexture.fun = SelectAddress(0x5789B0, 0x0, 0x5788A0);

    CTimer::m_nTimeInMilliseconds = SelectAddress(0x974B2C, 0x0, 0x973B34);

    CTxdStore::fpAddTxdSlot.fun = SelectAddress(0x580F00, 0x0, 0x580D30);
    CTxdStore::fpLoadTxd.fun = SelectAddress(0x580CD0, 0x0, 0x580B00);
    CTxdStore::fpAddRef.fun = SelectAddress(0x580A60, 0x0, 0x580890);
    CTxdStore::fpPushCurrentTxd.fun = SelectAddress(0x580AC0, 0x0, 0x5808F0);
    CTxdStore::fpPopCurrentTxd.fun = SelectAddress(0x580AA0, 0x0, 0x5808D0);
    CTxdStore::fpSetCurrentTxd.fun = SelectAddress(0x580AD0, 0x0, 0x580900);
    CTxdStore::fpFindTxdSlot.fun = SelectAddress(0x580D70, 0x0, 0x580BA0);
    CTxdStore::fpRemoveTxdSlot.fun = SelectAddress(0x580E90, 0x0, 0x580CC0);

    rwFunc::RsGlobalW = SelectAddress(0x9B48E4, 0x0, 0x9B38EC);
    rwFunc::RsGlobalH = rwFunc::RsGlobalW + 1;
    rwFunc::fpRwRenderStateSet.fun = SelectAddress(0x649BA0, 0x0, 0x648B50);

    //Patch
    unsigned char *FEO_LAN_Entry = SelectAddress(0x6DA386, 0x0, 0x6D9356);
    memcpy(FEO_LAN_Entry, FEO_LAN_Entry + 0x12, 0x12);
    memcpy(FEO_LAN_Entry + 0x12, FEO_LAN_Entry + 0x24, 0x12);
    memset(FEO_LAN_Entry + 0x24, 0, 0x12);

    injector::MakeCALL(SelectAddress(0x5852A0, 0x0, 0x5850D0), hook_load_gxt_mission);
    injector::MakeCALL(SelectAddress(0x5855EE, 0x0, 0x58541E), hook_load_gxt);

    injector::MakeCALL(SelectAddress(0x552461, 0x0, 0x552351), CFont::LoadCHSFont);
    injector::MakeCALL(SelectAddress(0x552300, 0x0, 0x5521F0), CFont::UnloadCHSFont);

    injector::MakeJMP(SelectAddress(0x550650, 0x0, 0x550540), CFont::GetStringWidth);
    injector::MakeJMP(SelectAddress(0x550720, 0x0, 0x550610), CFont::GetTextRect);
    injector::MakeJMP(SelectAddress(0x550C70, 0x0, 0x550B60), CFont::GetNumberLines);

    injector::MakeJMP(SelectAddress(0x551040, 0x0, 0x550F30), CFont::PrintString);
    injector::MakeJMP(SelectAddress(0x551A30, 0x0, 0x551920), CFont::RenderFontBuffer);

    injector::WriteMemory(SelectAddress(0x68FD58, 0x0, 0x68ED60), 999.0f);

    unsigned char *SpaceAddInstr = SelectAddress(0x6161BB, 0x0, 0x615DDB);
    injector::MakeNOP(SpaceAddInstr, 6);
    injector::MakeNOP(SpaceAddInstr + 8);
    injector::MakeNOP(SpaceAddInstr + 0x22);
    injector::MakeNOP(SpaceAddInstr + 0x3D);
    injector::MakeNOP(SpaceAddInstr + 0x42, 6);
    injector::WriteMemory<unsigned char>(SpaceAddInstr + 0x65, 1, true);
    injector::MakeNOP(SpaceAddInstr + 0x72, 6);
}
