#pragma once

#include <game_vc/CFont.h>
#include <game_vc/CFontDetails.h>
#include <game_vc/CSprite2d.h>


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
VALIDATE_SIZE(FontBufferPointer, 4);

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

class CFontPatch
{
  public:
    static const short iMaxCharWidth;
    static const float fMaxCharWidth;

    static char fontPath[];
    static char textPath[];

    static tFontTable *Size;

    static FontBufferPointer  FontBuffer;
    static FontBufferPointer *FontBufferIter;

    static CSprite2d ChsSprite;
    static CSprite2d ChsSlantSprite;

    static float GetCharacterSize(CharType arg_char, short nFontStyle, bool bFontHalfTexture, bool bProp,
                                  float fScaleX);
    static float GetCharacterSizeNormal(CharType arg_char);
    static float GetCharacterSizeDrawing(CharType arg_char);

    static float __cdecl GetStringWidth(CharType *arg_text, bool arg_getall);
    static CharType *GetNextSpace(CharType *arg_text);

    static short __cdecl GetNumberLines(float arg_x, float arg_y, CharType *arg_text);
    static void GetTextRect(CRect *result, float arg_x, float arg_y, CharType *arg_text);

    static void __cdecl PrintString(float arg_x, float arg_y, CharType *arg_text);
    static void __cdecl RenderFontBuffer();
    static void PrintCHSChar(float arg_x, float arg_y, CharType arg_char);
    static void PrintCharDispatcher(float arg_x, float arg_y, CharType arg_char);

    static void __cdecl DisableSlant(float slant);

    static void __cdecl LoadCHSFont();
    static void __cdecl UnloadCHSFont(int dummy);
};
