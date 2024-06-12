#include "CFontPatch.h"
#include "CCharTable.h"
#include <game_sa/CTheScripts.h>
#include <game_sa/RenderWare.h>
#include <injector.hpp>
#include <utf8/unchecked.h>

#include <game_sa/CSprite.h>
#include <game_sa/RenderWare.h>

float CFontPatch::fix_value_2;
float CFontPatch::fix_value_2_chs;

const float CFontPatch::scale_x_rec = 1.0f / 16.0f;
const float CFontPatch::scale_y_rec = 1.0f / 12.8f;
const float CFontPatch::scale_x_rec_chs = 1.0f / 64.0f;
const float CFontPatch::scale_y_rec_chs = 1.0f / 51.2f;
const float CFontPatch::fix_value_1 = 0.0021f;
const float CFontPatch::fix_value_1_chs = 0.0021f / 4.0f;

CSprite2d CFontPatch::m_ChsSprite;

CFontRenderState *CFontPatch::m_FontBuffer;
FontBufferPointer *CFontPatch::m_FontBufferIter;
CFontRenderState *CFontPatch::RenderState;

unsigned char(__cdecl *CFontPatch::FindSubFontCharacter)(unsigned char, unsigned char);
void(__cdecl *CFontPatch::PrintKeyTokenFormat)(char *);
void(__cdecl *CFontPatch::RenderString)(float, float, const char *, const char *, float);

float __cdecl CFontPatch::GetScaledLetterWidthNormal(unsigned int arg_char)
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

float CFontPatch::GetScaledLetterWidthScript(unsigned int arg_char)
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
            style = 0;
            arg_char = FindSubFontCharacter(arg_char, 2);
            break;

        case 3:
            style = 1;
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

float __cdecl CFontPatch::GetScaledLetterWidthDrawing(unsigned int arg_char)
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

float CFontPatch::GetStringWidth(const char *arg_text, bool bGetAll, bool bScript)
{
    char strbuf[400];
    char *bufIter = strbuf;

    float result = 0.0f;

    bool succeeded = false;
    bool StopAtDelim = false;
    
    strncpy(strbuf, arg_text, 400);
    strbuf[399] = 0;

    PrintKeyTokenFormat(strbuf);

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

            do 
            {
                ++bufIter;
            } while (*bufIter != '~');

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

char *CFontPatch::GetNextSpace(char *arg_pointer)
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

short CFontPatch::ProcessCurrentString(bool print, float arg_x, float arg_y, char *arg_text)
{
    char *esi = arg_text;
    const char *ebp = esi;
    const char *edi;

    __int16 result = 0;
    __int16 numWords = 0;

    bool emptyLine = true;
    char tag = '\0';

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
        strWidth = GetStringWidth(esi, false, false);

        if (*esi == '~')
        {
            esi = CFont::ParseToken(esi, tagColor, true, &tag);
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
            esi = GetNextSpace(esi);

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

                var_110 = xBound;
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

            ebp = esi;
            emptyLine = true;
            var_110 = 0.0f;
            numWords = 0;
        }

        CFont::m_nExtraFontSymbolId = 0;
    };

    if (print)
    {
        CFont::SetColor(fontColor);
    }

    return result;
}

void CFontPatch::RenderFontBuffer()
{
    CRGBA var_color;
    CVector2D pos;
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

            ebx.ptext = CFont::ParseToken(ebx.ptext, var_color, RenderState->IsBlip, nullptr);

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

            RWSRCGLOBAL(dOpenDevice).fpRenderStateSet(RwRenderState::rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);

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
            pos.x += (RenderState->Scale.y * 17.0f + RenderState->OutlineSize);
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

void CFontPatch::PrintCHSChar(float arg_x, float arg_y, unsigned int arg_char)
{
    CRect rect;
    float row, column;
    CCharTable::CharPos cpos;

    if (arg_y < 0.0f || RsGlobal.maximumHeight < arg_y || arg_x < 0.0f || RsGlobal.maximumWidth < arg_x)
    {
        return;
    }

    if (CFont::m_nExtraFontSymbolId != 0)
    {
        rect.top = RenderState->Scale.y * 2.0f + arg_y;
        rect.right = RenderState->Scale.y * 17.0f + arg_x;
        rect.bottom = RenderState->Scale.y * 19.0f + arg_y;
        rect.left = arg_x;
        CFont::ButtonSprite[CFont::m_nExtraFontSymbolId].Draw(rect, CRGBA(255, 255, 255, RenderState->Color.a));
        return;
    }

    if (arg_char == 0 || arg_char == '?')
    {
        return;
    }

    cpos = CCharTable::GetCharPos(arg_char);

    row = cpos.rowIndex;
    column = cpos.columnIndex;

    rect.left = arg_x;
    rect.top = arg_y;
    rect.right = RenderState->Scale.x * 32.0f + arg_x;
    rect.bottom = RenderState->Scale.y * 20.0f + arg_y;

    if (arg_char < 0x60)
    {
        row *= scale_y_rec;
        column *= scale_x_rec;

        CSprite2d::AddToBuffer(rect, RenderState->Color,
            column,
            row + fix_value_2,
            column + scale_x_rec - fix_value_1,
            row + fix_value_2,
            column,
            row + scale_y_rec - fix_value_2,
            column + scale_x_rec - fix_value_1,
            row + scale_y_rec - fix_value_2);
    }
    else
    {
        row *= scale_y_rec_chs;
        column *= scale_x_rec_chs;

        CSprite2d::AddToBuffer(rect, RenderState->Color,
            column,
            row + fix_value_2_chs,
            column + scale_x_rec_chs - fix_value_1_chs,
            row + fix_value_2_chs,
            column,
            row + scale_y_rec_chs - fix_value_2_chs,
            column + scale_x_rec_chs - fix_value_1_chs,
            row + scale_y_rec_chs - fix_value_2_chs);
    }
}

void CFontPatch::Init10U()
{
    fix_value_2 = 0.0021f;
    fix_value_2_chs = 0.0021f / 4.0f;

    m_FontBuffer = injector::raw_ptr(0xC716B0).get();
    m_FontBufferIter = injector::raw_ptr(0xC716A8).get();
    RenderState = injector::raw_ptr(0xC71AA0).get();

    FindSubFontCharacter = injector::raw_ptr(0x7192C0).get();

    PrintKeyTokenFormat = injector::raw_ptr(0x69E160).get();
    RenderString = injector::raw_ptr(0x719B40).get();
}
