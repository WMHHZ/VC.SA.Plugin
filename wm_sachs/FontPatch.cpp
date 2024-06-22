#include "FontPatch.h"
#include <game_sa/CMessages.h>
#include <game_sa/CSprite.h>
#include <game_sa/CTheScripts.h>
#include <game_sa/RenderWare.h>
#include <injector.hpp>
#include <utf8/unchecked.h>
#include <utility.hpp>

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

namespace FontPatch
{
const float         scale_x_rec     = 1.0f / 16.0f;
const float         scale_y_rec     = 1.0f / 12.8f;
const float         scale_x_rec_chs = 1.0f / 64.0f;
const float         scale_y_rec_chs = 1.0f / 51.2f;
const float         fix_value       = 0.0021f;
const float         fix_value_chs   = 0.0021f / 4.0f;
const unsigned char SBCLetterWidth  = 32;

struct CFontRenderState
{
  public:
    float     Useless;
    CVector2D Pos;
    CVector2D Scale;
    CRGBA     Color;
    float     JustifyWrap;
    float     Slant;
    CVector2D SlantRefPoint;
    bool      IsBlip;
    char      FontStyle;
    bool      Prop;
    char      _pad1;
    short     TextureID;
    char      OutlineSize;
    char      _pad2;
};
static_assert(sizeof(CFontRenderState) == 0x30);

union FontBufferPointer {
    CFontRenderState *pdata;
    char             *ptext;
    unsigned int      addr;
};
static_assert(sizeof(FontBufferPointer) == 0x4);

struct CharPos
{
    unsigned char rowIndex;
    unsigned char columnIndex;
};

CFontRenderState  *m_FontBuffer     = injector::auto_pointer(0xC716B0);
FontBufferPointer *m_FontBufferIter = injector::auto_pointer(0xC716A8);
CFontRenderState  *RenderState      = injector::auto_pointer(0xC71AA0);

CSprite2d m_ChsSprite;

unsigned char(__cdecl *FindSubFontCharacter)(unsigned char, unsigned char)   = injector::auto_pointer(0x7192C0);
void(__cdecl *RenderString)(float, float, const char *, const char *, float) = injector::auto_pointer(0x719B40);

float     *GInput_SymbolWidth;
CSprite2d *GInput_ButtonSprites;
char *(__cdecl *GInput_ParseTokenFunc)(char *, CRGBA &, bool, char *);
char *(__stdcall *GInput_SkipToken)(char *, float *);

CharPos TheTable[0x10000];

char texturePath[MAX_PATH];
char textPath[MAX_PATH];

char aRb[] = "rb";
__declspec(naked) void Hook_LoadGxt()
{
    __asm
    {
        pop eax;
        inc eax;
        push offset aRb;
        push offset textPath;
        jmp eax;
    }
}

CharPos GetCharPos(unsigned int chr)
{
    CharPos result;

    if (chr < 0x60)
    {
        if (FontPatch::RenderState->FontStyle != 0)
        {
            chr = FindSubFontCharacter(chr, RenderState->FontStyle);

            if (chr == 0xD0)
            {
                chr = 0;
            }
        }

        result.rowIndex    = (chr >> 4);
        result.columnIndex = (chr & 0xF);
    }
    else
    {
        result = TheTable[chr + 0x20];
    }

    return result;
}

float __cdecl GetScaledLetterWidthNormal(unsigned int arg_char)
{
    unsigned char charWidth;

    if (arg_char >= 0x60)
    {
        charWidth = SBCLetterWidth;
    }
    else
    {
        if (arg_char == '?')
        {
            arg_char = 0;
        }

        if (CFont::m_FontStyle != 0)
        {
            arg_char = FindSubFontCharacter(arg_char, CFont::m_FontStyle);
        }

        if (CFont::m_bFontPropOn)
        {
            charWidth = gFontData[CFont::m_FontTextureId].m_propValues[arg_char];
        }
        else
        {
            charWidth = gFontData[CFont::m_FontTextureId].m_unpropValue;
        }
    }

    return ((charWidth + CFont::m_nFontOutlineSize) * CFont::m_Scale->x);
}

float GetScaledLetterWidthScript(unsigned int arg_char)
{
    auto &drawer = CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame];

    unsigned char charWidth, style;

    if (arg_char >= 0x60)
    {
        charWidth = SBCLetterWidth;
    }
    else
    {
        if (arg_char == '?')
        {
            arg_char = 0;
        }

        switch (drawer.font)
        {
        case 2:
            style    = 0;
            arg_char = FindSubFontCharacter(arg_char, 2);
            break;

        case 3:
            style    = 1;
            arg_char = FindSubFontCharacter(arg_char, 1);
            break;

        default:
            style = drawer.font;
            break;
        }

        if (drawer.proportional)
        {
            charWidth = gFontData[style].m_propValues[arg_char];
        }
        else
        {
            charWidth = gFontData[style].m_unpropValue;
        }
    }

    return ((charWidth + drawer.outlineType) * drawer.letterWidth);
}

float __cdecl GetScaledLetterWidthDrawing(unsigned int arg_char)
{
    unsigned char charWidth;

    if (arg_char >= 0x60)
    {
        charWidth = SBCLetterWidth;
    }
    else
    {
        if (arg_char == '?')
        {
            arg_char = 0;
        }

        if (RenderState->FontStyle != 0)
        {
            arg_char = FindSubFontCharacter(arg_char, RenderState->FontStyle);
        }

        if (RenderState->Prop)
        {
            charWidth = gFontData[RenderState->TextureID].m_propValues[arg_char];
        }
        else
        {
            charWidth = gFontData[RenderState->TextureID].m_unpropValue;
        }
    }

    return (charWidth + RenderState->OutlineSize) * RenderState->Scale.x;
}

float GetStringWidth(const char *arg_text, bool bGetAll, bool bScript)
{
    char  strbuf[400];
    char *bufIter = strbuf;

    float result = 0.0f;

    bool succeeded   = false;
    bool StopAtDelim = false;

    strncpy(strbuf, arg_text, 400);
    strbuf[399] = 0;

    CMessages::InsertPlayerControlKeysInString(strbuf);

    while (true)
    {
        unsigned int code = utf8::unchecked::peek_next(bufIter);

        if (code == '\0')
        {
            break;
        }
        else if (code == ' ' && !bGetAll)
        {
            break;
        }
        else if (code == '~')
        {
            if (!bGetAll && (StopAtDelim || succeeded))
            {
                break;
            }

            bufIter = GInput_SkipToken(bufIter, &result);

            ++bufIter;

            if (succeeded || *bufIter == '~')
            {
                StopAtDelim = true;
            }
        }
        else if (code >= 0x80)
        {
            if (bGetAll || !succeeded)
            {
                if (bScript)
                {
                    result += GetScaledLetterWidthScript(code - 0x20);
                }
                else
                {
                    result += GetScaledLetterWidthNormal(code - 0x20);
                }

                succeeded = true;
            }

            if (!bGetAll)
            {
                break;
            }
        }
        else
        {
            if (!bGetAll && code == ' ' && StopAtDelim)
            {
                break;
            }

            if (bScript)
            {
                result += GetScaledLetterWidthScript(code - 0x20);
            }
            else
            {
                result += GetScaledLetterWidthNormal(code - 0x20);
            }

            succeeded = true;
        }

        utf8::unchecked::next(bufIter);
    }

    return result;
}

char *GetNextSpace(char *arg_pointer)
{
    char *var_pointer = arg_pointer;

    while (true)
    {
        unsigned int code = utf8::unchecked::peek_next(var_pointer);

        if (code == 0 || code == ' ' || code == '~')
        {
            break;
        }
        else if (code >= 0x80)
        {
            if (var_pointer == arg_pointer)
            {
                utf8::unchecked::next(var_pointer);
            }

            break;
        }

        utf8::unchecked::next(var_pointer);
    }

    return var_pointer;
}

void PrintCHSChar(float arg_x, float arg_y, unsigned int arg_char)
{
    CRect   rect;
    float   row, column;
    CharPos cpos;

    if (arg_y < 0.0f || RsGlobal.maximumHeight < arg_y || arg_x < 0.0f || RsGlobal.maximumWidth < arg_x)
    {
        return;
    }

    if (CFont::m_nExtraFontSymbolId != 0)
    {
        rect.top    = RenderState->Scale.y * 2.0f + arg_y;
        rect.right  = RenderState->Scale.y * *GInput_SymbolWidth + arg_x;
        rect.bottom = RenderState->Scale.y * 19.0f + arg_y;
        rect.left   = arg_x;
        GInput_ButtonSprites[CFont::m_nExtraFontSymbolId].Draw(rect, CRGBA(255, 255, 255, RenderState->Color.a));
        return;
    }

    if (arg_char == 0 || arg_char == '?')
    {
        return;
    }

    cpos = GetCharPos(arg_char);

    row    = cpos.rowIndex;
    column = cpos.columnIndex;

    rect.left   = arg_x;
    rect.top    = arg_y;
    rect.right  = RenderState->Scale.x * 32.0f + arg_x;
    rect.bottom = RenderState->Scale.y * 20.0f + arg_y;

    if (arg_char < 0x60)
    {
        row *= scale_y_rec;
        column *= scale_x_rec;

        CSprite2d::AddToBuffer(rect, RenderState->Color, column, row + fix_value, column + scale_x_rec - fix_value,
                               row + fix_value, column, row + scale_y_rec - fix_value, column + scale_x_rec - fix_value,
                               row + scale_y_rec - fix_value);
    }
    else
    {
        row *= scale_y_rec_chs;
        column *= scale_x_rec_chs;

        CSprite2d::AddToBuffer(rect, RenderState->Color, column, row + fix_value_chs,
                               column + scale_x_rec_chs - fix_value_chs, row + fix_value_chs, column,
                               row + scale_y_rec_chs - fix_value_chs, column + scale_x_rec_chs - fix_value_chs,
                               row + scale_y_rec_chs - fix_value_chs);
    }
}

short ProcessCurrentString(bool print, float arg_x, float arg_y, char *arg_text)
{
    char       *esi = arg_text;
    const char *ebp = esi;
    const char *edi;

    short result   = 0;
    short numWords = 0;

    bool emptyLine = true;
    char tag       = '\0';

    CRGBA fontColor = *CFont::m_Color;
    CRGBA tagColor;

    float xBound;
    float yBound = arg_y;
    float strWidth, widthLimit;
    float var_110 = 0.0f;
    float var_10C;
    float var_124;

    char var_100[256];

    if (CFont::m_bFontCentreAlign || CFont::m_bFontRightAlign)
    {
        xBound = 0.0f;
    }
    else
    {
        xBound = arg_x;
    }

    while (*esi != '\0')
    {
        CFont::m_nExtraFontSymbolId = 0;
        strWidth                    = GetStringWidth(esi, false, false);

        if (*esi == '~')
        {
            esi = GInput_ParseTokenFunc(esi, tagColor, true, &tag);
        }

        if (CFont::m_bFontCentreAlign)
        {
            widthLimit = CFont::m_fFontCentreSize;
        }
        else if (CFont::m_bFontRightAlign)
        {
            widthLimit = arg_x - CFont::m_fRightJustifyWrap;
        }
        else
        {
            widthLimit = CFont::m_fWrapx;
        }

        strWidth += xBound;

        if ((strWidth <= widthLimit || emptyLine) && !CFont::m_bNewLine)
        {
            xBound = strWidth;
            esi    = GetNextSpace(esi);

            if (*esi != '\0')
            {
                if (!emptyLine)
                {
                    ++numWords;
                }

                if (*esi == ' ')
                {
                    xBound += GetScaledLetterWidthNormal(0);
                    ++esi;
                }

                var_110   = xBound;
                emptyLine = false;
            }
            else
            {
                if (CFont::m_bFontCentreAlign)
                {
                    var_124 = arg_x - xBound * 0.5f;
                }
                else if (CFont::m_bFontRightAlign)
                {
                    var_124 = arg_x - xBound;
                }
                else
                {
                    var_124 = arg_x;
                }

                ++result;

                if (print)
                {
                    RenderString(var_124, yBound, ebp, esi, 0.0f);
                }
            }
        }
        else
        {
            var_10C = 0.0f;

            if (CFont::m_nExtraFontSymbolId != 0)
            {
                esi -= 3;
            }

            edi = esi - 3;

            if (!CFont::m_bNewLine)
            {
                edi = esi;
            }

            if (CFont::m_bFontCentreAlign)
            {
                var_124 = arg_x - xBound * 0.5f;
            }
            else
            {
                if (CFont::m_bFontJustify)
                {
                    var_10C = (CFont::m_fWrapx - var_110) / numWords;
                }

                if (CFont::m_bFontRightAlign)
                {
                    var_124 = arg_x - (xBound - GetScaledLetterWidthNormal(0));
                }
                else
                {
                    var_124 = arg_x;
                }
            }

            ++result;

            if (print)
            {
                RenderString(var_124, yBound, ebp, edi, var_10C);
            }

            if (tag != '\0')
            {
                var_100[0] = '~';
                var_100[1] = tag;
                var_100[2] = '~';

                if (CFont::m_bNewLine)
                {
                    edi += 3;
                }

                strcpy(&var_100[3], edi);

                esi = var_100;
                tag = '\0';
            }

            CFont::m_bNewLine = false;
            yBound += CFont::m_Scale->y * 18.0f;

            if (CFont::m_bFontCentreAlign || CFont::m_bFontRightAlign)
            {
                xBound = 0.0f;
            }
            else
            {
                xBound = arg_x;
            }

            ebp       = esi;
            emptyLine = true;
            var_110   = 0.0f;
            numWords  = 0;
        }

        CFont::m_nExtraFontSymbolId = 0;
    };

    if (print)
    {
        CFont::SetColor(fontColor);
    }

    return result;
}

void RenderFontBuffer()
{
    CRGBA        var_color;
    CVector2D    pos;
    unsigned int var_char;

    FontBufferPointer ebx;

    if (m_FontBufferIter->pdata == m_FontBuffer)
    {
        return;
    }

    RenderState = m_FontBuffer;

    var_color = RenderState->Color;

    pos = RenderState->Pos;

    ebx.pdata = m_FontBuffer + 1;

    while (ebx.addr < m_FontBufferIter->addr)
    {
        if (*ebx.ptext == '\0')
        {
            ++ebx.ptext;

            while ((ebx.addr & 3) != 0)
            {
                ++ebx.ptext;
            }

            if (ebx.addr >= m_FontBufferIter->addr)
            {
                break;
            }

            *RenderState = *ebx.pdata;

            var_color = RenderState->Color;

            pos = RenderState->Pos;

            ++ebx.pdata;
        }

        CFont::m_nExtraFontSymbolId = 0;

        while (*ebx.ptext == '~')
        {
            if (CFont::m_nExtraFontSymbolId != 0)
            {
                break;
            }

            ebx.ptext = GInput_ParseTokenFunc(ebx.ptext, var_color, RenderState->IsBlip, nullptr);

            if (!RenderState->IsBlip)
            {
                RenderState->Color = var_color;
            }
        }

        var_char = utf8::unchecked::peek_next(ebx.ptext) - 0x20;

        if (RenderState->Slant != 0.0f)
        {
            pos.y = (RenderState->SlantRefPoint.x - pos.x) * RenderState->Slant + RenderState->SlantRefPoint.y;
        }

        if (CFont::m_nExtraFontSymbolId == 0 || !RenderState->IsBlip)
        {
            if (var_char < 0x60)
            {
                CFont::Sprite[RenderState->TextureID].SetRenderState();
            }
            else
            {
                m_ChsSprite.SetRenderState();
            }

            RwRenderStateSet(RwRenderState::rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);

            PrintCHSChar(pos.x, pos.y, var_char);
            CSprite::FlushSpriteBuffer();
            CSprite2d::RenderVertexBuffer();
        }

        if (CFont::m_nExtraFontSymbolId == 0)
        {
            pos.x += GetScaledLetterWidthDrawing(var_char);
        }
        else
        {
            pos.x += (RenderState->Scale.y * *GInput_SymbolWidth + RenderState->OutlineSize);
        }

        if (var_char == 0)
        {
            pos.x += RenderState->JustifyWrap;
        }

        if (CFont::m_nExtraFontSymbolId != 0)
        {
            CFont::m_nExtraFontSymbolId = 0;
        }
        else if (*ebx.ptext != '\0')
        {
            utf8::unchecked::next(ebx.ptext);
        }
    }

    m_FontBufferIter->pdata = m_FontBuffer;
}

bool Init()
{
    auto GInputHandle = GetModuleHandleW(L"GInputSA.asi");

    if (GInputHandle == NULL)
    {
        return false;
    }

    std::intptr_t base = reinterpret_cast<std::intptr_t>(GInputHandle);

    GInput_SymbolWidth    = injector::auto_pointer(base + 0x3B084).get();
    GInput_ButtonSprites  = injector::auto_pointer(base + 0x3AD60).get();
    GInput_ParseTokenFunc = injector::auto_pointer(base + 0x9040).get();
    GInput_SkipToken      = injector::auto_pointer(base + 0x99C0).get();

    // 计算贴图路径
    char pluginPath[MAX_PATH];

    GetModuleFileNameA(GetModuleHandleA("wm_sachs.asi"), pluginPath, MAX_PATH);
    strcpy(texturePath, pluginPath);
    strcpy(textPath, pluginPath);
    strcpy(strrchr(texturePath, '.'), "\\wm_sachs.png");
    strcpy(strrchr(textPath, '.'), "\\wm_sachs.gxt");

    if (!PathFileExistsA(texturePath) || !PathFileExistsA(textPath))
    {
        return false;
    }

    //  加载贴图
    int width, height, depth, flags;

    RwImage *image = RtPNGImageRead(texturePath);
    RwImageFindRasterFormat(image, 4, &width, &height, &depth, &flags);
    RwRaster *raster = RwRasterCreate(width, height, depth, flags);
    RwRasterSetFromImage(raster, image);
    RwImageDestroy(image);
    m_ChsSprite.m_pTexture = RwTextureCreate(raster);

    // 打补丁
    injector::MakeCALL(0x69FD54, Hook_LoadGxt);
    injector::MakeCALL(0x6A0222, Hook_LoadGxt);
    injector::MemoryFill(0x8CFD6A, 0, 0x12, false);

    injector::MakeCALL(0x47B565, GetStringWidth);
    injector::MakeCALL(0x47B73A, GetStringWidth);
    injector::MakeCALL(0x57A49B, GetStringWidth);
    injector::MakeCALL(0x57FB52, GetStringWidth);
    injector::MakeCALL(0x57FE35, GetStringWidth);
    injector::MakeCALL(0x5814A7, GetStringWidth);
    injector::MakeCALL(0x581512, GetStringWidth);
    injector::MakeCALL(0x58BCCC, GetStringWidth);

    injector::MakeCALL(0x71A5F1, ProcessCurrentString);
    injector::MakeCALL(0x71A611, ProcessCurrentString);
    injector::MakeCALL(0x71A631, ProcessCurrentString);
    injector::MakeCALL(0x71A802, ProcessCurrentString);
    injector::MakeCALL(0x71A834, ProcessCurrentString);

    injector::MakeCALL(0x57BF70, RenderFontBuffer);
    injector::MakeCALL(0x719B5D, RenderFontBuffer);
    injector::MakeCALL(0x719F43, RenderFontBuffer);
    injector::MakeJMP(0x71A210, RenderFontBuffer);

    // 填充字符表
    TheTable[0xb0]   = {0, 0};
    TheTable[0xb7]   = {0, 1};
    TheTable[0x2018] = {0, 2};
    TheTable[0x2019] = {0, 3};
    TheTable[0x201c] = {0, 4};
    TheTable[0x201d] = {0, 5};
    TheTable[0x2026] = {0, 6};
    TheTable[0x2190] = {0, 7};
    TheTable[0x2191] = {0, 8};
    TheTable[0x2192] = {0, 9};
    TheTable[0x2193] = {0, 10};
    TheTable[0x3000] = {0, 11};
    TheTable[0x3001] = {0, 12};
    TheTable[0x3002] = {0, 13};
    TheTable[0x300a] = {0, 14};
    TheTable[0x300b] = {0, 15};
    TheTable[0x4e00] = {0, 16};
    TheTable[0x4e01] = {0, 17};
    TheTable[0x4e03] = {0, 18};
    TheTable[0x4e07] = {0, 19};
    TheTable[0x4e08] = {0, 20};
    TheTable[0x4e09] = {0, 21};
    TheTable[0x4e0a] = {0, 22};
    TheTable[0x4e0b] = {0, 23};
    TheTable[0x4e0d] = {0, 24};
    TheTable[0x4e0e] = {0, 25};
    TheTable[0x4e11] = {0, 26};
    TheTable[0x4e13] = {0, 27};
    TheTable[0x4e14] = {0, 28};
    TheTable[0x4e16] = {0, 29};
    TheTable[0x4e18] = {0, 30};
    TheTable[0x4e19] = {0, 31};
    TheTable[0x4e1a] = {0, 32};
    TheTable[0x4e1b] = {0, 33};
    TheTable[0x4e1c] = {0, 34};
    TheTable[0x4e1d] = {0, 35};
    TheTable[0x4e22] = {0, 36};
    TheTable[0x4e24] = {0, 37};
    TheTable[0x4e25] = {0, 38};
    TheTable[0x4e27] = {0, 39};
    TheTable[0x4e2a] = {0, 40};
    TheTable[0x4e2b] = {0, 41};
    TheTable[0x4e2d] = {0, 42};
    TheTable[0x4e30] = {0, 43};
    TheTable[0x4e32] = {0, 44};
    TheTable[0x4e34] = {0, 45};
    TheTable[0x4e38] = {0, 46};
    TheTable[0x4e39] = {0, 47};
    TheTable[0x4e3a] = {0, 48};
    TheTable[0x4e3b] = {0, 49};
    TheTable[0x4e3d] = {0, 50};
    TheTable[0x4e3e] = {0, 51};
    TheTable[0x4e43] = {0, 52};
    TheTable[0x4e45] = {0, 53};
    TheTable[0x4e48] = {0, 54};
    TheTable[0x4e49] = {0, 55};
    TheTable[0x4e4b] = {0, 56};
    TheTable[0x4e4c] = {0, 57};
    TheTable[0x4e4e] = {0, 58};
    TheTable[0x4e4f] = {0, 59};
    TheTable[0x4e50] = {0, 60};
    TheTable[0x4e52] = {0, 61};
    TheTable[0x4e53] = {0, 62};
    TheTable[0x4e54] = {0, 63};
    TheTable[0x4e56] = {1, 0};
    TheTable[0x4e58] = {1, 1};
    TheTable[0x4e59] = {1, 2};
    TheTable[0x4e5d] = {1, 3};
    TheTable[0x4e5e] = {1, 4};
    TheTable[0x4e5f] = {1, 5};
    TheTable[0x4e60] = {1, 6};
    TheTable[0x4e61] = {1, 7};
    TheTable[0x4e66] = {1, 8};
    TheTable[0x4e70] = {1, 9};
    TheTable[0x4e71] = {1, 10};
    TheTable[0x4e73] = {1, 11};
    TheTable[0x4e86] = {1, 12};
    TheTable[0x4e88] = {1, 13};
    TheTable[0x4e89] = {1, 14};
    TheTable[0x4e8b] = {1, 15};
    TheTable[0x4e8c] = {1, 16};
    TheTable[0x4e8e] = {1, 17};
    TheTable[0x4e8f] = {1, 18};
    TheTable[0x4e91] = {1, 19};
    TheTable[0x4e92] = {1, 20};
    TheTable[0x4e94] = {1, 21};
    TheTable[0x4e95] = {1, 22};
    TheTable[0x4e9a] = {1, 23};
    TheTable[0x4e9b] = {1, 24};
    TheTable[0x4ea1] = {1, 25};
    TheTable[0x4ea4] = {1, 26};
    TheTable[0x4ea5] = {1, 27};
    TheTable[0x4ea6] = {1, 28};
    TheTable[0x4ea7] = {1, 29};
    TheTable[0x4ea9] = {1, 30};
    TheTable[0x4eab] = {1, 31};
    TheTable[0x4eac] = {1, 32};
    TheTable[0x4ead] = {1, 33};
    TheTable[0x4eae] = {1, 34};
    TheTable[0x4eb2] = {1, 35};
    TheTable[0x4eba] = {1, 36};
    TheTable[0x4ebf] = {1, 37};
    TheTable[0x4ec0] = {1, 38};
    TheTable[0x4ec1] = {1, 39};
    TheTable[0x4ec5] = {1, 40};
    TheTable[0x4ec6] = {1, 41};
    TheTable[0x4ec7] = {1, 42};
    TheTable[0x4eca] = {1, 43};
    TheTable[0x4ecb] = {1, 44};
    TheTable[0x4ecd] = {1, 45};
    TheTable[0x4ece] = {1, 46};
    TheTable[0x4ed3] = {1, 47};
    TheTable[0x4ed4] = {1, 48};
    TheTable[0x4ed6] = {1, 49};
    TheTable[0x4ed7] = {1, 50};
    TheTable[0x4ed8] = {1, 51};
    TheTable[0x4ed9] = {1, 52};
    TheTable[0x4ee3] = {1, 53};
    TheTable[0x4ee4] = {1, 54};
    TheTable[0x4ee5] = {1, 55};
    TheTable[0x4ee8] = {1, 56};
    TheTable[0x4eea] = {1, 57};
    TheTable[0x4eec] = {1, 58};
    TheTable[0x4ef0] = {1, 59};
    TheTable[0x4ef6] = {1, 60};
    TheTable[0x4ef7] = {1, 61};
    TheTable[0x4efb] = {1, 62};
    TheTable[0x4efd] = {1, 63};
    TheTable[0x4eff] = {2, 0};
    TheTable[0x4f01] = {2, 1};
    TheTable[0x4f0a] = {2, 2};
    TheTable[0x4f0d] = {2, 3};
    TheTable[0x4f0f] = {2, 4};
    TheTable[0x4f10] = {2, 5};
    TheTable[0x4f11] = {2, 6};
    TheTable[0x4f17] = {2, 7};
    TheTable[0x4f18] = {2, 8};
    TheTable[0x4f19] = {2, 9};
    TheTable[0x4f1a] = {2, 10};
    TheTable[0x4f1e] = {2, 11};
    TheTable[0x4f1f] = {2, 12};
    TheTable[0x4f20] = {2, 13};
    TheTable[0x4f24] = {2, 14};
    TheTable[0x4f26] = {2, 15};
    TheTable[0x4f2a] = {2, 16};
    TheTable[0x4f2f] = {2, 17};
    TheTable[0x4f30] = {2, 18};
    TheTable[0x4f34] = {2, 19};
    TheTable[0x4f36] = {2, 20};
    TheTable[0x4f38] = {2, 21};
    TheTable[0x4f3c] = {2, 22};
    TheTable[0x4f3d] = {2, 23};
    TheTable[0x4f46] = {2, 24};
    TheTable[0x4f4d] = {2, 25};
    TheTable[0x4f4e] = {2, 26};
    TheTable[0x4f4f] = {2, 27};
    TheTable[0x4f51] = {2, 28};
    TheTable[0x4f53] = {2, 29};
    TheTable[0x4f55] = {2, 30};
    TheTable[0x4f59] = {2, 31};
    TheTable[0x4f5b] = {2, 32};
    TheTable[0x4f5c] = {2, 33};
    TheTable[0x4f60] = {2, 34};
    TheTable[0x4f63] = {2, 35};
    TheTable[0x4f69] = {2, 36};
    TheTable[0x4f6c] = {2, 37};
    TheTable[0x4f73] = {2, 38};
    TheTable[0x4f7f] = {2, 39};
    TheTable[0x4f84] = {2, 40};
    TheTable[0x4f88] = {2, 41};
    TheTable[0x4f8b] = {2, 42};
    TheTable[0x4f8d] = {2, 43};
    TheTable[0x4f8f] = {2, 44};
    TheTable[0x4f9b] = {2, 45};
    TheTable[0x4f9d] = {2, 46};
    TheTable[0x4fa0] = {2, 47};
    TheTable[0x4fa6] = {2, 48};
    TheTable[0x4fa7] = {2, 49};
    TheTable[0x4fa8] = {2, 50};
    TheTable[0x4fae] = {2, 51};
    TheTable[0x4fb5] = {2, 52};
    TheTable[0x4fbf] = {2, 53};
    TheTable[0x4fc3] = {2, 54};
    TheTable[0x4fc4] = {2, 55};
    TheTable[0x4fca] = {2, 56};
    TheTable[0x4fcf] = {2, 57};
    TheTable[0x4fd7] = {2, 58};
    TheTable[0x4fd8] = {2, 59};
    TheTable[0x4fdd] = {2, 60};
    TheTable[0x4fe1] = {2, 61};
    TheTable[0x4fe9] = {2, 62};
    TheTable[0x4fed] = {2, 63};
    TheTable[0x4fee] = {3, 0};
    TheTable[0x4fef] = {3, 1};
    TheTable[0x4ff1] = {3, 2};
    TheTable[0x500d] = {3, 3};
    TheTable[0x5012] = {3, 4};
    TheTable[0x5018] = {3, 5};
    TheTable[0x5019] = {3, 6};
    TheTable[0x501a] = {3, 7};
    TheTable[0x501f] = {3, 8};
    TheTable[0x5021] = {3, 9};
    TheTable[0x5026] = {3, 10};
    TheTable[0x503a] = {3, 11};
    TheTable[0x503c] = {3, 12};
    TheTable[0x503e] = {3, 13};
    TheTable[0x5047] = {3, 14};
    TheTable[0x504f] = {3, 15};
    TheTable[0x505a] = {3, 16};
    TheTable[0x505c] = {3, 17};
    TheTable[0x5065] = {3, 18};
    TheTable[0x5076] = {3, 19};
    TheTable[0x5077] = {3, 20};
    TheTable[0x507f] = {3, 21};
    TheTable[0x5085] = {3, 22};
    TheTable[0x508d] = {3, 23};
    TheTable[0x50a8] = {3, 24};
    TheTable[0x50ac] = {3, 25};
    TheTable[0x50b2] = {3, 26};
    TheTable[0x50bb] = {3, 27};
    TheTable[0x50cf] = {3, 28};
    TheTable[0x50da] = {3, 29};
    TheTable[0x50f5] = {3, 30};
    TheTable[0x50fb] = {3, 31};
    TheTable[0x5112] = {3, 32};
    TheTable[0x513f] = {3, 33};
    TheTable[0x5141] = {3, 34};
    TheTable[0x5143] = {3, 35};
    TheTable[0x5144] = {3, 36};
    TheTable[0x5145] = {3, 37};
    TheTable[0x5146] = {3, 38};
    TheTable[0x5148] = {3, 39};
    TheTable[0x5149] = {3, 40};
    TheTable[0x514b] = {3, 41};
    TheTable[0x514d] = {3, 42};
    TheTable[0x5151] = {3, 43};
    TheTable[0x5154] = {3, 44};
    TheTable[0x515a] = {3, 45};
    TheTable[0x515c] = {3, 46};
    TheTable[0x5165] = {3, 47};
    TheTable[0x5168] = {3, 48};
    TheTable[0x516b] = {3, 49};
    TheTable[0x516c] = {3, 50};
    TheTable[0x516d] = {3, 51};
    TheTable[0x5170] = {3, 52};
    TheTable[0x5171] = {3, 53};
    TheTable[0x5173] = {3, 54};
    TheTable[0x5174] = {3, 55};
    TheTable[0x5175] = {3, 56};
    TheTable[0x5176] = {3, 57};
    TheTable[0x5177] = {3, 58};
    TheTable[0x5178] = {3, 59};
    TheTable[0x5179] = {3, 60};
    TheTable[0x517b] = {3, 61};
    TheTable[0x517c] = {3, 62};
    TheTable[0x517d] = {3, 63};
    TheTable[0x5185] = {4, 0};
    TheTable[0x5188] = {4, 1};
    TheTable[0x518c] = {4, 2};
    TheTable[0x518d] = {4, 3};
    TheTable[0x5192] = {4, 4};
    TheTable[0x5199] = {4, 5};
    TheTable[0x519b] = {4, 6};
    TheTable[0x519c] = {4, 7};
    TheTable[0x51a0] = {4, 8};
    TheTable[0x51a4] = {4, 9};
    TheTable[0x51ac] = {4, 10};
    TheTable[0x51b0] = {4, 11};
    TheTable[0x51b2] = {4, 12};
    TheTable[0x51b3] = {4, 13};
    TheTable[0x51b5] = {4, 14};
    TheTable[0x51b6] = {4, 15};
    TheTable[0x51b7] = {4, 16};
    TheTable[0x51bb] = {4, 17};
    TheTable[0x51c0] = {4, 18};
    TheTable[0x51c6] = {4, 19};
    TheTable[0x51c9] = {4, 20};
    TheTable[0x51cf] = {4, 21};
    TheTable[0x51d1] = {4, 22};
    TheTable[0x51dd] = {4, 23};
    TheTable[0x51e0] = {4, 24};
    TheTable[0x51e1] = {4, 25};
    TheTable[0x51e4] = {4, 26};
    TheTable[0x51ed] = {4, 27};
    TheTable[0x51ef] = {4, 28};
    TheTable[0x51f3] = {4, 29};
    TheTable[0x51f6] = {4, 30};
    TheTable[0x51f8] = {4, 31};
    TheTable[0x51fa] = {4, 32};
    TheTable[0x51fb] = {4, 33};
    TheTable[0x5200] = {4, 34};
    TheTable[0x5203] = {4, 35};
    TheTable[0x5206] = {4, 36};
    TheTable[0x5207] = {4, 37};
    TheTable[0x520a] = {4, 38};
    TheTable[0x5211] = {4, 39};
    TheTable[0x5212] = {4, 40};
    TheTable[0x5217] = {4, 41};
    TheTable[0x5218] = {4, 42};
    TheTable[0x5219] = {4, 43};
    TheTable[0x521a] = {4, 44};
    TheTable[0x521b] = {4, 45};
    TheTable[0x521d] = {4, 46};
    TheTable[0x5220] = {4, 47};
    TheTable[0x5224] = {4, 48};
    TheTable[0x5229] = {4, 49};
    TheTable[0x522b] = {4, 50};
    TheTable[0x522e] = {4, 51};
    TheTable[0x5230] = {4, 52};
    TheTable[0x5236] = {4, 53};
    TheTable[0x5237] = {4, 54};
    TheTable[0x5238] = {4, 55};
    TheTable[0x5239] = {4, 56};
    TheTable[0x523a] = {4, 57};
    TheTable[0x523b] = {4, 58};
    TheTable[0x5242] = {4, 59};
    TheTable[0x5243] = {4, 60};
    TheTable[0x524a] = {4, 61};
    TheTable[0x524d] = {4, 62};
    TheTable[0x5251] = {4, 63};
    TheTable[0x5256] = {5, 0};
    TheTable[0x5265] = {5, 1};
    TheTable[0x5267] = {5, 2};
    TheTable[0x5269] = {5, 3};
    TheTable[0x526a] = {5, 4};
    TheTable[0x526f] = {5, 5};
    TheTable[0x5272] = {5, 6};
    TheTable[0x5288] = {5, 7};
    TheTable[0x529b] = {5, 8};
    TheTable[0x529d] = {5, 9};
    TheTable[0x529e] = {5, 10};
    TheTable[0x529f] = {5, 11};
    TheTable[0x52a0] = {5, 12};
    TheTable[0x52a1] = {5, 13};
    TheTable[0x52a3] = {5, 14};
    TheTable[0x52a8] = {5, 15};
    TheTable[0x52a9] = {5, 16};
    TheTable[0x52aa] = {5, 17};
    TheTable[0x52ab] = {5, 18};
    TheTable[0x52b1] = {5, 19};
    TheTable[0x52b2] = {5, 20};
    TheTable[0x52b3] = {5, 21};
    TheTable[0x52bf] = {5, 22};
    TheTable[0x52c3] = {5, 23};
    TheTable[0x52c7] = {5, 24};
    TheTable[0x52c9] = {5, 25};
    TheTable[0x52d2] = {5, 26};
    TheTable[0x52df] = {5, 27};
    TheTable[0x52e4] = {5, 28};
    TheTable[0x52fa] = {5, 29};
    TheTable[0x52fe] = {5, 30};
    TheTable[0x52ff] = {5, 31};
    TheTable[0x5300] = {5, 32};
    TheTable[0x5305] = {5, 33};
    TheTable[0x5306] = {5, 34};
    TheTable[0x5315] = {5, 35};
    TheTable[0x5316] = {5, 36};
    TheTable[0x5317] = {5, 37};
    TheTable[0x5319] = {5, 38};
    TheTable[0x5320] = {5, 39};
    TheTable[0x532a] = {5, 40};
    TheTable[0x5339] = {5, 41};
    TheTable[0x533a] = {5, 42};
    TheTable[0x533b] = {5, 43};
    TheTable[0x533f] = {5, 44};
    TheTable[0x5341] = {5, 45};
    TheTable[0x5343] = {5, 46};
    TheTable[0x5347] = {5, 47};
    TheTable[0x5348] = {5, 48};
    TheTable[0x534a] = {5, 49};
    TheTable[0x534e] = {5, 50};
    TheTable[0x534f] = {5, 51};
    TheTable[0x5351] = {5, 52};
    TheTable[0x5355] = {5, 53};
    TheTable[0x5356] = {5, 54};
    TheTable[0x5357] = {5, 55};
    TheTable[0x535a] = {5, 56};
    TheTable[0x535c] = {5, 57};
    TheTable[0x5360] = {5, 58};
    TheTable[0x5361] = {5, 59};
    TheTable[0x5362] = {5, 60};
    TheTable[0x5367] = {5, 61};
    TheTable[0x536b] = {5, 62};
    TheTable[0x5370] = {5, 63};
    TheTable[0x5371] = {6, 0};
    TheTable[0x5373] = {6, 1};
    TheTable[0x5374] = {6, 2};
    TheTable[0x5375] = {6, 3};
    TheTable[0x5377] = {6, 4};
    TheTable[0x5378] = {6, 5};
    TheTable[0x5382] = {6, 6};
    TheTable[0x5384] = {6, 7};
    TheTable[0x5385] = {6, 8};
    TheTable[0x5386] = {6, 9};
    TheTable[0x5389] = {6, 10};
    TheTable[0x538b] = {6, 11};
    TheTable[0x538c] = {6, 12};
    TheTable[0x5395] = {6, 13};
    TheTable[0x5398] = {6, 14};
    TheTable[0x539a] = {6, 15};
    TheTable[0x539f] = {6, 16};
    TheTable[0x53a2] = {6, 17};
    TheTable[0x53a6] = {6, 18};
    TheTable[0x53a8] = {6, 19};
    TheTable[0x53bb] = {6, 20};
    TheTable[0x53bf] = {6, 21};
    TheTable[0x53c2] = {6, 22};
    TheTable[0x53c8] = {6, 23};
    TheTable[0x53c9] = {6, 24};
    TheTable[0x53ca] = {6, 25};
    TheTable[0x53cb] = {6, 26};
    TheTable[0x53cc] = {6, 27};
    TheTable[0x53cd] = {6, 28};
    TheTable[0x53d1] = {6, 29};
    TheTable[0x53d4] = {6, 30};
    TheTable[0x53d6] = {6, 31};
    TheTable[0x53d7] = {6, 32};
    TheTable[0x53d8] = {6, 33};
    TheTable[0x53d9] = {6, 34};
    TheTable[0x53db] = {6, 35};
    TheTable[0x53e0] = {6, 36};
    TheTable[0x53e3] = {6, 37};
    TheTable[0x53e4] = {6, 38};
    TheTable[0x53e5] = {6, 39};
    TheTable[0x53e6] = {6, 40};
    TheTable[0x53e8] = {6, 41};
    TheTable[0x53ea] = {6, 42};
    TheTable[0x53eb] = {6, 43};
    TheTable[0x53ec] = {6, 44};
    TheTable[0x53ed] = {6, 45};
    TheTable[0x53ee] = {6, 46};
    TheTable[0x53ef] = {6, 47};
    TheTable[0x53f0] = {6, 48};
    TheTable[0x53f2] = {6, 49};
    TheTable[0x53f3] = {6, 50};
    TheTable[0x53f6] = {6, 51};
    TheTable[0x53f7] = {6, 52};
    TheTable[0x53f8] = {6, 53};
    TheTable[0x53f9] = {6, 54};
    TheTable[0x53fc] = {6, 55};
    TheTable[0x53fd] = {6, 56};
    TheTable[0x5403] = {6, 57};
    TheTable[0x5404] = {6, 58};
    TheTable[0x5408] = {6, 59};
    TheTable[0x5409] = {6, 60};
    TheTable[0x540a] = {6, 61};
    TheTable[0x540c] = {6, 62};
    TheTable[0x540d] = {6, 63};
    TheTable[0x540e] = {7, 0};
    TheTable[0x5410] = {7, 1};
    TheTable[0x5411] = {7, 2};
    TheTable[0x5413] = {7, 3};
    TheTable[0x5417] = {7, 4};
    TheTable[0x541b] = {7, 5};
    TheTable[0x541d] = {7, 6};
    TheTable[0x541e] = {7, 7};
    TheTable[0x541f] = {7, 8};
    TheTable[0x5426] = {7, 9};
    TheTable[0x5427] = {7, 10};
    TheTable[0x5428] = {7, 11};
    TheTable[0x5429] = {7, 12};
    TheTable[0x542b] = {7, 13};
    TheTable[0x542c] = {7, 14};
    TheTable[0x542d] = {7, 15};
    TheTable[0x542f] = {7, 16};
    TheTable[0x5434] = {7, 17};
    TheTable[0x5435] = {7, 18};
    TheTable[0x5438] = {7, 19};
    TheTable[0x5439] = {7, 20};
    TheTable[0x543b] = {7, 21};
    TheTable[0x543c] = {7, 22};
    TheTable[0x5440] = {7, 23};
    TheTable[0x5443] = {7, 24};
    TheTable[0x5446] = {7, 25};
    TheTable[0x5448] = {7, 26};
    TheTable[0x544a] = {7, 27};
    TheTable[0x5450] = {7, 28};
    TheTable[0x5455] = {7, 29};
    TheTable[0x5457] = {7, 30};
    TheTable[0x5458] = {7, 31};
    TheTable[0x545b] = {7, 32};
    TheTable[0x545c] = {7, 33};
    TheTable[0x5462] = {7, 34};
    TheTable[0x5468] = {7, 35};
    TheTable[0x5473] = {7, 36};
    TheTable[0x5475] = {7, 37};
    TheTable[0x547b] = {7, 38};
    TheTable[0x547c] = {7, 39};
    TheTable[0x547d] = {7, 40};
    TheTable[0x548b] = {7, 41};
    TheTable[0x548c] = {7, 42};
    TheTable[0x548f] = {7, 43};
    TheTable[0x5490] = {7, 44};
    TheTable[0x5492] = {7, 45};
    TheTable[0x5495] = {7, 46};
    TheTable[0x5496] = {7, 47};
    TheTable[0x5499] = {7, 48};
    TheTable[0x549a] = {7, 49};
    TheTable[0x54a6] = {7, 50};
    TheTable[0x54a7] = {7, 51};
    TheTable[0x54aa] = {7, 52};
    TheTable[0x54ac] = {7, 53};
    TheTable[0x54af] = {7, 54};
    TheTable[0x54b1] = {7, 55};
    TheTable[0x54b3] = {7, 56};
    TheTable[0x54b8] = {7, 57};
    TheTable[0x54bb] = {7, 58};
    TheTable[0x54bd] = {7, 59};
    TheTable[0x54c0] = {7, 60};
    TheTable[0x54c1] = {7, 61};
    TheTable[0x54c4] = {7, 62};
    TheTable[0x54c7] = {7, 63};
    TheTable[0x54c8] = {8, 0};
    TheTable[0x54cd] = {8, 1};
    TheTable[0x54ce] = {8, 2};
    TheTable[0x54d1] = {8, 3};
    TheTable[0x54d4] = {8, 4};
    TheTable[0x54d7] = {8, 5};
    TheTable[0x54dd] = {8, 6};
    TheTable[0x54df] = {8, 7};
    TheTable[0x54e5] = {8, 8};
    TheTable[0x54e6] = {8, 9};
    TheTable[0x54e8] = {8, 10};
    TheTable[0x54ea] = {8, 11};
    TheTable[0x54ed] = {8, 12};
    TheTable[0x54f2] = {8, 13};
    TheTable[0x54fc] = {8, 14};
    TheTable[0x5507] = {8, 15};
    TheTable[0x5509] = {8, 16};
    TheTable[0x5510] = {8, 17};
    TheTable[0x5514] = {8, 18};
    TheTable[0x5524] = {8, 19};
    TheTable[0x5527] = {8, 20};
    TheTable[0x552c] = {8, 21};
    TheTable[0x552e] = {8, 22};
    TheTable[0x552f] = {8, 23};
    TheTable[0x5531] = {8, 24};
    TheTable[0x5543] = {8, 25};
    TheTable[0x5544] = {8, 26};
    TheTable[0x5546] = {8, 27};
    TheTable[0x554a] = {8, 28};
    TheTable[0x555c] = {8, 29};
    TheTable[0x5561] = {8, 30};
    TheTable[0x5564] = {8, 31};
    TheTable[0x5565] = {8, 32};
    TheTable[0x5566] = {8, 33};
    TheTable[0x556a] = {8, 34};
    TheTable[0x556c] = {8, 35};
    TheTable[0x5582] = {8, 36};
    TheTable[0x5584] = {8, 37};
    TheTable[0x5587] = {8, 38};
    TheTable[0x5589] = {8, 39};
    TheTable[0x558a] = {8, 40};
    TheTable[0x558b] = {8, 41};
    TheTable[0x5594] = {8, 42};
    TheTable[0x5598] = {8, 43};
    TheTable[0x559c] = {8, 44};
    TheTable[0x559d] = {8, 45};
    TheTable[0x55b1] = {8, 46};
    TheTable[0x55b7] = {8, 47};
    TheTable[0x55bb] = {8, 48};
    TheTable[0x55bd] = {8, 49};
    TheTable[0x55c5] = {8, 50};
    TheTable[0x55d1] = {8, 51};
    TheTable[0x55d3] = {8, 52};
    TheTable[0x55dd] = {8, 53};
    TheTable[0x55e1] = {8, 54};
    TheTable[0x55e8] = {8, 55};
    TheTable[0x55ef] = {8, 56};
    TheTable[0x55f7] = {8, 57};
    TheTable[0x55fd] = {8, 58};
    TheTable[0x5609] = {8, 59};
    TheTable[0x560e] = {8, 60};
    TheTable[0x5618] = {8, 61};
    TheTable[0x561b] = {8, 62};
    TheTable[0x561e] = {8, 63};
    TheTable[0x561f] = {9, 0};
    TheTable[0x5623] = {9, 1};
    TheTable[0x5631] = {9, 2};
    TheTable[0x5632] = {9, 3};
    TheTable[0x5634] = {9, 4};
    TheTable[0x563b] = {9, 5};
    TheTable[0x563f] = {9, 6};
    TheTable[0x5657] = {9, 7};
    TheTable[0x5662] = {9, 8};
    TheTable[0x5668] = {9, 9};
    TheTable[0x5669] = {9, 10};
    TheTable[0x566a] = {9, 11};
    TheTable[0x5676] = {9, 12};
    TheTable[0x56a3] = {9, 13};
    TheTable[0x56af] = {9, 14};
    TheTable[0x56b7] = {9, 15};
    TheTable[0x56bc] = {9, 16};
    TheTable[0x56ca] = {9, 17};
    TheTable[0x56da] = {9, 18};
    TheTable[0x56db] = {9, 19};
    TheTable[0x56de] = {9, 20};
    TheTable[0x56e0] = {9, 21};
    TheTable[0x56e2] = {9, 22};
    TheTable[0x56ed] = {9, 23};
    TheTable[0x56f0] = {9, 24};
    TheTable[0x56f4] = {9, 25};
    TheTable[0x56fa] = {9, 26};
    TheTable[0x56fd] = {9, 27};
    TheTable[0x56fe] = {9, 28};
    TheTable[0x5706] = {9, 29};
    TheTable[0x5708] = {9, 30};
    TheTable[0x571f] = {9, 31};
    TheTable[0x5723] = {9, 32};
    TheTable[0x5728] = {9, 33};
    TheTable[0x5730] = {9, 34};
    TheTable[0x573a] = {9, 35};
    TheTable[0x573e] = {9, 36};
    TheTable[0x5740] = {9, 37};
    TheTable[0x5747] = {9, 38};
    TheTable[0x574a] = {9, 39};
    TheTable[0x574e] = {9, 40};
    TheTable[0x574f] = {9, 41};
    TheTable[0x5750] = {9, 42};
    TheTable[0x5751] = {9, 43};
    TheTable[0x5757] = {9, 44};
    TheTable[0x575a] = {9, 45};
    TheTable[0x575b] = {9, 46};
    TheTable[0x575d] = {9, 47};
    TheTable[0x575e] = {9, 48};
    TheTable[0x575f] = {9, 49};
    TheTable[0x5760] = {9, 50};
    TheTable[0x5761] = {9, 51};
    TheTable[0x5766] = {9, 52};
    TheTable[0x5768] = {9, 53};
    TheTable[0x5782] = {9, 54};
    TheTable[0x5783] = {9, 55};
    TheTable[0x5784] = {9, 56};
    TheTable[0x578b] = {9, 57};
    TheTable[0x5792] = {9, 58};
    TheTable[0x57a6] = {9, 59};
    TheTable[0x57ab] = {9, 60};
    TheTable[0x57ae] = {9, 61};
    TheTable[0x57c3] = {9, 62};
    TheTable[0x57cb] = {9, 63};
    TheTable[0x57ce] = {10, 0};
    TheTable[0x57df] = {10, 1};
    TheTable[0x57f9] = {10, 2};
    TheTable[0x57fa] = {10, 3};
    TheTable[0x5802] = {10, 4};
    TheTable[0x5806] = {10, 5};
    TheTable[0x5815] = {10, 6};
    TheTable[0x5821] = {10, 7};
    TheTable[0x5824] = {10, 8};
    TheTable[0x582a] = {10, 9};
    TheTable[0x5835] = {10, 10};
    TheTable[0x584c] = {10, 11};
    TheTable[0x5851] = {10, 12};
    TheTable[0x5854] = {10, 13};
    TheTable[0x5858] = {10, 14};
    TheTable[0x585e] = {10, 15};
    TheTable[0x586b] = {10, 16};
    TheTable[0x5883] = {10, 17};
    TheTable[0x5885] = {10, 18};
    TheTable[0x5893] = {10, 19};
    TheTable[0x5899] = {10, 20};
    TheTable[0x589e] = {10, 21};
    TheTable[0x589f] = {10, 22};
    TheTable[0x58a8] = {10, 23};
    TheTable[0x58c1] = {10, 24};
    TheTable[0x58d5] = {10, 25};
    TheTable[0x58e4] = {10, 26};
    TheTable[0x58eb] = {10, 27};
    TheTable[0x58ee] = {10, 28};
    TheTable[0x58f0] = {10, 29};
    TheTable[0x58f3] = {10, 30};
    TheTable[0x58f6] = {10, 31};
    TheTable[0x5904] = {10, 32};
    TheTable[0x5907] = {10, 33};
    TheTable[0x590d] = {10, 34};
    TheTable[0x590f] = {10, 35};
    TheTable[0x5915] = {10, 36};
    TheTable[0x5916] = {10, 37};
    TheTable[0x591a] = {10, 38};
    TheTable[0x591c] = {10, 39};
    TheTable[0x591f] = {10, 40};
    TheTable[0x5927] = {10, 41};
    TheTable[0x5929] = {10, 42};
    TheTable[0x592a] = {10, 43};
    TheTable[0x592b] = {10, 44};
    TheTable[0x592e] = {10, 45};
    TheTable[0x5931] = {10, 46};
    TheTable[0x5934] = {10, 47};
    TheTable[0x5937] = {10, 48};
    TheTable[0x5938] = {10, 49};
    TheTable[0x5939] = {10, 50};
    TheTable[0x593a] = {10, 51};
    TheTable[0x5947] = {10, 52};
    TheTable[0x5949] = {10, 53};
    TheTable[0x594b] = {10, 54};
    TheTable[0x594f] = {10, 55};
    TheTable[0x5951] = {10, 56};
    TheTable[0x5954] = {10, 57};
    TheTable[0x5956] = {10, 58};
    TheTable[0x5957] = {10, 59};
    TheTable[0x595a] = {10, 60};
    TheTable[0x5962] = {10, 61};
    TheTable[0x5965] = {10, 62};
    TheTable[0x5973] = {10, 63};
    TheTable[0x5974] = {11, 0};
    TheTable[0x5976] = {11, 1};
    TheTable[0x5978] = {11, 2};
    TheTable[0x5979] = {11, 3};
    TheTable[0x597d] = {11, 4};
    TheTable[0x5982] = {11, 5};
    TheTable[0x5984] = {11, 6};
    TheTable[0x5986] = {11, 7};
    TheTable[0x5987] = {11, 8};
    TheTable[0x5988] = {11, 9};
    TheTable[0x5992] = {11, 10};
    TheTable[0x5993] = {11, 11};
    TheTable[0x5996] = {11, 12};
    TheTable[0x5999] = {11, 13};
    TheTable[0x599e] = {11, 14};
    TheTable[0x59a5] = {11, 15};
    TheTable[0x59a8] = {11, 16};
    TheTable[0x59b9] = {11, 17};
    TheTable[0x59bb] = {11, 18};
    TheTable[0x59c6] = {11, 19};
    TheTable[0x59cb] = {11, 20};
    TheTable[0x59d0] = {11, 21};
    TheTable[0x59d1] = {11, 22};
    TheTable[0x59d3] = {11, 23};
    TheTable[0x59d4] = {11, 24};
    TheTable[0x59dc] = {11, 25};
    TheTable[0x59e5] = {11, 26};
    TheTable[0x59e8] = {11, 27};
    TheTable[0x59fb] = {11, 28};
    TheTable[0x59ff] = {11, 29};
    TheTable[0x5a01] = {11, 30};
    TheTable[0x5a03] = {11, 31};
    TheTable[0x5a07] = {11, 32};
    TheTable[0x5a18] = {11, 33};
    TheTable[0x5a1c] = {11, 34};
    TheTable[0x5a31] = {11, 35};
    TheTable[0x5a46] = {11, 36};
    TheTable[0x5a4a] = {11, 37};
    TheTable[0x5a5a] = {11, 38};
    TheTable[0x5a74] = {11, 39};
    TheTable[0x5a76] = {11, 40};
    TheTable[0x5a92] = {11, 41};
    TheTable[0x5a9a] = {11, 42};
    TheTable[0x5ac1] = {11, 43};
    TheTable[0x5ac2] = {11, 44};
    TheTable[0x5ac9] = {11, 45};
    TheTable[0x5acc] = {11, 46};
    TheTable[0x5ad6] = {11, 47};
    TheTable[0x5ae9] = {11, 48};
    TheTable[0x5b09] = {11, 49};
    TheTable[0x5b50] = {11, 50};
    TheTable[0x5b54] = {11, 51};
    TheTable[0x5b55] = {11, 52};
    TheTable[0x5b57] = {11, 53};
    TheTable[0x5b58] = {11, 54};
    TheTable[0x5b59] = {11, 55};
    TheTable[0x5b5d] = {11, 56};
    TheTable[0x5b5f] = {11, 57};
    TheTable[0x5b63] = {11, 58};
    TheTable[0x5b64] = {11, 59};
    TheTable[0x5b66] = {11, 60};
    TheTable[0x5b69] = {11, 61};
    TheTable[0x5b6c] = {11, 62};
    TheTable[0x5b70] = {11, 63};
    TheTable[0x5b7d] = {12, 0};
    TheTable[0x5b81] = {12, 1};
    TheTable[0x5b83] = {12, 2};
    TheTable[0x5b85] = {12, 3};
    TheTable[0x5b87] = {12, 4};
    TheTable[0x5b88] = {12, 5};
    TheTable[0x5b89] = {12, 6};
    TheTable[0x5b8b] = {12, 7};
    TheTable[0x5b8c] = {12, 8};
    TheTable[0x5b8f] = {12, 9};
    TheTable[0x5b97] = {12, 10};
    TheTable[0x5b98] = {12, 11};
    TheTable[0x5b99] = {12, 12};
    TheTable[0x5b9a] = {12, 13};
    TheTable[0x5b9c] = {12, 14};
    TheTable[0x5b9d] = {12, 15};
    TheTable[0x5b9e] = {12, 16};
    TheTable[0x5ba0] = {12, 17};
    TheTable[0x5ba1] = {12, 18};
    TheTable[0x5ba2] = {12, 19};
    TheTable[0x5ba3] = {12, 20};
    TheTable[0x5ba4] = {12, 21};
    TheTable[0x5baa] = {12, 22};
    TheTable[0x5bab] = {12, 23};
    TheTable[0x5bb0] = {12, 24};
    TheTable[0x5bb3] = {12, 25};
    TheTable[0x5bb4] = {12, 26};
    TheTable[0x5bb5] = {12, 27};
    TheTable[0x5bb6] = {12, 28};
    TheTable[0x5bb9] = {12, 29};
    TheTable[0x5bbd] = {12, 30};
    TheTable[0x5bbe] = {12, 31};
    TheTable[0x5bbf] = {12, 32};
    TheTable[0x5bc2] = {12, 33};
    TheTable[0x5bc4] = {12, 34};
    TheTable[0x5bc6] = {12, 35};
    TheTable[0x5bc7] = {12, 36};
    TheTable[0x5bcc] = {12, 37};
    TheTable[0x5bd2] = {12, 38};
    TheTable[0x5bd3] = {12, 39};
    TheTable[0x5bde] = {12, 40};
    TheTable[0x5bdf] = {12, 41};
    TheTable[0x5be8] = {12, 42};
    TheTable[0x5bf8] = {12, 43};
    TheTable[0x5bf9] = {12, 44};
    TheTable[0x5bfa] = {12, 45};
    TheTable[0x5bfb] = {12, 46};
    TheTable[0x5bfc] = {12, 47};
    TheTable[0x5bff] = {12, 48};
    TheTable[0x5c01] = {12, 49};
    TheTable[0x5c04] = {12, 50};
    TheTable[0x5c06] = {12, 51};
    TheTable[0x5c09] = {12, 52};
    TheTable[0x5c0a] = {12, 53};
    TheTable[0x5c0f] = {12, 54};
    TheTable[0x5c11] = {12, 55};
    TheTable[0x5c14] = {12, 56};
    TheTable[0x5c16] = {12, 57};
    TheTable[0x5c18] = {12, 58};
    TheTable[0x5c1a] = {12, 59};
    TheTable[0x5c1d] = {12, 60};
    TheTable[0x5c24] = {12, 61};
    TheTable[0x5c2c] = {12, 62};
    TheTable[0x5c31] = {12, 63};
    TheTable[0x5c34] = {13, 0};
    TheTable[0x5c38] = {13, 1};
    TheTable[0x5c3a] = {13, 2};
    TheTable[0x5c3c] = {13, 3};
    TheTable[0x5c3d] = {13, 4};
    TheTable[0x5c3e] = {13, 5};
    TheTable[0x5c3f] = {13, 6};
    TheTable[0x5c40] = {13, 7};
    TheTable[0x5c41] = {13, 8};
    TheTable[0x5c42] = {13, 9};
    TheTable[0x5c45] = {13, 10};
    TheTable[0x5c48] = {13, 11};
    TheTable[0x5c4a] = {13, 12};
    TheTable[0x5c4b] = {13, 13};
    TheTable[0x5c4e] = {13, 14};
    TheTable[0x5c4f] = {13, 15};
    TheTable[0x5c51] = {13, 16};
    TheTable[0x5c55] = {13, 17};
    TheTable[0x5c5e] = {13, 18};
    TheTable[0x5c60] = {13, 19};
    TheTable[0x5c61] = {13, 20};
    TheTable[0x5c6f] = {13, 21};
    TheTable[0x5c71] = {13, 22};
    TheTable[0x5c7f] = {13, 23};
    TheTable[0x5c81] = {13, 24};
    TheTable[0x5c82] = {13, 25};
    TheTable[0x5c94] = {13, 26};
    TheTable[0x5c97] = {13, 27};
    TheTable[0x5c98] = {13, 28};
    TheTable[0x5c9b] = {13, 29};
    TheTable[0x5ca9] = {13, 30};
    TheTable[0x5cad] = {13, 31};
    TheTable[0x5cb8] = {13, 32};
    TheTable[0x5cd9] = {13, 33};
    TheTable[0x5ce1] = {13, 34};
    TheTable[0x5cf0] = {13, 35};
    TheTable[0x5d07] = {13, 36};
    TheTable[0x5d16] = {13, 37};
    TheTable[0x5d1b] = {13, 38};
    TheTable[0x5d29] = {13, 39};
    TheTable[0x5d2d] = {13, 40};
    TheTable[0x5d3d] = {13, 41};
    TheTable[0x5dc5] = {13, 42};
    TheTable[0x5ddd] = {13, 43};
    TheTable[0x5dde] = {13, 44};
    TheTable[0x5de1] = {13, 45};
    TheTable[0x5de2] = {13, 46};
    TheTable[0x5de5] = {13, 47};
    TheTable[0x5de6] = {13, 48};
    TheTable[0x5de7] = {13, 49};
    TheTable[0x5de8] = {13, 50};
    TheTable[0x5de9] = {13, 51};
    TheTable[0x5dee] = {13, 52};
    TheTable[0x5df1] = {13, 53};
    TheTable[0x5df2] = {13, 54};
    TheTable[0x5df4] = {13, 55};
    TheTable[0x5df7] = {13, 56};
    TheTable[0x5dfe] = {13, 57};
    TheTable[0x5e01] = {13, 58};
    TheTable[0x5e02] = {13, 59};
    TheTable[0x5e03] = {13, 60};
    TheTable[0x5e05] = {13, 61};
    TheTable[0x5e06] = {13, 62};
    TheTable[0x5e08] = {13, 63};
    TheTable[0x5e0c] = {14, 0};
    TheTable[0x5e10] = {14, 1};
    TheTable[0x5e15] = {14, 2};
    TheTable[0x5e16] = {14, 3};
    TheTable[0x5e18] = {14, 4};
    TheTable[0x5e1a] = {14, 5};
    TheTable[0x5e1c] = {14, 6};
    TheTable[0x5e1d] = {14, 7};
    TheTable[0x5e26] = {14, 8};
    TheTable[0x5e27] = {14, 9};
    TheTable[0x5e2d] = {14, 10};
    TheTable[0x5e2e] = {14, 11};
    TheTable[0x5e38] = {14, 12};
    TheTable[0x5e3d] = {14, 13};
    TheTable[0x5e45] = {14, 14};
    TheTable[0x5e55] = {14, 15};
    TheTable[0x5e72] = {14, 16};
    TheTable[0x5e73] = {14, 17};
    TheTable[0x5e74] = {14, 18};
    TheTable[0x5e76] = {14, 19};
    TheTable[0x5e78] = {14, 20};
    TheTable[0x5e7b] = {14, 21};
    TheTable[0x5e7c] = {14, 22};
    TheTable[0x5e7d] = {14, 23};
    TheTable[0x5e7f] = {14, 24};
    TheTable[0x5e84] = {14, 25};
    TheTable[0x5e86] = {14, 26};
    TheTable[0x5e8a] = {14, 27};
    TheTable[0x5e8f] = {14, 28};
    TheTable[0x5e93] = {14, 29};
    TheTable[0x5e94] = {14, 30};
    TheTable[0x5e95] = {14, 31};
    TheTable[0x5e97] = {14, 32};
    TheTable[0x5e99] = {14, 33};
    TheTable[0x5e9c] = {14, 34};
    TheTable[0x5e9f] = {14, 35};
    TheTable[0x5ea6] = {14, 36};
    TheTable[0x5ea7] = {14, 37};
    TheTable[0x5ead] = {14, 38};
    TheTable[0x5eb7] = {14, 39};
    TheTable[0x5eb8] = {14, 40};
    TheTable[0x5ec9] = {14, 41};
    TheTable[0x5eca] = {14, 42};
    TheTable[0x5ed3] = {14, 43};
    TheTable[0x5ef6] = {14, 44};
    TheTable[0x5efa] = {14, 45};
    TheTable[0x5f00] = {14, 46};
    TheTable[0x5f02] = {14, 47};
    TheTable[0x5f03] = {14, 48};
    TheTable[0x5f04] = {14, 49};
    TheTable[0x5f0a] = {14, 50};
    TheTable[0x5f0f] = {14, 51};
    TheTable[0x5f13] = {14, 52};
    TheTable[0x5f15] = {14, 53};
    TheTable[0x5f17] = {14, 54};
    TheTable[0x5f1f] = {14, 55};
    TheTable[0x5f20] = {14, 56};
    TheTable[0x5f26] = {14, 57};
    TheTable[0x5f2f] = {14, 58};
    TheTable[0x5f31] = {14, 59};
    TheTable[0x5f39] = {14, 60};
    TheTable[0x5f3a] = {14, 61};
    TheTable[0x5f52] = {14, 62};
    TheTable[0x5f53] = {14, 63};
    TheTable[0x5f55] = {15, 0};
    TheTable[0x5f57] = {15, 1};
    TheTable[0x5f62] = {15, 2};
    TheTable[0x5f69] = {15, 3};
    TheTable[0x5f6d] = {15, 4};
    TheTable[0x5f71] = {15, 5};
    TheTable[0x5f79] = {15, 6};
    TheTable[0x5f7b] = {15, 7};
    TheTable[0x5f7c] = {15, 8};
    TheTable[0x5f80] = {15, 9};
    TheTable[0x5f81] = {15, 10};
    TheTable[0x5f84] = {15, 11};
    TheTable[0x5f85] = {15, 12};
    TheTable[0x5f88] = {15, 13};
    TheTable[0x5f8b] = {15, 14};
    TheTable[0x5f90] = {15, 15};
    TheTable[0x5f92] = {15, 16};
    TheTable[0x5f97] = {15, 17};
    TheTable[0x5fa1] = {15, 18};
    TheTable[0x5faa] = {15, 19};
    TheTable[0x5fae] = {15, 20};
    TheTable[0x5fb7] = {15, 21};
    TheTable[0x5fbd] = {15, 22};
    TheTable[0x5fc3] = {15, 23};
    TheTable[0x5fc5] = {15, 24};
    TheTable[0x5fc6] = {15, 25};
    TheTable[0x5fcc] = {15, 26};
    TheTable[0x5fcd] = {15, 27};
    TheTable[0x5fcf] = {15, 28};
    TheTable[0x5fd2] = {15, 29};
    TheTable[0x5fd7] = {15, 30};
    TheTable[0x5fd8] = {15, 31};
    TheTable[0x5fd9] = {15, 32};
    TheTable[0x5fe0] = {15, 33};
    TheTable[0x5fe7] = {15, 34};
    TheTable[0x5feb] = {15, 35};
    TheTable[0x5ff5] = {15, 36};
    TheTable[0x5ffd] = {15, 37};
    TheTable[0x6000] = {15, 38};
    TheTable[0x6001] = {15, 39};
    TheTable[0x600e] = {15, 40};
    TheTable[0x6012] = {15, 41};
    TheTable[0x6015] = {15, 42};
    TheTable[0x6016] = {15, 43};
    TheTable[0x601c] = {15, 44};
    TheTable[0x601d] = {15, 45};
    TheTable[0x6020] = {15, 46};
    TheTable[0x6025] = {15, 47};
    TheTable[0x6027] = {15, 48};
    TheTable[0x6028] = {15, 49};
    TheTable[0x602a] = {15, 50};
    TheTable[0x603b] = {15, 51};
    TheTable[0x604b] = {15, 52};
    TheTable[0x604d] = {15, 53};
    TheTable[0x6050] = {15, 54};
    TheTable[0x6052] = {15, 55};
    TheTable[0x6055] = {15, 56};
    TheTable[0x6059] = {15, 57};
    TheTable[0x6062] = {15, 58};
    TheTable[0x6064] = {15, 59};
    TheTable[0x6068] = {15, 60};
    TheTable[0x6069] = {15, 61};
    TheTable[0x606d] = {15, 62};
    TheTable[0x606f] = {15, 63};
    TheTable[0x6070] = {16, 0};
    TheTable[0x6073] = {16, 1};
    TheTable[0x6076] = {16, 2};
    TheTable[0x607a] = {16, 3};
    TheTable[0x607c] = {16, 4};
    TheTable[0x6084] = {16, 5};
    TheTable[0x6089] = {16, 6};
    TheTable[0x608d] = {16, 7};
    TheTable[0x6094] = {16, 8};
    TheTable[0x609a] = {16, 9};
    TheTable[0x609f] = {16, 10};
    TheTable[0x60a0] = {16, 11};
    TheTable[0x60a3] = {16, 12};
    TheTable[0x60a6] = {16, 13};
    TheTable[0x60a8] = {16, 14};
    TheTable[0x60ac] = {16, 15};
    TheTable[0x60b2] = {16, 16};
    TheTable[0x60bc] = {16, 17};
    TheTable[0x60c5] = {16, 18};
    TheTable[0x60ca] = {16, 19};
    TheTable[0x60cb] = {16, 20};
    TheTable[0x60d1] = {16, 21};
    TheTable[0x60d5] = {16, 22};
    TheTable[0x60da] = {16, 23};
    TheTable[0x60dc] = {16, 24};
    TheTable[0x60df] = {16, 25};
    TheTable[0x60e0] = {16, 26};
    TheTable[0x60e7] = {16, 27};
    TheTable[0x60e8] = {16, 28};
    TheTable[0x60e9] = {16, 29};
    TheTable[0x60ed] = {16, 30};
    TheTable[0x60ef] = {16, 31};
    TheTable[0x60f0] = {16, 32};
    TheTable[0x60f3] = {16, 33};
    TheTable[0x60f9] = {16, 34};
    TheTable[0x6101] = {16, 35};
    TheTable[0x6108] = {16, 36};
    TheTable[0x6109] = {16, 37};
    TheTable[0x610f] = {16, 38};
    TheTable[0x611a] = {16, 39};
    TheTable[0x611f] = {16, 40};
    TheTable[0x6123] = {16, 41};
    TheTable[0x6124] = {16, 42};
    TheTable[0x6127] = {16, 43};
    TheTable[0x613f] = {16, 44};
    TheTable[0x6148] = {16, 45};
    TheTable[0x614c] = {16, 46};
    TheTable[0x614e] = {16, 47};
    TheTable[0x6155] = {16, 48};
    TheTable[0x6162] = {16, 49};
    TheTable[0x6167] = {16, 50};
    TheTable[0x6168] = {16, 51};
    TheTable[0x6170] = {16, 52};
    TheTable[0x618b] = {16, 53};
    TheTable[0x618e] = {16, 54};
    TheTable[0x61a9] = {16, 55};
    TheTable[0x61be] = {16, 56};
    TheTable[0x61c2] = {16, 57};
    TheTable[0x61d2] = {16, 58};
    TheTable[0x61e6] = {16, 59};
    TheTable[0x6208] = {16, 60};
    TheTable[0x620f] = {16, 61};
    TheTable[0x6210] = {16, 62};
    TheTable[0x6211] = {16, 63};
    TheTable[0x6212] = {17, 0};
    TheTable[0x6216] = {17, 1};
    TheTable[0x6218] = {17, 2};
    TheTable[0x621a] = {17, 3};
    TheTable[0x622a] = {17, 4};
    TheTable[0x622e] = {17, 5};
    TheTable[0x6233] = {17, 6};
    TheTable[0x6234] = {17, 7};
    TheTable[0x6237] = {17, 8};
    TheTable[0x623f] = {17, 9};
    TheTable[0x6240] = {17, 10};
    TheTable[0x6241] = {17, 11};
    TheTable[0x6247] = {17, 12};
    TheTable[0x624b] = {17, 13};
    TheTable[0x624d] = {17, 14};
    TheTable[0x624e] = {17, 15};
    TheTable[0x6251] = {17, 16};
    TheTable[0x6252] = {17, 17};
    TheTable[0x6253] = {17, 18};
    TheTable[0x6254] = {17, 19};
    TheTable[0x6258] = {17, 20};
    TheTable[0x625b] = {17, 21};
    TheTable[0x6263] = {17, 22};
    TheTable[0x6267] = {17, 23};
    TheTable[0x6269] = {17, 24};
    TheTable[0x626a] = {17, 25};
    TheTable[0x626b] = {17, 26};
    TheTable[0x626c] = {17, 27};
    TheTable[0x626d] = {17, 28};
    TheTable[0x626e] = {17, 29};
    TheTable[0x626f] = {17, 30};
    TheTable[0x6270] = {17, 31};
    TheTable[0x6273] = {17, 32};
    TheTable[0x6276] = {17, 33};
    TheTable[0x6279] = {17, 34};
    TheTable[0x627c] = {17, 35};
    TheTable[0x627e] = {17, 36};
    TheTable[0x627f] = {17, 37};
    TheTable[0x6280] = {17, 38};
    TheTable[0x6284] = {17, 39};
    TheTable[0x628a] = {17, 40};
    TheTable[0x6291] = {17, 41};
    TheTable[0x6293] = {17, 42};
    TheTable[0x6295] = {17, 43};
    TheTable[0x6296] = {17, 44};
    TheTable[0x6297] = {17, 45};
    TheTable[0x6298] = {17, 46};
    TheTable[0x629a] = {17, 47};
    TheTable[0x629b] = {17, 48};
    TheTable[0x62a2] = {17, 49};
    TheTable[0x62a4] = {17, 50};
    TheTable[0x62a5] = {17, 51};
    TheTable[0x62ab] = {17, 52};
    TheTable[0x62ac] = {17, 53};
    TheTable[0x62b1] = {17, 54};
    TheTable[0x62b5] = {17, 55};
    TheTable[0x62b9] = {17, 56};
    TheTable[0x62bc] = {17, 57};
    TheTable[0x62bd] = {17, 58};
    TheTable[0x62c5] = {17, 59};
    TheTable[0x62c6] = {17, 60};
    TheTable[0x62c7] = {17, 61};
    TheTable[0x62c9] = {17, 62};
    TheTable[0x62cc] = {17, 63};
    TheTable[0x62cd] = {18, 0};
    TheTable[0x62d0] = {18, 1};
    TheTable[0x62d2] = {18, 2};
    TheTable[0x62d4] = {18, 3};
    TheTable[0x62d6] = {18, 4};
    TheTable[0x62d8] = {18, 5};
    TheTable[0x62db] = {18, 6};
    TheTable[0x62dc] = {18, 7};
    TheTable[0x62df] = {18, 8};
    TheTable[0x62e2] = {18, 9};
    TheTable[0x62e3] = {18, 10};
    TheTable[0x62e5] = {18, 11};
    TheTable[0x62e6] = {18, 12};
    TheTable[0x62e8] = {18, 13};
    TheTable[0x62e9] = {18, 14};
    TheTable[0x62ec] = {18, 15};
    TheTable[0x62ef] = {18, 16};
    TheTable[0x62f3] = {18, 17};
    TheTable[0x62f4] = {18, 18};
    TheTable[0x62fc] = {18, 19};
    TheTable[0x62fd] = {18, 20};
    TheTable[0x62fe] = {18, 21};
    TheTable[0x62ff] = {18, 22};
    TheTable[0x6301] = {18, 23};
    TheTable[0x6302] = {18, 24};
    TheTable[0x6307] = {18, 25};
    TheTable[0x6309] = {18, 26};
    TheTable[0x630e] = {18, 27};
    TheTable[0x6311] = {18, 28};
    TheTable[0x6316] = {18, 29};
    TheTable[0x631a] = {18, 30};
    TheTable[0x6320] = {18, 31};
    TheTable[0x6321] = {18, 32};
    TheTable[0x6323] = {18, 33};
    TheTable[0x6324] = {18, 34};
    TheTable[0x6325] = {18, 35};
    TheTable[0x6328] = {18, 36};
    TheTable[0x632a] = {18, 37};
    TheTable[0x632b] = {18, 38};
    TheTable[0x632f] = {18, 39};
    TheTable[0x633a] = {18, 40};
    TheTable[0x633d] = {18, 41};
    TheTable[0x6345] = {18, 42};
    TheTable[0x6346] = {18, 43};
    TheTable[0x6349] = {18, 44};
    TheTable[0x634d] = {18, 45};
    TheTable[0x634e] = {18, 46};
    TheTable[0x634f] = {18, 47};
    TheTable[0x6350] = {18, 48};
    TheTable[0x6355] = {18, 49};
    TheTable[0x635e] = {18, 50};
    TheTable[0x635f] = {18, 51};
    TheTable[0x6361] = {18, 52};
    TheTable[0x6362] = {18, 53};
    TheTable[0x6363] = {18, 54};
    TheTable[0x6367] = {18, 55};
    TheTable[0x636e] = {18, 56};
    TheTable[0x6377] = {18, 57};
    TheTable[0x6380] = {18, 58};
    TheTable[0x6388] = {18, 59};
    TheTable[0x6389] = {18, 60};
    TheTable[0x638c] = {18, 61};
    TheTable[0x638f] = {18, 62};
    TheTable[0x6392] = {18, 63};
    TheTable[0x6398] = {19, 0};
    TheTable[0x63a0] = {19, 1};
    TheTable[0x63a2] = {19, 2};
    TheTable[0x63a5] = {19, 3};
    TheTable[0x63a7] = {19, 4};
    TheTable[0x63a8] = {19, 5};
    TheTable[0x63a9] = {19, 6};
    TheTable[0x63aa] = {19, 7};
    TheTable[0x63b7] = {19, 8};
    TheTable[0x63c9] = {19, 9};
    TheTable[0x63cd] = {19, 10};
    TheTable[0x63cf] = {19, 11};
    TheTable[0x63d0] = {19, 12};
    TheTable[0x63d2] = {19, 13};
    TheTable[0x63e1] = {19, 14};
    TheTable[0x63ea] = {19, 15};
    TheTable[0x63ed] = {19, 16};
    TheTable[0x63f4] = {19, 17};
    TheTable[0x6400] = {19, 18};
    TheTable[0x6401] = {19, 19};
    TheTable[0x6402] = {19, 20};
    TheTable[0x6405] = {19, 21};
    TheTable[0x640f] = {19, 22};
    TheTable[0x641c] = {19, 23};
    TheTable[0x641e] = {19, 24};
    TheTable[0x642c] = {19, 25};
    TheTable[0x642d] = {19, 26};
    TheTable[0x643a] = {19, 27};
    TheTable[0x6444] = {19, 28};
    TheTable[0x6446] = {19, 29};
    TheTable[0x6447] = {19, 30};
    TheTable[0x644a] = {19, 31};
    TheTable[0x6454] = {19, 32};
    TheTable[0x6458] = {19, 33};
    TheTable[0x6467] = {19, 34};
    TheTable[0x6469] = {19, 35};
    TheTable[0x6478] = {19, 36};
    TheTable[0x6487] = {19, 37};
    TheTable[0x6491] = {19, 38};
    TheTable[0x6492] = {19, 39};
    TheTable[0x6495] = {19, 40};
    TheTable[0x649e] = {19, 41};
    TheTable[0x64a4] = {19, 42};
    TheTable[0x64ad] = {19, 43};
    TheTable[0x64b5] = {19, 44};
    TheTable[0x64b8] = {19, 45};
    TheTable[0x64c5] = {19, 46};
    TheTable[0x64cd] = {19, 47};
    TheTable[0x64ce] = {19, 48};
    TheTable[0x64e6] = {19, 49};
    TheTable[0x6500] = {19, 50};
    TheTable[0x652f] = {19, 51};
    TheTable[0x6536] = {19, 52};
    TheTable[0x6539] = {19, 53};
    TheTable[0x653b] = {19, 54};
    TheTable[0x653e] = {19, 55};
    TheTable[0x653f] = {19, 56};
    TheTable[0x6545] = {19, 57};
    TheTable[0x6548] = {19, 58};
    TheTable[0x654c] = {19, 59};
    TheTable[0x654f] = {19, 60};
    TheTable[0x6551] = {19, 61};
    TheTable[0x6559] = {19, 62};
    TheTable[0x655b] = {19, 63};
    TheTable[0x655e] = {20, 0};
    TheTable[0x6562] = {20, 1};
    TheTable[0x6563] = {20, 2};
    TheTable[0x656c] = {20, 3};
    TheTable[0x6570] = {20, 4};
    TheTable[0x6572] = {20, 5};
    TheTable[0x6574] = {20, 6};
    TheTable[0x6587] = {20, 7};
    TheTable[0x6590] = {20, 8};
    TheTable[0x6591] = {20, 9};
    TheTable[0x6597] = {20, 10};
    TheTable[0x6599] = {20, 11};
    TheTable[0x659c] = {20, 12};
    TheTable[0x65a4] = {20, 13};
    TheTable[0x65a5] = {20, 14};
    TheTable[0x65a7] = {20, 15};
    TheTable[0x65a9] = {20, 16};
    TheTable[0x65ad] = {20, 17};
    TheTable[0x65af] = {20, 18};
    TheTable[0x65b0] = {20, 19};
    TheTable[0x65b9] = {20, 20};
    TheTable[0x65bd] = {20, 21};
    TheTable[0x65c1] = {20, 22};
    TheTable[0x65c5] = {20, 23};
    TheTable[0x65cb] = {20, 24};
    TheTable[0x65cf] = {20, 25};
    TheTable[0x65d7] = {20, 26};
    TheTable[0x65e0] = {20, 27};
    TheTable[0x65e2] = {20, 28};
    TheTable[0x65e5] = {20, 29};
    TheTable[0x65e6] = {20, 30};
    TheTable[0x65e7] = {20, 31};
    TheTable[0x65e8] = {20, 32};
    TheTable[0x65e9] = {20, 33};
    TheTable[0x65ec] = {20, 34};
    TheTable[0x65f1] = {20, 35};
    TheTable[0x65f6] = {20, 36};
    TheTable[0x65f7] = {20, 37};
    TheTable[0x65fa] = {20, 38};
    TheTable[0x6602] = {20, 39};
    TheTable[0x6606] = {20, 40};
    TheTable[0x660c] = {20, 41};
    TheTable[0x660e] = {20, 42};
    TheTable[0x660f] = {20, 43};
    TheTable[0x6613] = {20, 44};
    TheTable[0x661f] = {20, 45};
    TheTable[0x6620] = {20, 46};
    TheTable[0x6625] = {20, 47};
    TheTable[0x6628] = {20, 48};
    TheTable[0x662d] = {20, 49};
    TheTable[0x662f] = {20, 50};
    TheTable[0x663c] = {20, 51};
    TheTable[0x663e] = {20, 52};
    TheTable[0x6643] = {20, 53};
    TheTable[0x664b] = {20, 54};
    TheTable[0x664c] = {20, 55};
    TheTable[0x6652] = {20, 56};
    TheTable[0x6653] = {20, 57};
    TheTable[0x6655] = {20, 58};
    TheTable[0x665a] = {20, 59};
    TheTable[0x6668] = {20, 60};
    TheTable[0x666e] = {20, 61};
    TheTable[0x666f] = {20, 62};
    TheTable[0x6674] = {20, 63};
    TheTable[0x6676] = {21, 0};
    TheTable[0x667a] = {21, 1};
    TheTable[0x6682] = {21, 2};
    TheTable[0x6691] = {21, 3};
    TheTable[0x6696] = {21, 4};
    TheTable[0x6697] = {21, 5};
    TheTable[0x66ae] = {21, 6};
    TheTable[0x66b4] = {21, 7};
    TheTable[0x66dd] = {21, 8};
    TheTable[0x66f2] = {21, 9};
    TheTable[0x66f4] = {21, 10};
    TheTable[0x66fc] = {21, 11};
    TheTable[0x66fe] = {21, 12};
    TheTable[0x66ff] = {21, 13};
    TheTable[0x6700] = {21, 14};
    TheTable[0x6708] = {21, 15};
    TheTable[0x6709] = {21, 16};
    TheTable[0x670b] = {21, 17};
    TheTable[0x670d] = {21, 18};
    TheTable[0x6717] = {21, 19};
    TheTable[0x671b] = {21, 20};
    TheTable[0x671d] = {21, 21};
    TheTable[0x671f] = {21, 22};
    TheTable[0x6728] = {21, 23};
    TheTable[0x672a] = {21, 24};
    TheTable[0x672b] = {21, 25};
    TheTable[0x672c] = {21, 26};
    TheTable[0x672f] = {21, 27};
    TheTable[0x6731] = {21, 28};
    TheTable[0x6734] = {21, 29};
    TheTable[0x6735] = {21, 30};
    TheTable[0x673a] = {21, 31};
    TheTable[0x673d] = {21, 32};
    TheTable[0x6740] = {21, 33};
    TheTable[0x6742] = {21, 34};
    TheTable[0x6743] = {21, 35};
    TheTable[0x6746] = {21, 36};
    TheTable[0x674e] = {21, 37};
    TheTable[0x674f] = {21, 38};
    TheTable[0x6750] = {21, 39};
    TheTable[0x6751] = {21, 40};
    TheTable[0x675c] = {21, 41};
    TheTable[0x675f] = {21, 42};
    TheTable[0x6760] = {21, 43};
    TheTable[0x6761] = {21, 44};
    TheTable[0x6765] = {21, 45};
    TheTable[0x6768] = {21, 46};
    TheTable[0x676f] = {21, 47};
    TheTable[0x6770] = {21, 48};
    TheTable[0x677e] = {21, 49};
    TheTable[0x677f] = {21, 50};
    TheTable[0x6781] = {21, 51};
    TheTable[0x6784] = {21, 52};
    TheTable[0x6790] = {21, 53};
    TheTable[0x6795] = {21, 54};
    TheTable[0x6797] = {21, 55};
    TheTable[0x679c] = {21, 56};
    TheTable[0x679d] = {21, 57};
    TheTable[0x67a3] = {21, 58};
    TheTable[0x67aa] = {21, 59};
    TheTable[0x67ad] = {21, 60};
    TheTable[0x67af] = {21, 61};
    TheTable[0x67b6] = {21, 62};
    TheTable[0x67c4] = {21, 63};
    TheTable[0x67cf] = {22, 0};
    TheTable[0x67d0] = {22, 1};
    TheTable[0x67d3] = {22, 2};
    TheTable[0x67d4] = {22, 3};
    TheTable[0x67dc] = {22, 4};
    TheTable[0x67e5] = {22, 5};
    TheTable[0x67f1] = {22, 6};
    TheTable[0x67f3] = {22, 7};
    TheTable[0x67f4] = {22, 8};
    TheTable[0x67ff] = {22, 9};
    TheTable[0x6805] = {22, 10};
    TheTable[0x6807] = {22, 11};
    TheTable[0x680b] = {22, 12};
    TheTable[0x680f] = {22, 13};
    TheTable[0x6811] = {22, 14};
    TheTable[0x6816] = {22, 15};
    TheTable[0x6817] = {22, 16};
    TheTable[0x6821] = {22, 17};
    TheTable[0x682a] = {22, 18};
    TheTable[0x6837] = {22, 19};
    TheTable[0x6838] = {22, 20};
    TheTable[0x6839] = {22, 21};
    TheTable[0x683c] = {22, 22};
    TheTable[0x683d] = {22, 23};
    TheTable[0x6842] = {22, 24};
    TheTable[0x6843] = {22, 25};
    TheTable[0x6846] = {22, 26};
    TheTable[0x6848] = {22, 27};
    TheTable[0x684c] = {22, 28};
    TheTable[0x6850] = {22, 29};
    TheTable[0x6851] = {22, 30};
    TheTable[0x6863] = {22, 31};
    TheTable[0x6865] = {22, 32};
    TheTable[0x6868] = {22, 33};
    TheTable[0x6876] = {22, 34};
    TheTable[0x6881] = {22, 35};
    TheTable[0x6885] = {22, 36};
    TheTable[0x6893] = {22, 37};
    TheTable[0x68a2] = {22, 38};
    TheTable[0x68a6] = {22, 39};
    TheTable[0x68a8] = {22, 40};
    TheTable[0x68af] = {22, 41};
    TheTable[0x68b0] = {22, 42};
    TheTable[0x68b3] = {22, 43};
    TheTable[0x68c0] = {22, 44};
    TheTable[0x68c9] = {22, 45};
    TheTable[0x68cb] = {22, 46};
    TheTable[0x68cd] = {22, 47};
    TheTable[0x68d2] = {22, 48};
    TheTable[0x68d5] = {22, 49};
    TheTable[0x68d8] = {22, 50};
    TheTable[0x68da] = {22, 51};
    TheTable[0x68ee] = {22, 52};
    TheTable[0x68f5] = {22, 53};
    TheTable[0x68fa] = {22, 54};
    TheTable[0x6905] = {22, 55};
    TheTable[0x690d] = {22, 56};
    TheTable[0x6912] = {22, 57};
    TheTable[0x6954] = {22, 58};
    TheTable[0x695a] = {22, 59};
    TheTable[0x697c] = {22, 60};
    TheTable[0x6982] = {22, 61};
    TheTable[0x6984] = {22, 62};
    TheTable[0x6986] = {22, 63};
    TheTable[0x6988] = {23, 0};
    TheTable[0x699c] = {23, 1};
    TheTable[0x69a8] = {23, 2};
    TheTable[0x69b4] = {23, 3};
    TheTable[0x69d0] = {23, 4};
    TheTable[0x69fd] = {23, 5};
    TheTable[0x6a21] = {23, 6};
    TheTable[0x6a2a] = {23, 7};
    TheTable[0x6a31] = {23, 8};
    TheTable[0x6a44] = {23, 9};
    TheTable[0x6a58] = {23, 10};
    TheTable[0x6a59] = {23, 11};
    TheTable[0x6a61] = {23, 12};
    TheTable[0x6a71] = {23, 13};
    TheTable[0x6b20] = {23, 14};
    TheTable[0x6b21] = {23, 15};
    TheTable[0x6b22] = {23, 16};
    TheTable[0x6b23] = {23, 17};
    TheTable[0x6b27] = {23, 18};
    TheTable[0x6b32] = {23, 19};
    TheTable[0x6b3a] = {23, 20};
    TheTable[0x6b3e] = {23, 21};
    TheTable[0x6b47] = {23, 22};
    TheTable[0x6b49] = {23, 23};
    TheTable[0x6b4c] = {23, 24};
    TheTable[0x6b62] = {23, 25};
    TheTable[0x6b63] = {23, 26};
    TheTable[0x6b64] = {23, 27};
    TheTable[0x6b65] = {23, 28};
    TheTable[0x6b66] = {23, 29};
    TheTable[0x6b67] = {23, 30};
    TheTable[0x6b6a] = {23, 31};
    TheTable[0x6b7b] = {23, 32};
    TheTable[0x6b7c] = {23, 33};
    TheTable[0x6b83] = {23, 34};
    TheTable[0x6b84] = {23, 35};
    TheTable[0x6b89] = {23, 36};
    TheTable[0x6b8a] = {23, 37};
    TheTable[0x6b8b] = {23, 38};
    TheTable[0x6b96] = {23, 39};
    TheTable[0x6bb4] = {23, 40};
    TheTable[0x6bb5] = {23, 41};
    TheTable[0x6bbf] = {23, 42};
    TheTable[0x6bc1] = {23, 43};
    TheTable[0x6bc5] = {23, 44};
    TheTable[0x6bcd] = {23, 45};
    TheTable[0x6bcf] = {23, 46};
    TheTable[0x6bd2] = {23, 47};
    TheTable[0x6bd4] = {23, 48};
    TheTable[0x6bd5] = {23, 49};
    TheTable[0x6bd9] = {23, 50};
    TheTable[0x6bdb] = {23, 51};
    TheTable[0x6be1] = {23, 52};
    TheTable[0x6beb] = {23, 53};
    TheTable[0x6bef] = {23, 54};
    TheTable[0x6c0f] = {23, 55};
    TheTable[0x6c11] = {23, 56};
    TheTable[0x6c13] = {23, 57};
    TheTable[0x6c14] = {23, 58};
    TheTable[0x6c1b] = {23, 59};
    TheTable[0x6c27] = {23, 60};
    TheTable[0x6c2e] = {23, 61};
    TheTable[0x6c34] = {23, 62};
    TheTable[0x6c38] = {23, 63};
    TheTable[0x6c41] = {24, 0};
    TheTable[0x6c42] = {24, 1};
    TheTable[0x6c47] = {24, 2};
    TheTable[0x6c49] = {24, 3};
    TheTable[0x6c57] = {24, 4};
    TheTable[0x6c5e] = {24, 5};
    TheTable[0x6c5f] = {24, 6};
    TheTable[0x6c60] = {24, 7};
    TheTable[0x6c61] = {24, 8};
    TheTable[0x6c64] = {24, 9};
    TheTable[0x6c6a] = {24, 10};
    TheTable[0x6c7d] = {24, 11};
    TheTable[0x6c83] = {24, 12};
    TheTable[0x6c88] = {24, 13};
    TheTable[0x6c89] = {24, 14};
    TheTable[0x6c99] = {24, 15};
    TheTable[0x6c9f] = {24, 16};
    TheTable[0x6ca1] = {24, 17};
    TheTable[0x6ca6] = {24, 18};
    TheTable[0x6ca7] = {24, 19};
    TheTable[0x6cab] = {24, 20};
    TheTable[0x6cae] = {24, 21};
    TheTable[0x6cb3] = {24, 22};
    TheTable[0x6cb8] = {24, 23};
    TheTable[0x6cb9] = {24, 24};
    TheTable[0x6cbb] = {24, 25};
    TheTable[0x6cbe] = {24, 26};
    TheTable[0x6cbf] = {24, 27};
    TheTable[0x6cc4] = {24, 28};
    TheTable[0x6cc9] = {24, 29};
    TheTable[0x6cca] = {24, 30};
    TheTable[0x6cd5] = {24, 31};
    TheTable[0x6cdb] = {24, 32};
    TheTable[0x6ce1] = {24, 33};
    TheTable[0x6ce2] = {24, 34};
    TheTable[0x6ce3] = {24, 35};
    TheTable[0x6ce5] = {24, 36};
    TheTable[0x6ce8] = {24, 37};
    TheTable[0x6cea] = {24, 38};
    TheTable[0x6cf0] = {24, 39};
    TheTable[0x6cf3] = {24, 40};
    TheTable[0x6cfb] = {24, 41};
    TheTable[0x6cfc] = {24, 42};
    TheTable[0x6cfd] = {24, 43};
    TheTable[0x6d01] = {24, 44};
    TheTable[0x6d0b] = {24, 45};
    TheTable[0x6d12] = {24, 46};
    TheTable[0x6d17] = {24, 47};
    TheTable[0x6d1b] = {24, 48};
    TheTable[0x6d1e] = {24, 49};
    TheTable[0x6d25] = {24, 50};
    TheTable[0x6d2a] = {24, 51};
    TheTable[0x6d32] = {24, 52};
    TheTable[0x6d3b] = {24, 53};
    TheTable[0x6d3d] = {24, 54};
    TheTable[0x6d3e] = {24, 55};
    TheTable[0x6d41] = {24, 56};
    TheTable[0x6d45] = {24, 57};
    TheTable[0x6d46] = {24, 58};
    TheTable[0x6d47] = {24, 59};
    TheTable[0x6d4a] = {24, 60};
    TheTable[0x6d4b] = {24, 61};
    TheTable[0x6d4e] = {24, 62};
    TheTable[0x6d51] = {24, 63};
    TheTable[0x6d53] = {25, 0};
    TheTable[0x6d59] = {25, 1};
    TheTable[0x6d69] = {25, 2};
    TheTable[0x6d6a] = {25, 3};
    TheTable[0x6d6e] = {25, 4};
    TheTable[0x6d74] = {25, 5};
    TheTable[0x6d77] = {25, 6};
    TheTable[0x6d78] = {25, 7};
    TheTable[0x6d82] = {25, 8};
    TheTable[0x6d88] = {25, 9};
    TheTable[0x6d89] = {25, 10};
    TheTable[0x6d8c] = {25, 11};
    TheTable[0x6d95] = {25, 12};
    TheTable[0x6d9b] = {25, 13};
    TheTable[0x6d9d] = {25, 14};
    TheTable[0x6da6] = {25, 15};
    TheTable[0x6da8] = {25, 16};
    TheTable[0x6db2] = {25, 17};
    TheTable[0x6dc7] = {25, 18};
    TheTable[0x6dcb] = {25, 19};
    TheTable[0x6dd8] = {25, 20};
    TheTable[0x6de1] = {25, 21};
    TheTable[0x6deb] = {25, 22};
    TheTable[0x6df1] = {25, 23};
    TheTable[0x6df7] = {25, 24};
    TheTable[0x6df9] = {25, 25};
    TheTable[0x6dfb] = {25, 26};
    TheTable[0x6e05] = {25, 27};
    TheTable[0x6e10] = {25, 28};
    TheTable[0x6e14] = {25, 29};
    TheTable[0x6e17] = {25, 30};
    TheTable[0x6e20] = {25, 31};
    TheTable[0x6e21] = {25, 32};
    TheTable[0x6e23] = {25, 33};
    TheTable[0x6e29] = {25, 34};
    TheTable[0x6e2f] = {25, 35};
    TheTable[0x6e34] = {25, 36};
    TheTable[0x6e38] = {25, 37};
    TheTable[0x6e56] = {25, 38};
    TheTable[0x6e7e] = {25, 39};
    TheTable[0x6e7f] = {25, 40};
    TheTable[0x6e83] = {25, 41};
    TheTable[0x6e85] = {25, 42};
    TheTable[0x6e89] = {25, 43};
    TheTable[0x6e90] = {25, 44};
    TheTable[0x6e9c] = {25, 45};
    TheTable[0x6eaa] = {25, 46};
    TheTable[0x6ecb] = {25, 47};
    TheTable[0x6ed1] = {25, 48};
    TheTable[0x6ed4] = {25, 49};
    TheTable[0x6eda] = {25, 50};
    TheTable[0x6ee1] = {25, 51};
    TheTable[0x6ee4] = {25, 52};
    TheTable[0x6ee5] = {25, 53};
    TheTable[0x6ee8] = {25, 54};
    TheTable[0x6ee9] = {25, 55};
    TheTable[0x6ef4] = {25, 56};
    TheTable[0x6f02] = {25, 57};
    TheTable[0x6f06] = {25, 58};
    TheTable[0x6f0f] = {25, 59};
    TheTable[0x6f14] = {25, 60};
    TheTable[0x6f20] = {25, 61};
    TheTable[0x6f2b] = {25, 62};
    TheTable[0x6f58] = {25, 63};
    TheTable[0x6f5c] = {26, 0};
    TheTable[0x6f6e] = {26, 1};
    TheTable[0x6fa1] = {26, 2};
    TheTable[0x6fc0] = {26, 3};
    TheTable[0x704c] = {26, 4};
    TheTable[0x706b] = {26, 5};
    TheTable[0x706d] = {26, 6};
    TheTable[0x706f] = {26, 7};
    TheTable[0x7070] = {26, 8};
    TheTable[0x7075] = {26, 9};
    TheTable[0x7076] = {26, 10};
    TheTable[0x707e] = {26, 11};
    TheTable[0x707f] = {26, 12};
    TheTable[0x7089] = {26, 13};
    TheTable[0x708a] = {26, 14};
    TheTable[0x708e] = {26, 15};
    TheTable[0x7092] = {26, 16};
    TheTable[0x7095] = {26, 17};
    TheTable[0x70ab] = {26, 18};
    TheTable[0x70ad] = {26, 19};
    TheTable[0x70ae] = {26, 20};
    TheTable[0x70b8] = {26, 21};
    TheTable[0x70b9] = {26, 22};
    TheTable[0x70bc] = {26, 23};
    TheTable[0x70c1] = {26, 24};
    TheTable[0x70c2] = {26, 25};
    TheTable[0x70c8] = {26, 26};
    TheTable[0x70ca] = {26, 27};
    TheTable[0x70d8] = {26, 28};
    TheTable[0x70db] = {26, 29};
    TheTable[0x70df] = {26, 30};
    TheTable[0x70e4] = {26, 31};
    TheTable[0x70e6] = {26, 32};
    TheTable[0x70e7] = {26, 33};
    TheTable[0x70eb] = {26, 34};
    TheTable[0x70ec] = {26, 35};
    TheTable[0x70ed] = {26, 36};
    TheTable[0x70f7] = {26, 37};
    TheTable[0x711a] = {26, 38};
    TheTable[0x7126] = {26, 39};
    TheTable[0x7130] = {26, 40};
    TheTable[0x7136] = {26, 41};
    TheTable[0x714c] = {26, 42};
    TheTable[0x714e] = {26, 43};
    TheTable[0x7164] = {26, 44};
    TheTable[0x7167] = {26, 45};
    TheTable[0x716e] = {26, 46};
    TheTable[0x7172] = {26, 47};
    TheTable[0x7184] = {26, 48};
    TheTable[0x718a] = {26, 49};
    TheTable[0x718f] = {26, 50};
    TheTable[0x7194] = {26, 51};
    TheTable[0x719f] = {26, 52};
    TheTable[0x71ac] = {26, 53};
    TheTable[0x71c3] = {26, 54};
    TheTable[0x71d5] = {26, 55};
    TheTable[0x71e5] = {26, 56};
    TheTable[0x7206] = {26, 57};
    TheTable[0x722a] = {26, 58};
    TheTable[0x722c] = {26, 59};
    TheTable[0x7231] = {26, 60};
    TheTable[0x7235] = {26, 61};
    TheTable[0x7236] = {26, 62};
    TheTable[0x7237] = {26, 63};
    TheTable[0x7238] = {27, 0};
    TheTable[0x7239] = {27, 1};
    TheTable[0x723d] = {27, 2};
    TheTable[0x7247] = {27, 3};
    TheTable[0x7248] = {27, 4};
    TheTable[0x724c] = {27, 5};
    TheTable[0x7259] = {27, 6};
    TheTable[0x725b] = {27, 7};
    TheTable[0x7261] = {27, 8};
    TheTable[0x7262] = {27, 9};
    TheTable[0x7267] = {27, 10};
    TheTable[0x7269] = {27, 11};
    TheTable[0x7272] = {27, 12};
    TheTable[0x7275] = {27, 13};
    TheTable[0x7279] = {27, 14};
    TheTable[0x727a] = {27, 15};
    TheTable[0x7280] = {27, 16};
    TheTable[0x7281] = {27, 17};
    TheTable[0x728a] = {27, 18};
    TheTable[0x72ac] = {27, 19};
    TheTable[0x72af] = {27, 20};
    TheTable[0x72b6] = {27, 21};
    TheTable[0x72b9] = {27, 22};
    TheTable[0x72c2] = {27, 23};
    TheTable[0x72d0] = {27, 24};
    TheTable[0x72d7] = {27, 25};
    TheTable[0x72d9] = {27, 26};
    TheTable[0x72e0] = {27, 27};
    TheTable[0x72e1] = {27, 28};
    TheTable[0x72ec] = {27, 29};
    TheTable[0x72ed] = {27, 30};
    TheTable[0x72ee] = {27, 31};
    TheTable[0x72f1] = {27, 32};
    TheTable[0x72f8] = {27, 33};
    TheTable[0x72fc] = {27, 34};
    TheTable[0x730e] = {27, 35};
    TheTable[0x731b] = {27, 36};
    TheTable[0x731c] = {27, 37};
    TheTable[0x7329] = {27, 38};
    TheTable[0x732a] = {27, 39};
    TheTable[0x732b] = {27, 40};
    TheTable[0x732e] = {27, 41};
    TheTable[0x7334] = {27, 42};
    TheTable[0x733e] = {27, 43};
    TheTable[0x7387] = {27, 44};
    TheTable[0x7389] = {27, 45};
    TheTable[0x738b] = {27, 46};
    TheTable[0x739b] = {27, 47};
    TheTable[0x73a9] = {27, 48};
    TheTable[0x73af] = {27, 49};
    TheTable[0x73b0] = {27, 50};
    TheTable[0x73bb] = {27, 51};
    TheTable[0x73c0] = {27, 52};
    TheTable[0x73cd] = {27, 53};
    TheTable[0x73e0] = {27, 54};
    TheTable[0x73ed] = {27, 55};
    TheTable[0x7403] = {27, 56};
    TheTable[0x7406] = {27, 57};
    TheTable[0x7433] = {27, 58};
    TheTable[0x7434] = {27, 59};
    TheTable[0x745c] = {27, 60};
    TheTable[0x745e] = {27, 61};
    TheTable[0x745f] = {27, 62};
    TheTable[0x7483] = {27, 63};
    TheTable[0x74dc] = {28, 0};
    TheTable[0x74e3] = {28, 1};
    TheTable[0x74e6] = {28, 2};
    TheTable[0x74ee] = {28, 3};
    TheTable[0x74f6] = {28, 4};
    TheTable[0x7518] = {28, 5};
    TheTable[0x751a] = {28, 6};
    TheTable[0x751c] = {28, 7};
    TheTable[0x751f] = {28, 8};
    TheTable[0x7528] = {28, 9};
    TheTable[0x7529] = {28, 10};
    TheTable[0x752d] = {28, 11};
    TheTable[0x7530] = {28, 12};
    TheTable[0x7531] = {28, 13};
    TheTable[0x7532] = {28, 14};
    TheTable[0x7533] = {28, 15};
    TheTable[0x7535] = {28, 16};
    TheTable[0x7537] = {28, 17};
    TheTable[0x753b] = {28, 18};
    TheTable[0x7545] = {28, 19};
    TheTable[0x754c] = {28, 20};
    TheTable[0x754f] = {28, 21};
    TheTable[0x7559] = {28, 22};
    TheTable[0x755c] = {28, 23};
    TheTable[0x7565] = {28, 24};
    TheTable[0x756a] = {28, 25};
    TheTable[0x7586] = {28, 26};
    TheTable[0x758f] = {28, 27};
    TheTable[0x7591] = {28, 28};
    TheTable[0x7597] = {28, 29};
    TheTable[0x7599] = {28, 30};
    TheTable[0x759a] = {28, 31};
    TheTable[0x75a1] = {28, 32};
    TheTable[0x75a4] = {28, 33};
    TheTable[0x75ab] = {28, 34};
    TheTable[0x75ae] = {28, 35};
    TheTable[0x75af] = {28, 36};
    TheTable[0x75b2] = {28, 37};
    TheTable[0x75bc] = {28, 38};
    TheTable[0x75be] = {28, 39};
    TheTable[0x75c5] = {28, 40};
    TheTable[0x75c7] = {28, 41};
    TheTable[0x75ca] = {28, 42};
    TheTable[0x75d2] = {28, 43};
    TheTable[0x75d5] = {28, 44};
    TheTable[0x75db] = {28, 45};
    TheTable[0x75de] = {28, 46};
    TheTable[0x75e2] = {28, 47};
    TheTable[0x75f0] = {28, 48};
    TheTable[0x75f4] = {28, 49};
    TheTable[0x7626] = {28, 50};
    TheTable[0x7629] = {28, 51};
    TheTable[0x7634] = {28, 52};
    TheTable[0x763e] = {28, 53};
    TheTable[0x767b] = {28, 54};
    TheTable[0x767d] = {28, 55};
    TheTable[0x767e] = {28, 56};
    TheTable[0x7682] = {28, 57};
    TheTable[0x7684] = {28, 58};
    TheTable[0x7686] = {28, 59};
    TheTable[0x7687] = {28, 60};
    TheTable[0x76ae] = {28, 61};
    TheTable[0x76b1] = {28, 62};
    TheTable[0x76c6] = {28, 63};
    TheTable[0x76c8] = {29, 0};
    TheTable[0x76ca] = {29, 1};
    TheTable[0x76ce] = {29, 2};
    TheTable[0x76cf] = {29, 3};
    TheTable[0x76d0] = {29, 4};
    TheTable[0x76d1] = {29, 5};
    TheTable[0x76d2] = {29, 6};
    TheTable[0x76d4] = {29, 7};
    TheTable[0x76d6] = {29, 8};
    TheTable[0x76d7] = {29, 9};
    TheTable[0x76d8] = {29, 10};
    TheTable[0x76db] = {29, 11};
    TheTable[0x76df] = {29, 12};
    TheTable[0x76ee] = {29, 13};
    TheTable[0x76ef] = {29, 14};
    TheTable[0x76f2] = {29, 15};
    TheTable[0x76f4] = {29, 16};
    TheTable[0x76f8] = {29, 17};
    TheTable[0x76fc] = {29, 18};
    TheTable[0x76fe] = {29, 19};
    TheTable[0x7701] = {29, 20};
    TheTable[0x7708] = {29, 21};
    TheTable[0x7709] = {29, 22};
    TheTable[0x770b] = {29, 23};
    TheTable[0x771f] = {29, 24};
    TheTable[0x7720] = {29, 25};
    TheTable[0x7728] = {29, 26};
    TheTable[0x772f] = {29, 27};
    TheTable[0x773c] = {29, 28};
    TheTable[0x7740] = {29, 29};
    TheTable[0x7741] = {29, 30};
    TheTable[0x775b] = {29, 31};
    TheTable[0x7761] = {29, 32};
    TheTable[0x7763] = {29, 33};
    TheTable[0x776c] = {29, 34};
    TheTable[0x7779] = {29, 35};
    TheTable[0x777e] = {29, 36};
    TheTable[0x7784] = {29, 37};
    TheTable[0x778c] = {29, 38};
    TheTable[0x778e] = {29, 39};
    TheTable[0x7792] = {29, 40};
    TheTable[0x77a7] = {29, 41};
    TheTable[0x77ad] = {29, 42};
    TheTable[0x77db] = {29, 43};
    TheTable[0x77e3] = {29, 44};
    TheTable[0x77e5] = {29, 45};
    TheTable[0x77e9] = {29, 46};
    TheTable[0x77ed] = {29, 47};
    TheTable[0x77ee] = {29, 48};
    TheTable[0x77f3] = {29, 49};
    TheTable[0x77ff] = {29, 50};
    TheTable[0x7801] = {29, 51};
    TheTable[0x780c] = {29, 52};
    TheTable[0x780d] = {29, 53};
    TheTable[0x7814] = {29, 54};
    TheTable[0x7816] = {29, 55};
    TheTable[0x7834] = {29, 56};
    TheTable[0x7838] = {29, 57};
    TheTable[0x7840] = {29, 58};
    TheTable[0x786c] = {29, 59};
    TheTable[0x786e] = {29, 60};
    TheTable[0x788c] = {29, 61};
    TheTable[0x788d] = {29, 62};
    TheTable[0x788e] = {29, 63};
    TheTable[0x7891] = {30, 0};
    TheTable[0x7897] = {30, 1};
    TheTable[0x789f] = {30, 2};
    TheTable[0x78a7] = {30, 3};
    TheTable[0x78b0] = {30, 4};
    TheTable[0x78be] = {30, 5};
    TheTable[0x78c1] = {30, 6};
    TheTable[0x78c5] = {30, 7};
    TheTable[0x78cb] = {30, 8};
    TheTable[0x78d5] = {30, 9};
    TheTable[0x78e8] = {30, 10};
    TheTable[0x7901] = {30, 11};
    TheTable[0x793a] = {30, 12};
    TheTable[0x793c] = {30, 13};
    TheTable[0x793e] = {30, 14};
    TheTable[0x7948] = {30, 15};
    TheTable[0x7956] = {30, 16};
    TheTable[0x795d] = {30, 17};
    TheTable[0x795e] = {30, 18};
    TheTable[0x7965] = {30, 19};
    TheTable[0x7968] = {30, 20};
    TheTable[0x7977] = {30, 21};
    TheTable[0x7978] = {30, 22};
    TheTable[0x7981] = {30, 23};
    TheTable[0x798f] = {30, 24};
    TheTable[0x79bb] = {30, 25};
    TheTable[0x79bd] = {30, 26};
    TheTable[0x79be] = {30, 27};
    TheTable[0x79c0] = {30, 28};
    TheTable[0x79c1] = {30, 29};
    TheTable[0x79c3] = {30, 30};
    TheTable[0x79c6] = {30, 31};
    TheTable[0x79cb] = {30, 32};
    TheTable[0x79cd] = {30, 33};
    TheTable[0x79d1] = {30, 34};
    TheTable[0x79d2] = {30, 35};
    TheTable[0x79d8] = {30, 36};
    TheTable[0x79df] = {30, 37};
    TheTable[0x79e4] = {30, 38};
    TheTable[0x79e7] = {30, 39};
    TheTable[0x79e9] = {30, 40};
    TheTable[0x79ef] = {30, 41};
    TheTable[0x79f0] = {30, 42};
    TheTable[0x79fb] = {30, 43};
    TheTable[0x7a00] = {30, 44};
    TheTable[0x7a0b] = {30, 45};
    TheTable[0x7a0d] = {30, 46};
    TheTable[0x7a0e] = {30, 47};
    TheTable[0x7a1a] = {30, 48};
    TheTable[0x7a20] = {30, 49};
    TheTable[0x7a23] = {30, 50};
    TheTable[0x7a33] = {30, 51};
    TheTable[0x7a3b] = {30, 52};
    TheTable[0x7a3c] = {30, 53};
    TheTable[0x7a3f] = {30, 54};
    TheTable[0x7a46] = {30, 55};
    TheTable[0x7a57] = {30, 56};
    TheTable[0x7a74] = {30, 57};
    TheTable[0x7a76] = {30, 58};
    TheTable[0x7a77] = {30, 59};
    TheTable[0x7a7a] = {30, 60};
    TheTable[0x7a7f] = {30, 61};
    TheTable[0x7a81] = {30, 62};
    TheTable[0x7a83] = {30, 63};
    TheTable[0x7a84] = {31, 0};
    TheTable[0x7a91] = {31, 1};
    TheTable[0x7a92] = {31, 2};
    TheTable[0x7a97] = {31, 3};
    TheTable[0x7a9c] = {31, 4};
    TheTable[0x7a9d] = {31, 5};
    TheTable[0x7a9f] = {31, 6};
    TheTable[0x7aa5] = {31, 7};
    TheTable[0x7acb] = {31, 8};
    TheTable[0x7ad6] = {31, 9};
    TheTable[0x7ad9] = {31, 10};
    TheTable[0x7ade] = {31, 11};
    TheTable[0x7adf] = {31, 12};
    TheTable[0x7ae0] = {31, 13};
    TheTable[0x7ae5] = {31, 14};
    TheTable[0x7aed] = {31, 15};
    TheTable[0x7aef] = {31, 16};
    TheTable[0x7af9] = {31, 17};
    TheTable[0x7aff] = {31, 18};
    TheTable[0x7b0b] = {31, 19};
    TheTable[0x7b11] = {31, 20};
    TheTable[0x7b14] = {31, 21};
    TheTable[0x7b1b] = {31, 22};
    TheTable[0x7b26] = {31, 23};
    TheTable[0x7b28] = {31, 24};
    TheTable[0x7b2c] = {31, 25};
    TheTable[0x7b3c] = {31, 26};
    TheTable[0x7b49] = {31, 27};
    TheTable[0x7b4b] = {31, 28};
    TheTable[0x7b50] = {31, 29};
    TheTable[0x7b51] = {31, 30};
    TheTable[0x7b52] = {31, 31};
    TheTable[0x7b54] = {31, 32};
    TheTable[0x7b56] = {31, 33};
    TheTable[0x7b5b] = {31, 34};
    TheTable[0x7b5d] = {31, 35};
    TheTable[0x7b79] = {31, 36};
    TheTable[0x7b7e] = {31, 37};
    TheTable[0x7b80] = {31, 38};
    TheTable[0x7b97] = {31, 39};
    TheTable[0x7ba1] = {31, 40};
    TheTable[0x7ba9] = {31, 41};
    TheTable[0x7bab] = {31, 42};
    TheTable[0x7bad] = {31, 43};
    TheTable[0x7bb1] = {31, 44};
    TheTable[0x7bc7] = {31, 45};
    TheTable[0x7bdd] = {31, 46};
    TheTable[0x7bee] = {31, 47};
    TheTable[0x7bf7] = {31, 48};
    TheTable[0x7c27] = {31, 49};
    TheTable[0x7c38] = {31, 50};
    TheTable[0x7c4d] = {31, 51};
    TheTable[0x7c73] = {31, 52};
    TheTable[0x7c7b] = {31, 53};
    TheTable[0x7c89] = {31, 54};
    TheTable[0x7c92] = {31, 55};
    TheTable[0x7c97] = {31, 56};
    TheTable[0x7c98] = {31, 57};
    TheTable[0x7c9f] = {31, 58};
    TheTable[0x7ca5] = {31, 59};
    TheTable[0x7caa] = {31, 60};
    TheTable[0x7cae] = {31, 61};
    TheTable[0x7cb1] = {31, 62};
    TheTable[0x7cbe] = {31, 63};
    TheTable[0x7cca] = {32, 0};
    TheTable[0x7cd5] = {32, 1};
    TheTable[0x7cd6] = {32, 2};
    TheTable[0x7cdf] = {32, 3};
    TheTable[0x7ce0] = {32, 4};
    TheTable[0x7cfb] = {32, 5};
    TheTable[0x7d20] = {32, 6};
    TheTable[0x7d22] = {32, 7};
    TheTable[0x7d27] = {32, 8};
    TheTable[0x7d2b] = {32, 9};
    TheTable[0x7d2f] = {32, 10};
    TheTable[0x7d6e] = {32, 11};
    TheTable[0x7e41] = {32, 12};
    TheTable[0x7ea0] = {32, 13};
    TheTable[0x7ea2] = {32, 14};
    TheTable[0x7ea4] = {32, 15};
    TheTable[0x7ea6] = {32, 16};
    TheTable[0x7ea7] = {32, 17};
    TheTable[0x7eaa] = {32, 18};
    TheTable[0x7eaf] = {32, 19};
    TheTable[0x7eb1] = {32, 20};
    TheTable[0x7eb2] = {32, 21};
    TheTable[0x7eb3] = {32, 22};
    TheTable[0x7eb5] = {32, 23};
    TheTable[0x7eb7] = {32, 24};
    TheTable[0x7eb8] = {32, 25};
    TheTable[0x7eb9] = {32, 26};
    TheTable[0x7eba] = {32, 27};
    TheTable[0x7ebd] = {32, 28};
    TheTable[0x7ebf] = {32, 29};
    TheTable[0x7ec3] = {32, 30};
    TheTable[0x7ec4] = {32, 31};
    TheTable[0x7ec5] = {32, 32};
    TheTable[0x7ec6] = {32, 33};
    TheTable[0x7ec7] = {32, 34};
    TheTable[0x7ec8] = {32, 35};
    TheTable[0x7eca] = {32, 36};
    TheTable[0x7ecd] = {32, 37};
    TheTable[0x7ecf] = {32, 38};
    TheTable[0x7ed1] = {32, 39};
    TheTable[0x7ed2] = {32, 40};
    TheTable[0x7ed3] = {32, 41};
    TheTable[0x7ed5] = {32, 42};
    TheTable[0x7ed8] = {32, 43};
    TheTable[0x7ed9] = {32, 44};
    TheTable[0x7edc] = {32, 45};
    TheTable[0x7edd] = {32, 46};
    TheTable[0x7ede] = {32, 47};
    TheTable[0x7edf] = {32, 48};
    TheTable[0x7ee2] = {32, 49};
    TheTable[0x7ee3] = {32, 50};
    TheTable[0x7ee7] = {32, 51};
    TheTable[0x7ee9] = {32, 52};
    TheTable[0x7eea] = {32, 53};
    TheTable[0x7eed] = {32, 54};
    TheTable[0x7ef3] = {32, 55};
    TheTable[0x7ef4] = {32, 56};
    TheTable[0x7ef5] = {32, 57};
    TheTable[0x7ef8] = {32, 58};
    TheTable[0x7efc] = {32, 59};
    TheTable[0x7eff] = {32, 60};
    TheTable[0x7f00] = {32, 61};
    TheTable[0x7f05] = {32, 62};
    TheTable[0x7f09] = {32, 63};
    TheTable[0x7f0e] = {33, 0};
    TheTable[0x7f13] = {33, 1};
    TheTable[0x7f14] = {33, 2};
    TheTable[0x7f15] = {33, 3};
    TheTable[0x7f16] = {33, 4};
    TheTable[0x7f18] = {33, 5};
    TheTable[0x7f1a] = {33, 6};
    TheTable[0x7f1d] = {33, 7};
    TheTable[0x7f20] = {33, 8};
    TheTable[0x7f29] = {33, 9};
    TheTable[0x7f34] = {33, 10};
    TheTable[0x7f38] = {33, 11};
    TheTable[0x7f3a] = {33, 12};
    TheTable[0x7f44] = {33, 13};
    TheTable[0x7f50] = {33, 14};
    TheTable[0x7f51] = {33, 15};
    TheTable[0x7f57] = {33, 16};
    TheTable[0x7f5a] = {33, 17};
    TheTable[0x7f62] = {33, 18};
    TheTable[0x7f69] = {33, 19};
    TheTable[0x7f6a] = {33, 20};
    TheTable[0x7f6e] = {33, 21};
    TheTable[0x7f8a] = {33, 22};
    TheTable[0x7f8e] = {33, 23};
    TheTable[0x7f9e] = {33, 24};
    TheTable[0x7fa1] = {33, 25};
    TheTable[0x7fa4] = {33, 26};
    TheTable[0x7fb9] = {33, 27};
    TheTable[0x7fbd] = {33, 28};
    TheTable[0x7fc1] = {33, 29};
    TheTable[0x7fc5] = {33, 30};
    TheTable[0x7fd4] = {33, 31};
    TheTable[0x7fd8] = {33, 32};
    TheTable[0x7fe0] = {33, 33};
    TheTable[0x7ff0] = {33, 34};
    TheTable[0x7ffb] = {33, 35};
    TheTable[0x7ffc] = {33, 36};
    TheTable[0x8000] = {33, 37};
    TheTable[0x8001] = {33, 38};
    TheTable[0x8003] = {33, 39};
    TheTable[0x8005] = {33, 40};
    TheTable[0x800c] = {33, 41};
    TheTable[0x800d] = {33, 42};
    TheTable[0x8010] = {33, 43};
    TheTable[0x8015] = {33, 44};
    TheTable[0x8017] = {33, 45};
    TheTable[0x8033] = {33, 46};
    TheTable[0x8036] = {33, 47};
    TheTable[0x803b] = {33, 48};
    TheTable[0x803d] = {33, 49};
    TheTable[0x804a] = {33, 50};
    TheTable[0x804b] = {33, 51};
    TheTable[0x804c] = {33, 52};
    TheTable[0x8054] = {33, 53};
    TheTable[0x805a] = {33, 54};
    TheTable[0x806a] = {33, 55};
    TheTable[0x8083] = {33, 56};
    TheTable[0x8086] = {33, 57};
    TheTable[0x8089] = {33, 58};
    TheTable[0x808b] = {33, 59};
    TheTable[0x808c] = {33, 60};
    TheTable[0x809a] = {33, 61};
    TheTable[0x809d] = {33, 62};
    TheTable[0x80a0] = {33, 63};
    TheTable[0x80a1] = {34, 0};
    TheTable[0x80a2] = {34, 1};
    TheTable[0x80a4] = {34, 2};
    TheTable[0x80a5] = {34, 3};
    TheTable[0x80a9] = {34, 4};
    TheTable[0x80aa] = {34, 5};
    TheTable[0x80ae] = {34, 6};
    TheTable[0x80af] = {34, 7};
    TheTable[0x80b2] = {34, 8};
    TheTable[0x80ba] = {34, 9};
    TheTable[0x80be] = {34, 10};
    TheTable[0x80bf] = {34, 11};
    TheTable[0x80c0] = {34, 12};
    TheTable[0x80c1] = {34, 13};
    TheTable[0x80c3] = {34, 14};
    TheTable[0x80c6] = {34, 15};
    TheTable[0x80cc] = {34, 16};
    TheTable[0x80ce] = {34, 17};
    TheTable[0x80d6] = {34, 18};
    TheTable[0x80dc] = {34, 19};
    TheTable[0x80de] = {34, 20};
    TheTable[0x80e1] = {34, 21};
    TheTable[0x80f3] = {34, 22};
    TheTable[0x80f6] = {34, 23};
    TheTable[0x80f8] = {34, 24};
    TheTable[0x80fd] = {34, 25};
    TheTable[0x8102] = {34, 26};
    TheTable[0x8106] = {34, 27};
    TheTable[0x8109] = {34, 28};
    TheTable[0x810a] = {34, 29};
    TheTable[0x810f] = {34, 30};
    TheTable[0x8111] = {34, 31};
    TheTable[0x8116] = {34, 32};
    TheTable[0x811a] = {34, 33};
    TheTable[0x8131] = {34, 34};
    TheTable[0x8138] = {34, 35};
    TheTable[0x813e] = {34, 36};
    TheTable[0x814a] = {34, 37};
    TheTable[0x814c] = {34, 38};
    TheTable[0x8150] = {34, 39};
    TheTable[0x8154] = {34, 40};
    TheTable[0x8165] = {34, 41};
    TheTable[0x8170] = {34, 42};
    TheTable[0x8179] = {34, 43};
    TheTable[0x817e] = {34, 44};
    TheTable[0x817f] = {34, 45};
    TheTable[0x8180] = {34, 46};
    TheTable[0x818a] = {34, 47};
    TheTable[0x818f] = {34, 48};
    TheTable[0x819b] = {34, 49};
    TheTable[0x819c] = {34, 50};
    TheTable[0x819d] = {34, 51};
    TheTable[0x81a8] = {34, 52};
    TheTable[0x81c2] = {34, 53};
    TheTable[0x81e3] = {34, 54};
    TheTable[0x81ea] = {34, 55};
    TheTable[0x81ed] = {34, 56};
    TheTable[0x81f3] = {34, 57};
    TheTable[0x81f4] = {34, 58};
    TheTable[0x8205] = {34, 59};
    TheTable[0x820c] = {34, 60};
    TheTable[0x820d] = {34, 61};
    TheTable[0x8212] = {34, 62};
    TheTable[0x8214] = {34, 63};
    TheTable[0x821e] = {35, 0};
    TheTable[0x821f] = {35, 1};
    TheTable[0x822a] = {35, 2};
    TheTable[0x822c] = {35, 3};
    TheTable[0x8230] = {35, 4};
    TheTable[0x8231] = {35, 5};
    TheTable[0x8235] = {35, 6};
    TheTable[0x8236] = {35, 7};
    TheTable[0x8239] = {35, 8};
    TheTable[0x8247] = {35, 9};
    TheTable[0x8258] = {35, 10};
    TheTable[0x826f] = {35, 11};
    TheTable[0x8270] = {35, 12};
    TheTable[0x8272] = {35, 13};
    TheTable[0x8273] = {35, 14};
    TheTable[0x827a] = {35, 15};
    TheTable[0x827e] = {35, 16};
    TheTable[0x8282] = {35, 17};
    TheTable[0x8292] = {35, 18};
    TheTable[0x829c] = {35, 19};
    TheTable[0x829d] = {35, 20};
    TheTable[0x82a6] = {35, 21};
    TheTable[0x82ac] = {35, 22};
    TheTable[0x82ad] = {35, 23};
    TheTable[0x82b1] = {35, 24};
    TheTable[0x82b3] = {35, 25};
    TheTable[0x82b9] = {35, 26};
    TheTable[0x82bd] = {35, 27};
    TheTable[0x82cd] = {35, 28};
    TheTable[0x82cf] = {35, 29};
    TheTable[0x82d7] = {35, 30};
    TheTable[0x82db] = {35, 31};
    TheTable[0x82df] = {35, 32};
    TheTable[0x82e5] = {35, 33};
    TheTable[0x82e6] = {35, 34};
    TheTable[0x82f1] = {35, 35};
    TheTable[0x82f9] = {35, 36};
    TheTable[0x8302] = {35, 37};
    TheTable[0x8303] = {35, 38};
    TheTable[0x8304] = {35, 39};
    TheTable[0x8305] = {35, 40};
    TheTable[0x830e] = {35, 41};
    TheTable[0x8327] = {35, 42};
    TheTable[0x8328] = {35, 43};
    TheTable[0x832b] = {35, 44};
    TheTable[0x832c] = {35, 45};
    TheTable[0x8336] = {35, 46};
    TheTable[0x8349] = {35, 47};
    TheTable[0x8350] = {35, 48};
    TheTable[0x8352] = {35, 49};
    TheTable[0x8361] = {35, 50};
    TheTable[0x8363] = {35, 51};
    TheTable[0x836b] = {35, 52};
    TheTable[0x836f] = {35, 53};
    TheTable[0x8377] = {35, 54};
    TheTable[0x8389] = {35, 55};
    TheTable[0x838e] = {35, 56};
    TheTable[0x8393] = {35, 57};
    TheTable[0x83ab] = {35, 58};
    TheTable[0x83b1] = {35, 59};
    TheTable[0x83b2] = {35, 60};
    TheTable[0x83b7] = {35, 61};
    TheTable[0x83c7] = {35, 62};
    TheTable[0x83ca] = {35, 63};
    TheTable[0x83cc] = {36, 0};
    TheTable[0x83dc] = {36, 1};
    TheTable[0x83e0] = {36, 2};
    TheTable[0x83f2] = {36, 3};
    TheTable[0x8404] = {36, 4};
    TheTable[0x840c] = {36, 5};
    TheTable[0x840d] = {36, 6};
    TheTable[0x840e] = {36, 7};
    TheTable[0x841d] = {36, 8};
    TheTable[0x8425] = {36, 9};
    TheTable[0x8428] = {36, 10};
    TheTable[0x843d] = {36, 11};
    TheTable[0x8457] = {36, 12};
    TheTable[0x845b] = {36, 13};
    TheTable[0x8461] = {36, 14};
    TheTable[0x8463] = {36, 15};
    TheTable[0x8469] = {36, 16};
    TheTable[0x846c] = {36, 17};
    TheTable[0x8471] = {36, 18};
    TheTable[0x8475] = {36, 19};
    TheTable[0x8482] = {36, 20};
    TheTable[0x8499] = {36, 21};
    TheTable[0x849c] = {36, 22};
    TheTable[0x84b8] = {36, 23};
    TheTable[0x84c4] = {36, 24};
    TheTable[0x84dd] = {36, 25};
    TheTable[0x84ec] = {36, 26};
    TheTable[0x8511] = {36, 27};
    TheTable[0x8513] = {36, 28};
    TheTable[0x852c] = {36, 29};
    TheTable[0x853c] = {36, 30};
    TheTable[0x853d] = {36, 31};
    TheTable[0x8549] = {36, 32};
    TheTable[0x8568] = {36, 33};
    TheTable[0x8584] = {36, 34};
    TheTable[0x85aa] = {36, 35};
    TheTable[0x85af] = {36, 36};
    TheTable[0x85c9] = {36, 37};
    TheTable[0x85cf] = {36, 38};
    TheTable[0x8611] = {36, 39};
    TheTable[0x8638] = {36, 40};
    TheTable[0x864e] = {36, 41};
    TheTable[0x864f] = {36, 42};
    TheTable[0x8650] = {36, 43};
    TheTable[0x8651] = {36, 44};
    TheTable[0x865a] = {36, 45};
    TheTable[0x866b] = {36, 46};
    TheTable[0x8679] = {36, 47};
    TheTable[0x867d] = {36, 48};
    TheTable[0x867e] = {36, 49};
    TheTable[0x8680] = {36, 50};
    TheTable[0x8681] = {36, 51};
    TheTable[0x8682] = {36, 52};
    TheTable[0x868a] = {36, 53};
    TheTable[0x8695] = {36, 54};
    TheTable[0x86c7] = {36, 55};
    TheTable[0x86cb] = {36, 56};
    TheTable[0x86ce] = {36, 57};
    TheTable[0x86d9] = {36, 58};
    TheTable[0x86db] = {36, 59};
    TheTable[0x86ee] = {36, 60};
    TheTable[0x86fe] = {36, 61};
    TheTable[0x8702] = {36, 62};
    TheTable[0x8713] = {36, 63};
    TheTable[0x8718] = {37, 0};
    TheTable[0x871c] = {37, 1};
    TheTable[0x8721] = {37, 2};
    TheTable[0x8725] = {37, 3};
    TheTable[0x8734] = {37, 4};
    TheTable[0x8737] = {37, 5};
    TheTable[0x873b] = {37, 6};
    TheTable[0x8747] = {37, 7};
    TheTable[0x8770] = {37, 8};
    TheTable[0x8774] = {37, 9};
    TheTable[0x8776] = {37, 10};
    TheTable[0x878d] = {37, 11};
    TheTable[0x87ba] = {37, 12};
    TheTable[0x8822] = {37, 13};
    TheTable[0x8840] = {37, 14};
    TheTable[0x884c] = {37, 15};
    TheTable[0x884d] = {37, 16};
    TheTable[0x8854] = {37, 17};
    TheTable[0x8857] = {37, 18};
    TheTable[0x8861] = {37, 19};
    TheTable[0x8863] = {37, 20};
    TheTable[0x8865] = {37, 21};
    TheTable[0x8868] = {37, 22};
    TheTable[0x886b] = {37, 23};
    TheTable[0x886c] = {37, 24};
    TheTable[0x8870] = {37, 25};
    TheTable[0x8877] = {37, 26};
    TheTable[0x8884] = {37, 27};
    TheTable[0x888b] = {37, 28};
    TheTable[0x888d] = {37, 29};
    TheTable[0x8896] = {37, 30};
    TheTable[0x889c] = {37, 31};
    TheTable[0x88ab] = {37, 32};
    TheTable[0x88ad] = {37, 33};
    TheTable[0x88c1] = {37, 34};
    TheTable[0x88c2] = {37, 35};
    TheTable[0x88c5] = {37, 36};
    TheTable[0x88d4] = {37, 37};
    TheTable[0x88d5] = {37, 38};
    TheTable[0x88d9] = {37, 39};
    TheTable[0x88e4] = {37, 40};
    TheTable[0x88f3] = {37, 41};
    TheTable[0x88f9] = {37, 42};
    TheTable[0x8910] = {37, 43};
    TheTable[0x897f] = {37, 44};
    TheTable[0x8981] = {37, 45};
    TheTable[0x8986] = {37, 46};
    TheTable[0x89c1] = {37, 47};
    TheTable[0x89c2] = {37, 48};
    TheTable[0x89c4] = {37, 49};
    TheTable[0x89c6] = {37, 50};
    TheTable[0x89c8] = {37, 51};
    TheTable[0x89c9] = {37, 52};
    TheTable[0x89d2] = {37, 53};
    TheTable[0x89e3] = {37, 54};
    TheTable[0x89e6] = {37, 55};
    TheTable[0x8a00] = {37, 56};
    TheTable[0x8a79] = {37, 57};
    TheTable[0x8a89] = {37, 58};
    TheTable[0x8a93] = {37, 59};
    TheTable[0x8b66] = {37, 60};
    TheTable[0x8ba1] = {37, 61};
    TheTable[0x8ba2] = {37, 62};
    TheTable[0x8ba4] = {37, 63};
    TheTable[0x8ba8] = {38, 0};
    TheTable[0x8ba9] = {38, 1};
    TheTable[0x8baa] = {38, 2};
    TheTable[0x8bad] = {38, 3};
    TheTable[0x8bae] = {38, 4};
    TheTable[0x8baf] = {38, 5};
    TheTable[0x8bb0] = {38, 6};
    TheTable[0x8bb2] = {38, 7};
    TheTable[0x8bb6] = {38, 8};
    TheTable[0x8bb8] = {38, 9};
    TheTable[0x8bba] = {38, 10};
    TheTable[0x8bbd] = {38, 11};
    TheTable[0x8bbe] = {38, 12};
    TheTable[0x8bbf] = {38, 13};
    TheTable[0x8bc1] = {38, 14};
    TheTable[0x8bc4] = {38, 15};
    TheTable[0x8bc5] = {38, 16};
    TheTable[0x8bc6] = {38, 17};
    TheTable[0x8bc8] = {38, 18};
    TheTable[0x8bc9] = {38, 19};
    TheTable[0x8bca] = {38, 20};
    TheTable[0x8bcd] = {38, 21};
    TheTable[0x8bd1] = {38, 22};
    TheTable[0x8bd5] = {38, 23};
    TheTable[0x8bd7] = {38, 24};
    TheTable[0x8bda] = {38, 25};
    TheTable[0x8bdd] = {38, 26};
    TheTable[0x8bde] = {38, 27};
    TheTable[0x8be2] = {38, 28};
    TheTable[0x8be5] = {38, 29};
    TheTable[0x8be6] = {38, 30};
    TheTable[0x8be7] = {38, 31};
    TheTable[0x8beb] = {38, 32};
    TheTable[0x8bed] = {38, 33};
    TheTable[0x8bef] = {38, 34};
    TheTable[0x8bf1] = {38, 35};
    TheTable[0x8bf4] = {38, 36};
    TheTable[0x8bf5] = {38, 37};
    TheTable[0x8bf6] = {38, 38};
    TheTable[0x8bf7] = {38, 39};
    TheTable[0x8bf8] = {38, 40};
    TheTable[0x8bfa] = {38, 41};
    TheTable[0x8bfb] = {38, 42};
    TheTable[0x8bfe] = {38, 43};
    TheTable[0x8c00] = {38, 44};
    TheTable[0x8c01] = {38, 45};
    TheTable[0x8c03] = {38, 46};
    TheTable[0x8c05] = {38, 47};
    TheTable[0x8c08] = {38, 48};
    TheTable[0x8c0a] = {38, 49};
    TheTable[0x8c0b] = {38, 50};
    TheTable[0x8c0d] = {38, 51};
    TheTable[0x8c0e] = {38, 52};
    TheTable[0x8c10] = {38, 53};
    TheTable[0x8c13] = {38, 54};
    TheTable[0x8c1a] = {38, 55};
    TheTable[0x8c1c] = {38, 56};
    TheTable[0x8c22] = {38, 57};
    TheTable[0x8c23] = {38, 58};
    TheTable[0x8c26] = {38, 59};
    TheTable[0x8c28] = {38, 60};
    TheTable[0x8c2c] = {38, 61};
    TheTable[0x8c31] = {38, 62};
    TheTable[0x8c37] = {38, 63};
    TheTable[0x8c46] = {39, 0};
    TheTable[0x8c61] = {39, 1};
    TheTable[0x8c6a] = {39, 2};
    TheTable[0x8c6b] = {39, 3};
    TheTable[0x8c79] = {39, 4};
    TheTable[0x8c82] = {39, 5};
    TheTable[0x8c8c] = {39, 6};
    TheTable[0x8d1d] = {39, 7};
    TheTable[0x8d1e] = {39, 8};
    TheTable[0x8d1f] = {39, 9};
    TheTable[0x8d21] = {39, 10};
    TheTable[0x8d22] = {39, 11};
    TheTable[0x8d23] = {39, 12};
    TheTable[0x8d24] = {39, 13};
    TheTable[0x8d25] = {39, 14};
    TheTable[0x8d26] = {39, 15};
    TheTable[0x8d27] = {39, 16};
    TheTable[0x8d28] = {39, 17};
    TheTable[0x8d29] = {39, 18};
    TheTable[0x8d2a] = {39, 19};
    TheTable[0x8d2b] = {39, 20};
    TheTable[0x8d2c] = {39, 21};
    TheTable[0x8d2d] = {39, 22};
    TheTable[0x8d2f] = {39, 23};
    TheTable[0x8d31] = {39, 24};
    TheTable[0x8d34] = {39, 25};
    TheTable[0x8d35] = {39, 26};
    TheTable[0x8d37] = {39, 27};
    TheTable[0x8d38] = {39, 28};
    TheTable[0x8d39] = {39, 29};
    TheTable[0x8d3a] = {39, 30};
    TheTable[0x8d3c] = {39, 31};
    TheTable[0x8d3f] = {39, 32};
    TheTable[0x8d42] = {39, 33};
    TheTable[0x8d43] = {39, 34};
    TheTable[0x8d44] = {39, 35};
    TheTable[0x8d4b] = {39, 36};
    TheTable[0x8d4c] = {39, 37};
    TheTable[0x8d4f] = {39, 38};
    TheTable[0x8d54] = {39, 39};
    TheTable[0x8d56] = {39, 40};
    TheTable[0x8d58] = {39, 41};
    TheTable[0x8d5a] = {39, 42};
    TheTable[0x8d5b] = {39, 43};
    TheTable[0x8d5e] = {39, 44};
    TheTable[0x8d60] = {39, 45};
    TheTable[0x8d62] = {39, 46};
    TheTable[0x8d64] = {39, 47};
    TheTable[0x8d6b] = {39, 48};
    TheTable[0x8d70] = {39, 49};
    TheTable[0x8d74] = {39, 50};
    TheTable[0x8d75] = {39, 51};
    TheTable[0x8d76] = {39, 52};
    TheTable[0x8d77] = {39, 53};
    TheTable[0x8d81] = {39, 54};
    TheTable[0x8d85] = {39, 55};
    TheTable[0x8d8a] = {39, 56};
    TheTable[0x8d8b] = {39, 57};
    TheTable[0x8d9f] = {39, 58};
    TheTable[0x8da3] = {39, 59};
    TheTable[0x8db3] = {39, 60};
    TheTable[0x8db4] = {39, 61};
    TheTable[0x8dbe] = {39, 62};
    TheTable[0x8dc3] = {39, 63};
    TheTable[0x8dcc] = {40, 0};
    TheTable[0x8dd1] = {40, 1};
    TheTable[0x8ddd] = {40, 2};
    TheTable[0x8ddf] = {40, 3};
    TheTable[0x8de8] = {40, 4};
    TheTable[0x8dea] = {40, 5};
    TheTable[0x8def] = {40, 6};
    TheTable[0x8df3] = {40, 7};
    TheTable[0x8df5] = {40, 8};
    TheTable[0x8e0c] = {40, 9};
    TheTable[0x8e0f] = {40, 10};
    TheTable[0x8e22] = {40, 11};
    TheTable[0x8e29] = {40, 12};
    TheTable[0x8e2a] = {40, 13};
    TheTable[0x8e44] = {40, 14};
    TheTable[0x8e48] = {40, 15};
    TheTable[0x8e4b] = {40, 16};
    TheTable[0x8e66] = {40, 17};
    TheTable[0x8e6c] = {40, 18};
    TheTable[0x8e6d] = {40, 19};
    TheTable[0x8e72] = {40, 20};
    TheTable[0x8e81] = {40, 21};
    TheTable[0x8e87] = {40, 22};
    TheTable[0x8eab] = {40, 23};
    TheTable[0x8eac] = {40, 24};
    TheTable[0x8eaf] = {40, 25};
    TheTable[0x8eb2] = {40, 26};
    TheTable[0x8eba] = {40, 27};
    TheTable[0x8f66] = {40, 28};
    TheTable[0x8f67] = {40, 29};
    TheTable[0x8f68] = {40, 30};
    TheTable[0x8f6c] = {40, 31};
    TheTable[0x8f6e] = {40, 32};
    TheTable[0x8f6f] = {40, 33};
    TheTable[0x8f70] = {40, 34};
    TheTable[0x8f74] = {40, 35};
    TheTable[0x8f7b] = {40, 36};
    TheTable[0x8f7d] = {40, 37};
    TheTable[0x8f7f] = {40, 38};
    TheTable[0x8f83] = {40, 39};
    TheTable[0x8f85] = {40, 40};
    TheTable[0x8f86] = {40, 41};
    TheTable[0x8f88] = {40, 42};
    TheTable[0x8f89] = {40, 43};
    TheTable[0x8f8a] = {40, 44};
    TheTable[0x8f8d] = {40, 45};
    TheTable[0x8f90] = {40, 46};
    TheTable[0x8f91] = {40, 47};
    TheTable[0x8f93] = {40, 48};
    TheTable[0x8f96] = {40, 49};
    TheTable[0x8f99] = {40, 50};
    TheTable[0x8f9b] = {40, 51};
    TheTable[0x8f9c] = {40, 52};
    TheTable[0x8f9e] = {40, 53};
    TheTable[0x8f9f] = {40, 54};
    TheTable[0x8fa3] = {40, 55};
    TheTable[0x8fa8] = {40, 56};
    TheTable[0x8fa9] = {40, 57};
    TheTable[0x8fab] = {40, 58};
    TheTable[0x8fb0] = {40, 59};
    TheTable[0x8fb1] = {40, 60};
    TheTable[0x8fb9] = {40, 61};
    TheTable[0x8fbd] = {40, 62};
    TheTable[0x8fbe] = {40, 63};
    TheTable[0x8fc1] = {41, 0};
    TheTable[0x8fc2] = {41, 1};
    TheTable[0x8fc5] = {41, 2};
    TheTable[0x8fc7] = {41, 3};
    TheTable[0x8fc8] = {41, 4};
    TheTable[0x8fce] = {41, 5};
    TheTable[0x8fd0] = {41, 6};
    TheTable[0x8fd1] = {41, 7};
    TheTable[0x8fd4] = {41, 8};
    TheTable[0x8fd8] = {41, 9};
    TheTable[0x8fd9] = {41, 10};
    TheTable[0x8fdb] = {41, 11};
    TheTable[0x8fdc] = {41, 12};
    TheTable[0x8fdd] = {41, 13};
    TheTable[0x8fde] = {41, 14};
    TheTable[0x8fdf] = {41, 15};
    TheTable[0x8fea] = {41, 16};
    TheTable[0x8feb] = {41, 17};
    TheTable[0x8ff0] = {41, 18};
    TheTable[0x8ff7] = {41, 19};
    TheTable[0x8ff9] = {41, 20};
    TheTable[0x8ffd] = {41, 21};
    TheTable[0x9000] = {41, 22};
    TheTable[0x9001] = {41, 23};
    TheTable[0x9002] = {41, 24};
    TheTable[0x9003] = {41, 25};
    TheTable[0x9006] = {41, 26};
    TheTable[0x9009] = {41, 27};
    TheTable[0x900a] = {41, 28};
    TheTable[0x900d] = {41, 29};
    TheTable[0x900f] = {41, 30};
    TheTable[0x9010] = {41, 31};
    TheTable[0x9012] = {41, 32};
    TheTable[0x9014] = {41, 33};
    TheTable[0x9017] = {41, 34};
    TheTable[0x901a] = {41, 35};
    TheTable[0x901b] = {41, 36};
    TheTable[0x901d] = {41, 37};
    TheTable[0x901e] = {41, 38};
    TheTable[0x901f] = {41, 39};
    TheTable[0x9020] = {41, 40};
    TheTable[0x9022] = {41, 41};
    TheTable[0x902e] = {41, 42};
    TheTable[0x9038] = {41, 43};
    TheTable[0x903b] = {41, 44};
    TheTable[0x903c] = {41, 45};
    TheTable[0x9047] = {41, 46};
    TheTable[0x904d] = {41, 47};
    TheTable[0x9053] = {41, 48};
    TheTable[0x9057] = {41, 49};
    TheTable[0x9062] = {41, 50};
    TheTable[0x9063] = {41, 51};
    TheTable[0x9065] = {41, 52};
    TheTable[0x906d] = {41, 53};
    TheTable[0x906e] = {41, 54};
    TheTable[0x9075] = {41, 55};
    TheTable[0x907f] = {41, 56};
    TheTable[0x9080] = {41, 57};
    TheTable[0x908b] = {41, 58};
    TheTable[0x90a3] = {41, 59};
    TheTable[0x90a6] = {41, 60};
    TheTable[0x90aa] = {41, 61};
    TheTable[0x90ae] = {41, 62};
    TheTable[0x90bb] = {41, 63};
    TheTable[0x90ca] = {42, 0};
    TheTable[0x90ce] = {42, 1};
    TheTable[0x90d1] = {42, 2};
    TheTable[0x90e8] = {42, 3};
    TheTable[0x90fd] = {42, 4};
    TheTable[0x9119] = {42, 5};
    TheTable[0x914d] = {42, 6};
    TheTable[0x9152] = {42, 7};
    TheTable[0x916a] = {42, 8};
    TheTable[0x916c] = {42, 9};
    TheTable[0x9171] = {42, 10};
    TheTable[0x9177] = {42, 11};
    TheTable[0x9178] = {42, 12};
    TheTable[0x917f] = {42, 13};
    TheTable[0x9187] = {42, 14};
    TheTable[0x9189] = {42, 15};
    TheTable[0x918b] = {42, 16};
    TheTable[0x9192] = {42, 17};
    TheTable[0x91c7] = {42, 18};
    TheTable[0x91ca] = {42, 19};
    TheTable[0x91cc] = {42, 20};
    TheTable[0x91cd] = {42, 21};
    TheTable[0x91ce] = {42, 22};
    TheTable[0x91cf] = {42, 23};
    TheTable[0x91d1] = {42, 24};
    TheTable[0x9274] = {42, 25};
    TheTable[0x9488] = {42, 26};
    TheTable[0x9489] = {42, 27};
    TheTable[0x9493] = {42, 28};
    TheTable[0x949d] = {42, 29};
    TheTable[0x949e] = {42, 30};
    TheTable[0x949f] = {42, 31};
    TheTable[0x94a2] = {42, 32};
    TheTable[0x94a5] = {42, 33};
    TheTable[0x94a9] = {42, 34};
    TheTable[0x94ae] = {42, 35};
    TheTable[0x94b1] = {42, 36};
    TheTable[0x94b3] = {42, 37};
    TheTable[0x94bb] = {42, 38};
    TheTable[0x94c1] = {42, 39};
    TheTable[0x94c3] = {42, 40};
    TheTable[0x94c5] = {42, 41};
    TheTable[0x94dc] = {42, 42};
    TheTable[0x94dd] = {42, 43};
    TheTable[0x94ec] = {42, 44};
    TheTable[0x94f2] = {42, 45};
    TheTable[0x94f6] = {42, 46};
    TheTable[0x94f8] = {42, 47};
    TheTable[0x94fa] = {42, 48};
    TheTable[0x94fe] = {42, 49};
    TheTable[0x9500] = {42, 50};
    TheTable[0x9501] = {42, 51};
    TheTable[0x9504] = {42, 52};
    TheTable[0x9505] = {42, 53};
    TheTable[0x9508] = {42, 54};
    TheTable[0x950b] = {42, 55};
    TheTable[0x9510] = {42, 56};
    TheTable[0x9519] = {42, 57};
    TheTable[0x9521] = {42, 58};
    TheTable[0x9523] = {42, 59};
    TheTable[0x9524] = {42, 60};
    TheTable[0x9525] = {42, 61};
    TheTable[0x9526] = {42, 62};
    TheTable[0x952e] = {42, 63};
    TheTable[0x952f] = {43, 0};
    TheTable[0x9539] = {43, 1};
    TheTable[0x953b] = {43, 2};
    TheTable[0x9547] = {43, 3};
    TheTable[0x9556] = {43, 4};
    TheTable[0x955c] = {43, 5};
    TheTable[0x9570] = {43, 6};
    TheTable[0x957f] = {43, 7};
    TheTable[0x95e8] = {43, 8};
    TheTable[0x95ea] = {43, 9};
    TheTable[0x95ed] = {43, 10};
    TheTable[0x95ee] = {43, 11};
    TheTable[0x95ef] = {43, 12};
    TheTable[0x95f2] = {43, 13};
    TheTable[0x95f4] = {43, 14};
    TheTable[0x95f7] = {43, 15};
    TheTable[0x95f8] = {43, 16};
    TheTable[0x95f9] = {43, 17};
    TheTable[0x95fb] = {43, 18};
    TheTable[0x9600] = {43, 19};
    TheTable[0x9601] = {43, 20};
    TheTable[0x9605] = {43, 21};
    TheTable[0x9609] = {43, 22};
    TheTable[0x9614] = {43, 23};
    TheTable[0x961f] = {43, 24};
    TheTable[0x9631] = {43, 25};
    TheTable[0x9632] = {43, 26};
    TheTable[0x9633] = {43, 27};
    TheTable[0x9634] = {43, 28};
    TheTable[0x9635] = {43, 29};
    TheTable[0x9636] = {43, 30};
    TheTable[0x963b] = {43, 31};
    TheTable[0x963f] = {43, 32};
    TheTable[0x9640] = {43, 33};
    TheTable[0x9644] = {43, 34};
    TheTable[0x9645] = {43, 35};
    TheTable[0x9646] = {43, 36};
    TheTable[0x9648] = {43, 37};
    TheTable[0x964c] = {43, 38};
    TheTable[0x964d] = {43, 39};
    TheTable[0x9650] = {43, 40};
    TheTable[0x9655] = {43, 41};
    TheTable[0x9661] = {43, 42};
    TheTable[0x9662] = {43, 43};
    TheTable[0x9664] = {43, 44};
    TheTable[0x9669] = {43, 45};
    TheTable[0x966a] = {43, 46};
    TheTable[0x9675] = {43, 47};
    TheTable[0x9676] = {43, 48};
    TheTable[0x9677] = {43, 49};
    TheTable[0x9686] = {43, 50};
    TheTable[0x968f] = {43, 51};
    TheTable[0x9690] = {43, 52};
    TheTable[0x9694] = {43, 53};
    TheTable[0x9699] = {43, 54};
    TheTable[0x969c] = {43, 55};
    TheTable[0x96a7] = {43, 56};
    TheTable[0x96b6] = {43, 57};
    TheTable[0x96be] = {43, 58};
    TheTable[0x96c0] = {43, 59};
    TheTable[0x96c1] = {43, 60};
    TheTable[0x96c4] = {43, 61};
    TheTable[0x96c5] = {43, 62};
    TheTable[0x96c6] = {43, 63};
    TheTable[0x96c7] = {44, 0};
    TheTable[0x96d5] = {44, 1};
    TheTable[0x96e8] = {44, 2};
    TheTable[0x96ea] = {44, 3};
    TheTable[0x96f6] = {44, 4};
    TheTable[0x96f7] = {44, 5};
    TheTable[0x96f9] = {44, 6};
    TheTable[0x96fe] = {44, 7};
    TheTable[0x9700] = {44, 8};
    TheTable[0x9707] = {44, 9};
    TheTable[0x9709] = {44, 10};
    TheTable[0x970d] = {44, 11};
    TheTable[0x971c] = {44, 12};
    TheTable[0x971e] = {44, 13};
    TheTable[0x9730] = {44, 14};
    TheTable[0x9732] = {44, 15};
    TheTable[0x9738] = {44, 16};
    TheTable[0x9752] = {44, 17};
    TheTable[0x9753] = {44, 18};
    TheTable[0x9759] = {44, 19};
    TheTable[0x975e] = {44, 20};
    TheTable[0x9760] = {44, 21};
    TheTable[0x9762] = {44, 22};
    TheTable[0x9769] = {44, 23};
    TheTable[0x9774] = {44, 24};
    TheTable[0x9776] = {44, 25};
    TheTable[0x978b] = {44, 26};
    TheTable[0x978d] = {44, 27};
    TheTable[0x97a0] = {44, 28};
    TheTable[0x97ad] = {44, 29};
    TheTable[0x97e6] = {44, 30};
    TheTable[0x97f3] = {44, 31};
    TheTable[0x97f5] = {44, 32};
    TheTable[0x9875] = {44, 33};
    TheTable[0x9876] = {44, 34};
    TheTable[0x9877] = {44, 35};
    TheTable[0x9879] = {44, 36};
    TheTable[0x987a] = {44, 37};
    TheTable[0x987b] = {44, 38};
    TheTable[0x987d] = {44, 39};
    TheTable[0x987e] = {44, 40};
    TheTable[0x987f] = {44, 41};
    TheTable[0x9881] = {44, 42};
    TheTable[0x9882] = {44, 43};
    TheTable[0x9884] = {44, 44};
    TheTable[0x9886] = {44, 45};
    TheTable[0x9888] = {44, 46};
    TheTable[0x9891] = {44, 47};
    TheTable[0x9897] = {44, 48};
    TheTable[0x9898] = {44, 49};
    TheTable[0x989c] = {44, 50};
    TheTable[0x989d] = {44, 51};
    TheTable[0x98a0] = {44, 52};
    TheTable[0x98a4] = {44, 53};
    TheTable[0x98ce] = {44, 54};
    TheTable[0x98d8] = {44, 55};
    TheTable[0x98d9] = {44, 56};
    TheTable[0x98da] = {44, 57};
    TheTable[0x98de] = {44, 58};
    TheTable[0x98df] = {44, 59};
    TheTable[0x9910] = {44, 60};
    TheTable[0x9965] = {44, 61};
    TheTable[0x996d] = {44, 62};
    TheTable[0x996e] = {44, 63};
    TheTable[0x9970] = {45, 0};
    TheTable[0x9971] = {45, 1};
    TheTable[0x9972] = {45, 2};
    TheTable[0x9975] = {45, 3};
    TheTable[0x9976] = {45, 4};
    TheTable[0x997a] = {45, 5};
    TheTable[0x997c] = {45, 6};
    TheTable[0x997f] = {45, 7};
    TheTable[0x9985] = {45, 8};
    TheTable[0x9986] = {45, 9};
    TheTable[0x9988] = {45, 10};
    TheTable[0x998b] = {45, 11};
    TheTable[0x9992] = {45, 12};
    TheTable[0x9996] = {45, 13};
    TheTable[0x9999] = {45, 14};
    TheTable[0x9a6c] = {45, 15};
    TheTable[0x9a70] = {45, 16};
    TheTable[0x9a71] = {45, 17};
    TheTable[0x9a73] = {45, 18};
    TheTable[0x9a74] = {45, 19};
    TheTable[0x9a76] = {45, 20};
    TheTable[0x9a7b] = {45, 21};
    TheTable[0x9a7c] = {45, 22};
    TheTable[0x9a7e] = {45, 23};
    TheTable[0x9a82] = {45, 24};
    TheTable[0x9a84] = {45, 25};
    TheTable[0x9a86] = {45, 26};
    TheTable[0x9a8c] = {45, 27};
    TheTable[0x9a91] = {45, 28};
    TheTable[0x9a97] = {45, 29};
    TheTable[0x9a9a] = {45, 30};
    TheTable[0x9aa1] = {45, 31};
    TheTable[0x9aa4] = {45, 32};
    TheTable[0x9aa8] = {45, 33};
    TheTable[0x9ab0] = {45, 34};
    TheTable[0x9ad8] = {45, 35};
    TheTable[0x9ae6] = {45, 36};
    TheTable[0x9b3c] = {45, 37};
    TheTable[0x9b42] = {45, 38};
    TheTable[0x9b44] = {45, 39};
    TheTable[0x9b45] = {45, 40};
    TheTable[0x9b54] = {45, 41};
    TheTable[0x9c7c] = {45, 42};
    TheTable[0x9c7f] = {45, 43};
    TheTable[0x9c81] = {45, 44};
    TheTable[0x9c8d] = {45, 45};
    TheTable[0x9c9c] = {45, 46};
    TheTable[0x9cb8] = {45, 47};
    TheTable[0x9cd5] = {45, 48};
    TheTable[0x9cd6] = {45, 49};
    TheTable[0x9cd7] = {45, 50};
    TheTable[0x9e1f] = {45, 51};
    TheTable[0x9e21] = {45, 52};
    TheTable[0x9e23] = {45, 53};
    TheTable[0x9e25] = {45, 54};
    TheTable[0x9e26] = {45, 55};
    TheTable[0x9e2d] = {45, 56};
    TheTable[0x9e3d] = {45, 57};
    TheTable[0x9e45] = {45, 58};
    TheTable[0x9e49] = {45, 59};
    TheTable[0x9e4a] = {45, 60};
    TheTable[0x9e66] = {45, 61};
    TheTable[0x9e70] = {45, 62};
    TheTable[0x9e7f] = {45, 63};
    TheTable[0x9ea6] = {46, 0};
    TheTable[0x9ebb] = {46, 1};
    TheTable[0x9ec4] = {46, 2};
    TheTable[0x9ece] = {46, 3};
    TheTable[0x9ecf] = {46, 4};
    TheTable[0x9ed1] = {46, 5};
    TheTable[0x9ed8] = {46, 6};
    TheTable[0x9f13] = {46, 7};
    TheTable[0x9f20] = {46, 8};
    TheTable[0x9f39] = {46, 9};
    TheTable[0x9f3b] = {46, 10};
    TheTable[0x9f50] = {46, 11};
    TheTable[0x9f7f] = {46, 12};
    TheTable[0x9f84] = {46, 13};
    TheTable[0x9f99] = {46, 14};
    TheTable[0x9f9a] = {46, 15};
    TheTable[0x9f9f] = {46, 16};
    TheTable[0xff01] = {46, 17};
    TheTable[0xff08] = {46, 18};
    TheTable[0xff09] = {46, 19};
    TheTable[0xff0c] = {46, 20};
    TheTable[0xff0d] = {46, 21};
    TheTable[0xff1a] = {46, 22};
    TheTable[0xff1b] = {46, 23};
    TheTable[0xff1f] = {46, 24};
    TheTable[0xff5e] = {46, 25};

    return true;
}

void Shutdown()
{
    m_ChsSprite.Delete();
}
} // namespace FontPatch