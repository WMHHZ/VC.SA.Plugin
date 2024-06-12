#include "CFontPatch.h"
#include <game_vc/CTimer.h>
#include <game_vc/CTxdStore.h>
#include <game_vc/RenderWare.h>
#include "CCharTable.h"

const short CFontPatch::iMaxCharWidth = 28;
const float CFontPatch::fMaxCharWidth = CFontPatch::iMaxCharWidth;

char CFontPatch::fontPath[260];
char CFontPatch::textPath[260];

tFontTable* CFontPatch::Size;
FontBufferPointer CFontPatch::FontBuffer;
FontBufferPointer* CFontPatch::FontBufferIter;

CSprite2d CFontPatch::ChsSprite;
CSprite2d CFontPatch::ChsSlantSprite;

void CFontPatch::LoadCHSFont()
{
    char normal[] = "normal";
    char normalm[] = "normalm";
    char slant[] = "slant";
    char slantm[] = "slantm";

    CTxdStore::PopCurrentTxd();
    int slot = CTxdStore::AddTxdSlot("wm_vcchs");
    CTxdStore::LoadTxd(slot, fontPath);
    CTxdStore::AddRef(slot);
    CTxdStore::PushCurrentTxd();
    CTxdStore::SetCurrentTxd(slot);
    ChsSprite.SetTexture(normal, normalm);
    ChsSlantSprite.SetTexture(slant, slantm);
    CTxdStore::PopCurrentTxd();
}

void CFontPatch::UnloadCHSFont(int dummy)
{
    CTxdStore::RemoveTxdSlot(dummy);
    ChsSprite.Delete();
    ChsSlantSprite.Delete();
    CTxdStore::RemoveTxdSlot(CTxdStore::FindTxdSlot("wm_vcchs"));
}

float CFontPatch::GetCharacterSize(CharType arg_char, short nFontStyle, bool bFontHalfTexture, bool bProp, float fScaleX)
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

float CFontPatch::GetCharacterSizeNormal(CharType arg_char)
{
    return GetCharacterSize(arg_char, CFont::Details.m_FontStyle, CFont::Details.m_bFontHalfTexture, CFont::Details.m_bPropOn, CFont::Details.m_vecScale.x);
}

float CFontPatch::GetCharacterSizeDrawing(CharType arg_char)
{
    return GetCharacterSize(arg_char, CFont::RenderState.FontStyle, CFont::RenderState.bFontHalfTexture, CFont::RenderState.bProp, CFont::RenderState.fTextSizeX);
}

float CFontPatch::GetStringWidth(CharType* arg_text, bool bGetAll)
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

CharType* CFontPatch::GetNextSpace(CharType* arg_text)
{
    CharType* temp = arg_text;

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

short CFontPatch::GetNumberLines(float arg_x, float arg_y, CharType* arg_text)
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

void CFontPatch::GetTextRect(CRect* result, float arg_x, float arg_y, CharType* arg_text)
{
    short numLines = GetNumberLines(arg_x, arg_y, arg_text);

    if (CFont::Details.m_bCentre)
    {
        if (CFont::Details.m_bBackGroundOnlyText)
        {
            result->left = arg_x - 4.0f;
            result->right = arg_x + 4.0f;
            result->bottom = (18.0f * CFont::Details.m_vecScale.y) * numLines + arg_y + 2.0f;
            result->top = arg_y - 2.0f;
        }
        else
        {
            result->left = arg_x - (CFont::Details.m_fCentreSize * 0.5f) - 4.0f;
            result->right = arg_x + (CFont::Details.m_fCentreSize * 0.5f) + 4.0f;
            result->bottom = arg_y + (18.0f * CFont::Details.m_vecScale.y * numLines) + 2.0f;
            result->top = arg_y - 2.0f;
        }
    }
    else
    {
        result->left = arg_x - 4.0f;
        result->right = CFont::Details.m_fWrapX;
        result->bottom = arg_y;
        result->top = (18.0f * CFont::Details.m_vecScale.y) * numLines + arg_y + 4.0f;
    }
}

void CFontPatch::PrintString(float arg_x, float arg_y, CharType* arg_text)
{
    CRect textBoxRect;

    float xBound;
    float yBound = arg_y;
    float strWidth, widthLimit;
    float var_38 = 0.0f;
    float print_x;
    float justifyWrap;

    CharType* ptext = arg_text;
    CharType* strHead = arg_text;

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
            var_38 = 0.0f;
            numSpaces = 0;
            emptyLine = true;
        }
    }
}

void CFontPatch::RenderFontBuffer()
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
    var_14 = FontBuffer.pdata->color;

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
            pos.y = (CFont::RenderState.fSlantRefPointX - pos.x) * CFont::RenderState.fSlant + CFont::RenderState.fSlantRefPointY;
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

        RwRenderStateSet(RwRenderState::rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void*>(1));

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

void CFontPatch::PrintCHSChar(float arg_x, float arg_y, CharType arg_char)
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

    if (arg_x >= RsGlobal.maximumWidth ||
        arg_x <= 0.0f ||
        arg_y <= 0.0f ||
        arg_y >= RsGlobal.maximumHeight)
    {
        return;
    }

    pos = CCharTable::GetCharPos(arg_char);

    yOffset = CFont::RenderState.fTextSizeY * 2.0f;

    if (CFont::RenderState.fSlant == 0.0f)
    {
        rect.left = arg_x;
        rect.top = arg_y + yOffset;
        rect.right = CFont::RenderState.fTextSizeX * 32.0f + arg_x;
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
        rect.left = arg_x;
        rect.top = arg_y + 0.015f + yOffset;
        rect.right = CFont::RenderState.fTextSizeX * 32.0f + arg_x;
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

void CFontPatch::PrintCharDispatcher(float arg_x, float arg_y, CharType arg_char)
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

void CFontPatch::DisableSlant(float slant)
{
    CFont::Details.m_fSlant = 0.0f;
}
