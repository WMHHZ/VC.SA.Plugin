#include "CFont.h"
#include "CSprite2d.h"
#include "CTimer.h"
#include "rwFunc.h"
#include "CTxdStore.h"
#include "CCharTable.h"

const short CFont::iMaxCharWidth = 28;
const float CFont::fMaxCharWidth = CFont::iMaxCharWidth;

char CFont::fontPath[260];
char CFont::textPath[260];

CFontSizes *CFont::Size;
FontBufferPointer CFont::FontBuffer;
FontBufferPointer *CFont::FontBufferIter;
CFontRenderState *CFont::RenderState;
CFontDetails *CFont::Details;

CSprite2d *CFont::Sprite;
CSprite2d CFont::ChsSprite;
CSprite2d CFont::ChsSlantSprite;

injector::hook_back<CharType(*)(CharType arg_char)>
CFont::fpFindNewCharacter;

injector::hook_back<CharType *(*)(CharType *)>
CFont::fpParseTokenEPt;

injector::hook_back<CharType *(*)(CharType *arg_text, CRGBA &result_color, bool &result_blip, bool &result_bold)>
CFont::fpParseTokenEPtR5CRGBARbRb;

injector::hook_back<void(*)(float arg_x, float arg_y, CharType arg_char)>
CFont::fpPrintChar;

injector::hook_back<void(*)(float arg_x, float arg_y, unsigned int useless, CharType *arg_strbeg, CharType *arg_strend, float justifywrap)>
CFont::fpPrintStringPart;

void CFont::LoadCHSFont()
{
    CTxdStore::fpPopCurrentTxd.fun();
    int slot = CTxdStore::fpAddTxdSlot.fun("wm_vcchs");
    CTxdStore::fpLoadTxd.fun(slot, fontPath);
    CTxdStore::fpAddRef.fun(slot);
    CTxdStore::fpPushCurrentTxd.fun();
    CTxdStore::fpSetCurrentTxd.fun(slot);
    CSprite2d::fpSetTexture.fun(&ChsSprite, 0, "normal", "normalm");
    CSprite2d::fpSetTexture.fun(&ChsSlantSprite, 0, "slant", "slantm");
    CTxdStore::fpPopCurrentTxd.fun();
}

void CFont::UnloadCHSFont(int dummy)
{
    CTxdStore::fpRemoveTxdSlot.fun(dummy);
    CSprite2d::fpDelete.fun(&ChsSprite, 0);
    CSprite2d::fpDelete.fun(&ChsSlantSprite, 0);
    CTxdStore::fpRemoveTxdSlot.fun(CTxdStore::fpFindTxdSlot.fun("wm_vcchs"));
}

float CFont::GetCharacterSize(CharType arg_char, short nFontStyle, bool bBaseCharset, bool bProp, float fScaleX)
{
    short charWidth;

    if (arg_char >= 0x80)
    {
        charWidth = iMaxCharWidth + 1;
    }
    else
    {
        arg_char -= 0x20;

        if (bBaseCharset)
        {
            arg_char = fpFindNewCharacter.fun(arg_char);
        }

        if (bProp)
        {
            charWidth = Size[nFontStyle].PropValues[arg_char];
        }
        else
        {
            charWidth = Size[nFontStyle].UnpropValue;
        }
    }

    return (charWidth * fScaleX);
}

float CFont::GetCharacterSizeNormal(CharType arg_char)
{
    return GetCharacterSize(arg_char, Details->FontStyle, Details->BaseCharset, Details->Prop, Details->Scale.x);
}

float CFont::GetCharacterSizeDrawing(CharType arg_char)
{
    return GetCharacterSize(arg_char, RenderState->FontStyle, RenderState->BaseCharset, RenderState->Prop, RenderState->Scale.x);
}

float CFont::GetStringWidth(CharType *arg_text, bool bGetAll)
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

CharType *CFont::GetNextSpace(CharType *arg_text)
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

short CFont::GetNumberLines(float arg_x, float arg_y, CharType *arg_text)
{
    short result = 0;
    float xBound;
    float yBound = arg_y;
    float strWidth, widthLimit;

    if (Details->Centre || Details->RightJustify)
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

        if (Details->Centre)
        {
            widthLimit = Details->CentreSize;
        }
        else
        {
            widthLimit = Details->WrapX;
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
            if (Details->Centre || Details->RightJustify)
            {
                xBound = 0.0f;
            }
            else
            {
                xBound = arg_x;
            }

            ++result;
            yBound += Details->Scale.y * 18.0f;
        }
    }

    return result;
}

void CFont::GetTextRect(CRect *result, float arg_x, float arg_y, CharType *arg_text)
{
    short numLines = GetNumberLines(arg_x, arg_y, arg_text);

    if (Details->Centre)
    {
        if (Details->BackGroundOnlyText)
        {
            result->x1 = arg_x - 4.0f;
            result->x2 = arg_x + 4.0f;
            result->y1 = (18.0f * Details->Scale.y) * numLines + arg_y + 2.0f;
            result->y2 = arg_y - 2.0f;
        }
        else
        {
            result->x1 = arg_x - (Details->CentreSize * 0.5f) - 4.0f;
            result->x2 = arg_x + (Details->CentreSize * 0.5f) + 4.0f;
            result->y1 = arg_y + (18.0f * Details->Scale.y * numLines) + 2.0f;
            result->y2 = arg_y - 2.0f;
        }
    }
    else
    {
        result->x1 = arg_x - 4.0f;
        result->x2 = Details->WrapX;
        result->y1 = arg_y;
        result->y2 = (18.0f * Details->Scale.y) * numLines + arg_y + 4.0f;
    }
}

void CFont::PrintString(float arg_x, float arg_y, CharType *arg_text)
{
    CRect textBoxRect;

    float xBound;
    float yBound = arg_y;
    float strWidth, widthLimit;
    float var_38 = 0.0f;
    float print_x;
    float justifyWrap;

    CharType *ptext = arg_text;
    CharType *strHead = arg_text;

    bool emptyLine = true;

    short numSpaces = 0;

    Details->UselessFlag1 = false;

    if (*arg_text == '*')
    {
        return;
    }

    ++Details->TextCount;

    if (Details->Background)
    {
        GetTextRect(&textBoxRect, arg_x, arg_y, arg_text);
        CSprite2d::fpDrawRect.fun(textBoxRect, Details->BackgroundColor);
    }

    if (Details->Centre || Details->RightJustify)
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

        if (Details->Centre)
        {
            widthLimit = Details->CentreSize;
        }
        else if (Details->RightJustify)
        {
            widthLimit = arg_x - Details->RightJustifyWrap;
        }
        else
        {
            widthLimit = Details->WrapX;
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
                if (Details->Centre)
                {
                    print_x = arg_x - xBound * 0.5f;
                }
                else if (Details->RightJustify)
                {
                    print_x = arg_x - xBound;
                }
                else
                {
                    print_x = arg_x;
                }

                fpPrintStringPart.fun(print_x, yBound, 0, strHead, ptext, 0.0f);
            }
        }
        else
        {
            if (Details->Justify && !(Details->Centre))
            {
                justifyWrap = (Details->WrapX - var_38) / numSpaces;
            }
            else
            {
                justifyWrap = 0.0f;
            }

            if (Details->Centre)
            {
                print_x = arg_x - xBound * 0.5f;
            }
            else if (Details->RightJustify)
            {
                print_x = arg_x - xBound;
            }
            else
            {
                print_x = arg_x;
            }

            fpPrintStringPart.fun(print_x, yBound, 0, strHead, ptext, justifyWrap);

            strHead = ptext;

            if (Details->Centre || Details->RightJustify)
            {
                xBound = 0.0f;
            }
            else
            {
                xBound = arg_x;
            }

            yBound += Details->Scale.y * 18.0f;
            var_38 = 0.0f;
            numSpaces = 0;
            emptyLine = true;
        }
    }
}

void CFont::RenderFontBuffer()
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

    *RenderState = *(FontBuffer.pdata);
    var_14 = FontBuffer.pdata->Color;

    pos = RenderState->Pos;

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

            *RenderState = *pbuffer.pdata;

            var_14 = RenderState->Color;

            pos = RenderState->Pos;

            pbuffer.addr += 0x30;
        }

        if (*pbuffer.ptext == '~')
        {
            pbuffer.ptext = fpParseTokenEPtR5CRGBARbRb.fun(pbuffer.ptext, var_14, var_E, var_D);

            if (var_E)
            {
                if ((*CTimer::m_nTimeInMilliseconds - Details->BlipStartTime) > 300)
                {
                    Details->IsBlip = true;
                    Details->BlipStartTime = *CTimer::m_nTimeInMilliseconds;
                }

                if (Details->IsBlip)
                {
                    Details->Color.alpha = 0;
                }
                else
                {
                    Details->Color.alpha = 255;
                }
            }

            if (!RenderState->KeepColor)
            {
                RenderState->Color = var_14;
            }
        }

        if (RenderState->Slant != 0.0f)
        {
            pos.y = (RenderState->SlantRefPoint.x - pos.x) * RenderState->Slant + RenderState->SlantRefPoint.y;
        }

        var_char = *pbuffer.ptext;

        if (var_char < 0x80)
        {
            CSprite2d::fpSetRenderState.fun(&Sprite[RenderState->FontStyle], 0);
        }
        else
        {
            if (RenderState->Slant == 0.0f)
            {
                CSprite2d::fpSetRenderState.fun(&ChsSprite, 0);
            }
            else
            {
                CSprite2d::fpSetRenderState.fun(&ChsSlantSprite, 0);
            }
        }

        rwFunc::fpRwRenderStateSet.fun(RwRenderState::rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);

        PrintCharDispatcher(pos.x, pos.y, var_char);

        if (var_D)
        {
            PrintCharDispatcher(pos.x + 1.0f, pos.y, var_char);
            PrintCharDispatcher(pos.x + 2.0f, pos.y, var_char);
            pos.x += 2.0f;
        }

        CSprite2d::fpRenderVertexBuffer.fun();

        pos.x += GetCharacterSizeDrawing(var_char);

        if (var_char == ' ')
        {
            pos.x += RenderState->JustifyWrap;
        }

        ++pbuffer.ptext;
    }

    FontBufferIter->addr = FontBuffer.addr;


}

void CFont::PrintCHSChar(float arg_x, float arg_y, CharType arg_char)
{
    static const float rRowsCount = 1.0f / 64.0f;
    static const float rColumnsCount = 1.0f / 64.0f;
    static const float ufix = 0.001f / 4.0f;
    //static const float vfix = 0.0021f / 4.0f;
    static const float vfix = 0.001f / 4.0f;
    static const float vfix1_slant = 0.00055f / 4.0f;
    //static const float vfix2_slant = 0.01f / 4.0f;
    static const float vfix2_slant = 0.007f / 4.0f;
    static const float vfix3_slant = 0.009f / 4.0f;

    CRect rect;

    float yOffset;

    float u1, v1, u2, v2, u3, v3, u4, v4;

    CharPos pos;

    if (arg_x >= *rwFunc::RsGlobalW ||
        arg_x <= 0.0f ||
        arg_y <= 0.0f ||
        arg_y >= *rwFunc::RsGlobalH)
    {
        return;
    }

    pos = CCharTable::GetCharPos(arg_char);

    yOffset = RenderState->Scale.y * 2.0f;

    if (RenderState->Slant == 0.0f)
    {
        rect.x1 = arg_x;
        rect.y2 = arg_y + yOffset;
        rect.x2 = RenderState->Scale.x * 32.0f + arg_x;
        rect.y1 = RenderState->Scale.y * 16.0f + arg_y + yOffset;

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
        rect.x1 = arg_x;
        rect.y2 = arg_y + 0.015f + yOffset;
        rect.x2 = RenderState->Scale.x * 32.0f + arg_x;
        rect.y1 = RenderState->Scale.y * 16.0f + arg_y + yOffset;

        u1 = pos.columnIndex * rColumnsCount;
        v1 = pos.rowIndex * rRowsCount + vfix1_slant;
        u2 = (pos.columnIndex + 1) * rColumnsCount - ufix;
        v2 = pos.rowIndex * rRowsCount + vfix + vfix2_slant;
        u3 = pos.columnIndex * rColumnsCount;
        v3 = (pos.rowIndex + 1) * rRowsCount - vfix3_slant;
        u4 = (pos.columnIndex + 1) * rColumnsCount - ufix;
        v4 = (pos.rowIndex + 1) * rRowsCount + vfix2_slant - vfix;
    }

    CSprite2d::fpAddToBuffer.fun(rect, RenderState->Color, u1, v1, u2, v2, u3, v3, u4, v4);
}

void CFont::PrintCharDispatcher(float arg_x, float arg_y, CharType arg_char)
{
    if (arg_char < 0x80)
    {
        arg_char -= 0x20;

        if (RenderState->BaseCharset)
        {
            arg_char = fpFindNewCharacter.fun(arg_char);
        }

        fpPrintChar.fun(arg_x, arg_y, arg_char);
    }
    else
    {
        PrintCHSChar(arg_x, arg_y, arg_char);
    }
}

void CFont::DisableSlant(float slant)
{
    Details->Slant = 0.0f;
}
