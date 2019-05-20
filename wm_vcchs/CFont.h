#pragma once

#include "stdinc.h"
#include "CSprite2d.h"

typedef unsigned short CharType;

struct CFontSizes
{
    short PropValues[209];
    short UnpropValue;
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
VALIDATE_SIZE(CFontRenderState, 0x30)

struct FontBufferEntry
{
    CFontRenderState data;
    CharType text[1];
};

union FontBufferPointer
{
    CFontRenderState *pdata;
    CharType *ptext;
    std::uintptr_t addr;
};
VALIDATE_SIZE(FontBufferPointer, 4)

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
VALIDATE_SIZE(CFontDetails, 0x54)

class CFont
{
public:
    static const short iMaxCharWidth;
    static const float fMaxCharWidth;

    static char fontPath[];
    static char textPath[];

    static CFontSizes *Size;

    static FontBufferPointer FontBuffer;
    static FontBufferPointer *FontBufferIter;
    static CFontRenderState *RenderState;

    static CSprite2d *Sprite;
    static CSprite2d ChsSprite;
    static CSprite2d ChsSlantSprite;

    static CFontDetails *Details;

    static injector::hook_back<CharType(*)(CharType arg_char)>
        fpFindNewCharacter;

    static injector::hook_back<CharType *(*)(CharType *)>
        fpParseTokenEPt;

    static injector::hook_back<CharType *(*)(CharType *arg_text, CRGBA &result_color, bool &result_blip, bool &result_bold)>
        fpParseTokenEPtR5CRGBARbRb;

    static injector::hook_back<void(*)(float arg_x, float arg_y, CharType arg_char)>
        fpPrintChar;

    static injector::hook_back<void(*)(float arg_x, float arg_y, unsigned int useless, CharType *arg_strbeg, CharType *arg_strend, float justifywrap)>
        fpPrintStringPart;

    static float GetCharacterSize(CharType arg_char, short nFontStyle, bool bBaseCharset, bool bProp, float fScaleX);
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
