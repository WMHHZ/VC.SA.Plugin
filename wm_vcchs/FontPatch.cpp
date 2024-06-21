#include "FontPatch.h"
#include <array>
#include <game_vc/CTimer.h>
#include <game_vc/CTxdStore.h>
#include <game_vc/RenderWare.h>

namespace FontPatch
{
typedef wchar_t CharType;

struct FontBufferEntry
{
    CFontRenderState data;
    CharType         text[1];
};

union FontBufferPointer {
    CFontRenderState *pdata;
    CharType         *ptext;
    std::uintptr_t    addr;
};
static_assert(sizeof(FontBufferPointer) == 4);

#if 0
class CFontDetails
{
public:
    CRGBA Color;
    CVector2D Scale;
    float Slant;
    CVector2D SlantRefPoint;
    bool Justify;
    bool Centre;
    bool RightJustify;
    bool Background;
    bool BackGroundOnlyText;
    bool Prop;
    bool KeepColor;
    bool UselessFlag1;
    bool UselessFlag2;
    char pad1[3];
    float AlphaFade;
    CRGBA BackgroundColor;
    float WrapX;
    float CentreSize;
    float RightJustifyWrap;
    short FontStyle;
    bool BaseCharset;
    char pad2[5];
    short DropShadowPos;
    CRGBA DropColor;
    bool IsBlip;
    char pad3;
    unsigned int BlipStartTime;
    bool UselessFlag3;
    char pad4[3];
    int TextCount;
};

class CFontRenderState
{
public:
    int Useless;
    CVector2D Pos;
    CVector2D Scale;
    CRGBA Color;
    float JustifyWrap;
    float Slant;
    CVector2D SlantRefPoint;
    bool KeepColor;
    bool BaseCharset;
    bool Prop;
    short FontStyle;
};
#endif

const short iMaxCharWidth = 28;
const float fMaxCharWidth = iMaxCharWidth;

struct CharPos
{
    unsigned char rowIndex;
    unsigned char columnIndex;
};

char                                fontPath[260];
char                                textPath[260];
char                                datPath[260];
static std::array<CharPos, 0x10000> sTable;

tFontTable        *Size;
FontBufferPointer  FontBuffer;
FontBufferPointer *FontBufferIter;

CSprite2d ChsSprite;
CSprite2d ChsSlantSprite;

const CharPos &GetCharPos(CharType chr)
{
    return sTable[chr];
}

void ReadTable()
{
    sTable.fill({63, 63});

    FILE *hfile = std::fopen(datPath, "rb");

    if (hfile != nullptr)
    {
        std::fseek(hfile, 0, SEEK_END);

        if (std::ftell(hfile) == 131072)
        {
            std::fseek(hfile, 0, SEEK_SET);
            std::fread(sTable.data(), 2, 0x10000, hfile);
        }

        std::fclose(hfile);
    }
}

void LoadCHSFont()
{
    char normal[]  = "normal";
    char normalm[] = "normalm";
    char slant[]   = "slant";
    char slantm[]  = "slantm";

    int slot = CTxdStore::AddTxdSlot("wm_vcchs");
    CTxdStore::LoadTxd(slot, fontPath);
    CTxdStore::AddRef(slot);
    CTxdStore::PushCurrentTxd();
    CTxdStore::SetCurrentTxd(slot);
    ChsSprite.SetTexture(normal, normalm);
    ChsSlantSprite.SetTexture(slant, slantm);
    CTxdStore::PopCurrentTxd();
}

void UnloadCHSFont(int dummy)
{
    CTxdStore::RemoveTxdSlot(dummy);
    ChsSprite.Delete();
    ChsSlantSprite.Delete();
    CTxdStore::RemoveTxdSlot(CTxdStore::FindTxdSlot("wm_vcchs"));
}

float GetCharacterSize(CharType arg_char, short nFontStyle, bool bFontHalfTexture, bool bProp, float fScaleX)
{
    short charWidth;

    if (arg_char >= 0x80)
    {
        charWidth = iMaxCharWidth + 1;
    }
    else
    {
        arg_char -= 0x20;

        if (bFontHalfTexture)
        {
            arg_char = CFont::FindNewCharacter(arg_char);
        }

        if (bProp)
        {
            charWidth = Size[nFontStyle].prop[arg_char];
        }
        else
        {
            charWidth = Size[nFontStyle].unprop;
        }
    }

    return (charWidth * fScaleX);
}

float GetCharacterSizeNormal(CharType arg_char)
{
    return GetCharacterSize(arg_char, CFont::Details.m_FontStyle, CFont::Details.m_bFontHalfTexture,
                            CFont::Details.m_bPropOn, CFont::Details.m_vecScale.x);
}

float GetCharacterSizeDrawing(CharType arg_char)
{
    return GetCharacterSize(arg_char, CFont::RenderState.FontStyle, CFont::RenderState.bFontHalfTexture,
                            CFont::RenderState.bProp, CFont::RenderState.fTextSizeX);
}

float GetStringWidth(CharType *arg_text, bool bGetAll)
{
    float result = 0.0f;

    while (*arg_text != '\0')
    {
        if (*arg_text == ' ')
        {
            if (bGetAll)
            {
                result += GetCharacterSizeNormal(' ');
            }
            else
            {
                break;
            }
        }
        else if (*arg_text == '~')
        {
            if (result == 0.0f || bGetAll)
            {
                do
                {
                    ++arg_text;
                } while (*arg_text != '~');
            }
            else
            {
                break;
            }
        }
        else if (*arg_text < 0x80)
        {
            result += GetCharacterSizeNormal(*arg_text);
        }
        else
        {
            if (result == 0.0f || bGetAll)
            {
                result += GetCharacterSizeNormal(*arg_text);
            }

            if (!bGetAll)
            {
                break;
            }
        }

        ++arg_text;
    }

    return result;
}

CharType *GetNextSpace(CharType *arg_text)
{
    CharType *temp = arg_text;

    while (*temp != ' ' && *temp != '\0')
    {
        if (*temp == '~')
        {
            if (temp == arg_text)
            {
                do
                {
                    ++temp;
                } while (*temp != '~');

                ++temp;
                arg_text = temp;
                continue;
            }
            else
            {
                break;
            }
        }
        else if (*temp >= 0x80)
        {
            if (temp == arg_text)
            {
                ++temp;
            }

            break;
        }

        ++temp;
    }

    return temp;
}

short GetNumberLines(float arg_x, float arg_y, CharType *arg_text)
{
    short result = 0;
    float xBound;
    float yBound = arg_y;
    float strWidth, widthLimit;

    if (CFont::Details.m_bCentre || CFont::Details.m_bRightJustify)
    {
        xBound = 0.0f;
    }
    else
    {
        xBound = arg_x;
    }

    while (*arg_text != 0)
    {
        strWidth = GetStringWidth(arg_text, false);

        if (CFont::Details.m_bCentre)
        {
            widthLimit = CFont::Details.m_fCentreSize;
        }
        else
        {
            widthLimit = CFont::Details.m_fWrapX;
        }

        if ((xBound + strWidth) <= widthLimit)
        {
            xBound += strWidth;
            arg_text = GetNextSpace(arg_text);

            if (*arg_text == ' ')
            {
                xBound += GetCharacterSizeNormal(' ');
                ++arg_text;
            }
            else if (*arg_text == 0)
            {
                ++result;
            }
        }
        else
        {
            if (CFont::Details.m_bCentre || CFont::Details.m_bRightJustify)
            {
                xBound = 0.0f;
            }
            else
            {
                xBound = arg_x;
            }

            ++result;
            yBound += CFont::Details.m_vecScale.y * 18.0f;
        }
    }

    return result;
}

void GetTextRect(CRect *result, float arg_x, float arg_y, CharType *arg_text)
{
    short numLines = GetNumberLines(arg_x, arg_y, arg_text);

    if (CFont::Details.m_bCentre)
    {
        if (CFont::Details.m_bBackGroundOnlyText)
        {
            result->left   = arg_x - 4.0f;
            result->right  = arg_x + 4.0f;
            result->bottom = (18.0f * CFont::Details.m_vecScale.y) * numLines + arg_y + 2.0f;
            result->top    = arg_y - 2.0f;
        }
        else
        {
            result->left   = arg_x - (CFont::Details.m_fCentreSize * 0.5f) - 4.0f;
            result->right  = arg_x + (CFont::Details.m_fCentreSize * 0.5f) + 4.0f;
            result->bottom = arg_y + (18.0f * CFont::Details.m_vecScale.y * numLines) + 2.0f;
            result->top    = arg_y - 2.0f;
        }
    }
    else
    {
        result->left   = arg_x - 4.0f;
        result->right  = CFont::Details.m_fWrapX;
        result->bottom = arg_y;
        result->top    = (18.0f * CFont::Details.m_vecScale.y) * numLines + arg_y + 4.0f;
    }
}

void PrintString(float arg_x, float arg_y, CharType *arg_text)
{
    CRect textBoxRect;

    float xBound;
    float yBound = arg_y;
    float strWidth, widthLimit;
    float var_38 = 0.0f;
    float print_x;
    float justifyWrap;

    CharType *ptext   = arg_text;
    CharType *strHead = arg_text;

    bool emptyLine = true;

    short numSpaces = 0;

    CFont::Details.field_1F = 0;

    if (*arg_text == '*')
    {
        return;
    }

    ++CFont::Details.field_50;

    if (CFont::Details.m_bBackground)
    {
        GetTextRect(&textBoxRect, arg_x, arg_y, arg_text);
        CSprite2d::DrawRect(textBoxRect, CFont::Details.m_BackgroundColor);
    }

    if (CFont::Details.m_bCentre || CFont::Details.m_bRightJustify)
    {
        xBound = 0.0f;
    }
    else
    {
        xBound = arg_x;
    }

    while (*ptext != 0)
    {
        strWidth = GetStringWidth(ptext, false);

        if (CFont::Details.m_bCentre)
        {
            widthLimit = CFont::Details.m_fCentreSize;
        }
        else if (CFont::Details.m_bRightJustify)
        {
            widthLimit = arg_x - CFont::Details.m_fRightJustifyWrap;
        }
        else
        {
            widthLimit = CFont::Details.m_fWrapX;
        }

        if (((xBound + strWidth) <= widthLimit) || emptyLine)
        {
            ptext = GetNextSpace(ptext);
            xBound += strWidth;

            if (*ptext != 0)
            {
                if (*ptext == ' ')
                {
                    if (*(ptext + 1) == 0)
                    {
                        *ptext = 0;
                    }
                    else
                    {
                        if (!emptyLine)
                        {
                            ++numSpaces;
                        }

                        xBound += GetCharacterSizeNormal(' ');
                        ++ptext;
                    }
                }

                emptyLine = false;

                var_38 = xBound;
            }
            else
            {
                if (CFont::Details.m_bCentre)
                {
                    print_x = arg_x - xBound * 0.5f;
                }
                else if (CFont::Details.m_bRightJustify)
                {
                    print_x = arg_x - xBound;
                }
                else
                {
                    print_x = arg_x;
                }

                CFont::PrintString(print_x, yBound, 0, strHead, ptext, 0.0f);
            }
        }
        else
        {
            if (CFont::Details.m_bJustify && !(CFont::Details.m_bCentre))
            {
                justifyWrap = (CFont::Details.m_fWrapX - var_38) / numSpaces;
            }
            else
            {
                justifyWrap = 0.0f;
            }

            if (CFont::Details.m_bCentre)
            {
                print_x = arg_x - xBound * 0.5f;
            }
            else if (CFont::Details.m_bRightJustify)
            {
                print_x = arg_x - xBound;
            }
            else
            {
                print_x = arg_x;
            }

            CFont::PrintString(print_x, yBound, 0, strHead, ptext, justifyWrap);

            strHead = ptext;

            if (CFont::Details.m_bCentre || CFont::Details.m_bRightJustify)
            {
                xBound = 0.0f;
            }
            else
            {
                xBound = arg_x;
            }

            yBound += CFont::Details.m_vecScale.y * 18.0f;
            var_38    = 0.0f;
            numSpaces = 0;
            emptyLine = true;
        }
    }
}

void PrintCHSChar(float arg_x, float arg_y, CharType arg_char)
{
    static const float rRowsCount    = 1.0f / 64.0f;
    static const float rColumnsCount = 1.0f / 64.0f;
    static const float ufix          = 0.001f / 4.0f;
    // static const float vfix = 0.0021f / 4.0f;
    static const float vfix          = 0.001f / 4.0f;
    static const float vfix1_slant   = 0.00055f / 4.0f;
    // static const float vfix2_slant = 0.01f / 4.0f;
    static const float vfix2_slant   = 0.007f / 4.0f;
    static const float vfix3_slant   = 0.009f / 4.0f;

    CRect rect;

    float yOffset;

    float u1, v1, u2, v2, u3, v3, u4, v4;

    CharPos pos;

    if (arg_x >= RsGlobal.maximumWidth || arg_x <= 0.0f || arg_y <= 0.0f || arg_y >= RsGlobal.maximumHeight)
    {
        return;
    }

    pos = GetCharPos(arg_char);

    yOffset = CFont::RenderState.fTextSizeY * 2.0f;

    if (CFont::RenderState.fSlant == 0.0f)
    {
        rect.left   = arg_x;
        rect.top    = arg_y + yOffset;
        rect.right  = CFont::RenderState.fTextSizeX * 32.0f + arg_x;
        rect.bottom = CFont::RenderState.fTextSizeY * 16.0f + arg_y + yOffset;

        u1 = pos.columnIndex * rColumnsCount;
        v1 = pos.rowIndex * rRowsCount;
        u2 = (pos.columnIndex + 1) * rColumnsCount - ufix;
        v2 = v1;
        u3 = u1;
        v3 = (pos.rowIndex + 1) * rRowsCount - vfix;
        u4 = u2;
        v4 = v3;
    }
    else
    {
        rect.left   = arg_x;
        rect.top    = arg_y + 0.015f + yOffset;
        rect.right  = CFont::RenderState.fTextSizeX * 32.0f + arg_x;
        rect.bottom = CFont::RenderState.fTextSizeY * 16.0f + arg_y + yOffset;

        u1 = pos.columnIndex * rColumnsCount;
        v1 = pos.rowIndex * rRowsCount + vfix1_slant;
        u2 = (pos.columnIndex + 1) * rColumnsCount - ufix;
        v2 = pos.rowIndex * rRowsCount + vfix + vfix2_slant;
        u3 = pos.columnIndex * rColumnsCount;
        v3 = (pos.rowIndex + 1) * rRowsCount - vfix3_slant;
        u4 = (pos.columnIndex + 1) * rColumnsCount - ufix;
        v4 = (pos.rowIndex + 1) * rRowsCount + vfix2_slant - vfix;
    }

    CSprite2d::AddToBuffer(rect, CFont::RenderState.color, u1, v1, u2, v2, u3, v3, u4, v4);
}

void PrintCharDispatcher(float arg_x, float arg_y, CharType arg_char)
{
    if (arg_char < 0x80)
    {
        arg_char -= 0x20;

        if (CFont::RenderState.bFontHalfTexture)
        {
            arg_char = CFont::FindNewCharacter(arg_char);
        }

        CFont::PrintChar(arg_x, arg_y, arg_char);
    }
    else
    {
        PrintCHSChar(arg_x, arg_y, arg_char);
    }
}

void RenderFontBuffer()
{
    bool var_D = false;
    bool var_E = false;

    CRGBA var_14;

    CVector2D pos;

    CharType var_char;

    FontBufferPointer pbuffer;

    if (FontBufferIter->addr == FontBuffer.addr)
    {
        return;
    }

    CFont::RenderState = *(FontBuffer.pdata);
    var_14             = FontBuffer.pdata->color;

    pos.x = CFont::RenderState.fTextPosX;
    pos.y = CFont::RenderState.fTextPosY;

    pbuffer.addr = FontBuffer.addr + 0x30;

    while (pbuffer.addr < FontBufferIter->addr)
    {
        if (*(pbuffer.ptext) == 0)
        {
            ++pbuffer.ptext;

            if ((pbuffer.addr & 3) != 0)
            {
                ++pbuffer.ptext;
            }

            if (pbuffer.addr >= FontBufferIter->addr)
            {
                break;
            }

            CFont::RenderState = *pbuffer.pdata;

            var_14 = CFont::RenderState.color;

            pos.x = CFont::RenderState.fTextPosX;
            pos.y = CFont::RenderState.fTextPosY;

            pbuffer.addr += 0x30;
        }

        if (*pbuffer.ptext == '~')
        {
            pbuffer.ptext = CFont::ParseToken(pbuffer.ptext, var_14, var_E, var_D);

            if (var_E)
            {
                if ((CTimer::m_snTimeInMilliseconds - CFont::Details.field_48) > 300)
                {
                    CFont::Details.field_46 = true;
                    CFont::Details.field_48 = CTimer::m_snTimeInMilliseconds;
                }

                if (CFont::Details.field_46)
                {
                    CFont::Details.m_Color.a = 0;
                }
                else
                {
                    CFont::Details.m_Color.a = 255;
                }
            }

            if (!CFont::RenderState.bIsShadow)
            {
                CFont::RenderState.color = var_14;
            }
        }

        if (CFont::RenderState.fSlant != 0.0f)
        {
            pos.y = (CFont::RenderState.fSlantRefPointX - pos.x) * CFont::RenderState.fSlant +
                    CFont::RenderState.fSlantRefPointY;
        }

        var_char = *pbuffer.ptext;

        if (var_char < 0x80)
        {
            CFont::Sprite[CFont::RenderState.FontStyle].SetRenderState();
        }
        else
        {
            if (CFont::RenderState.fSlant == 0.0f)
            {
                ChsSprite.SetRenderState();
            }
            else
            {
                ChsSlantSprite.SetRenderState();
            }
        }

        RwRenderStateSet(RwRenderState::rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void *>(1));

        PrintCharDispatcher(pos.x, pos.y, var_char);

        if (var_D)
        {
            PrintCharDispatcher(pos.x + 1.0f, pos.y, var_char);
            PrintCharDispatcher(pos.x + 2.0f, pos.y, var_char);
            pos.x += 2.0f;
        }

        CSprite2d::RenderVertexBuffer();

        pos.x += GetCharacterSizeDrawing(var_char);

        if (var_char == ' ')
        {
            pos.x += CFont::RenderState.fExtraSpace;
        }

        ++pbuffer.ptext;
    }

    FontBufferIter->addr = FontBuffer.addr;
}

void DisableSlant(float slant)
{
    CFont::Details.m_fSlant = 0.0f;
}

__declspec(naked) void hook_load_gxt_mission()
{
    __asm
    {
        pop eax;
        add eax, 3;
        push offset textPath;
        jmp eax;
    }
}

char                   aRb[] = "rb";
__declspec(naked) void hook_load_gxt()
{
    __asm
    {
        pop eax;
        add eax, 5;
        push offset aRb;
        push offset textPath;
        jmp eax;
    }
}

bool Init()
{
    auto GInputHandle = GetModuleHandleW(L"GInputVC.asi");

    if (GInputHandle == NULL)
    {
        return false;
    }

    std::intptr_t base = reinterpret_cast<std::intptr_t>(GInputHandle);

    // 计算资源路径
    char pluginPath[260];
    GetModuleFileNameA(GetModuleHandleA("wm_vcchs.asi"), pluginPath, 260);
    std::strcpy(fontPath, pluginPath);
    std::strcpy(textPath, pluginPath);
    std::strcpy(datPath, pluginPath);

    std::strcpy(std::strrchr(fontPath, '.'), "\\wm_vcchs.txd");
    std::strcpy(std::strrchr(textPath, '.'), "\\wm_vcchs.gxt");
    std::strcpy(std::strrchr(datPath, '.'), "\\wm_vcchs.dat");

    // 读取资源文件
    ReadTable();

    // 加载贴图
    LoadCHSFont();

    // 打补丁
    Size             = (tFontTable *)GLOBAL_ADDRESS_BY_VERSION(0x696BD8, 0x0, 0x695BE0);
    FontBufferIter   = (FontBufferPointer *)GLOBAL_ADDRESS_BY_VERSION(0x70975C, 0x0, 0x70875C);
    FontBuffer.pdata = (CFontRenderState *)GLOBAL_ADDRESS_BY_VERSION(0x70935C, 0x0, 0x70835C);

    // Patch
    unsigned char *FEO_LAN_Entry = (unsigned char *)GLOBAL_ADDRESS_BY_VERSION(0x6DA386, 0x0, 0x6D9356);
    memcpy(FEO_LAN_Entry, FEO_LAN_Entry + 0x12, 0x12);
    memcpy(FEO_LAN_Entry + 0x12, FEO_LAN_Entry + 0x24, 0x12);
    memset(FEO_LAN_Entry + 0x24, 0, 0x12);

    injector::MakeCALL(GLOBAL_ADDRESS_BY_VERSION(0x5852A0, 0x0, 0x5850D0), hook_load_gxt_mission);
    injector::MakeCALL(GLOBAL_ADDRESS_BY_VERSION(0x5855EE, 0x0, 0x58541E), hook_load_gxt);

    injector::MakeJMP(GLOBAL_ADDRESS_BY_VERSION(0x550650, 0x0, 0x550540), GetStringWidth);
    injector::MakeJMP(GLOBAL_ADDRESS_BY_VERSION(0x550720, 0x0, 0x550610), GetTextRect);
    injector::MakeJMP(GLOBAL_ADDRESS_BY_VERSION(0x550C70, 0x0, 0x550B60), GetNumberLines);

    injector::MakeJMP(GLOBAL_ADDRESS_BY_VERSION(0x551040, 0x0, 0x550F30), PrintString);
    injector::MakeJMP(GLOBAL_ADDRESS_BY_VERSION(0x551A30, 0x0, 0x551920), RenderFontBuffer);

    injector::WriteMemory(GLOBAL_ADDRESS_BY_VERSION(0x68FD58, 0x0, 0x68ED60), 999.0f);

    unsigned char *SpaceAddInstr = (unsigned char *)GLOBAL_ADDRESS_BY_VERSION(0x6161BB, 0x0, 0x615DDB);
    injector::MakeNOP(SpaceAddInstr, 6);
    injector::MakeNOP(SpaceAddInstr + 8);
    injector::MakeNOP(SpaceAddInstr + 0x22);
    injector::MakeNOP(SpaceAddInstr + 0x3D);
    injector::MakeNOP(SpaceAddInstr + 0x42, 6);
    injector::WriteMemory<unsigned char>(SpaceAddInstr + 0x65, 1, true);
    injector::MakeNOP(SpaceAddInstr + 0x72, 6);

    return true;
}
} // namespace FontPatch
