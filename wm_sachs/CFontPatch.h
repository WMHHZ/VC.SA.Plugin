#pragma once
#include <CRGBA.h>
#include <game_sa/CTheScripts.h>
#include <game_sa/CVector2D.h>

#include <game_sa/CFont.h>

class CFontRenderState
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
static_assert(sizeof(CFontRenderState) == 0x30, "Class CFontRenderState is wrong.");

union FontBufferPointer {
    CFontRenderState *pdata;
    char             *ptext;
    unsigned int      addr;
};
static_assert(sizeof(FontBufferPointer) == 0x4, "Union FontBufferPointer is wrong.");

class CFontPatch
{
  public:
    static float fix_value_2;
    static float fix_value_2_chs;

    static const float scale_x_rec;
    static const float scale_y_rec;
    static const float scale_x_rec_chs;
    static const float scale_y_rec_chs;
    static const float fix_value_1;
    static const float fix_value_1_chs;

    static const unsigned char SBCLetterWidth = 32;

    static CSprite2d m_ChsSprite;

    static CFontRenderState  *m_FontBuffer;
    static FontBufferPointer *m_FontBufferIter;
    static CFontRenderState  *RenderState;

    static unsigned char(__cdecl *FindSubFontCharacter)(unsigned char, unsigned char);
    static void(__cdecl *RenderString)(float, float, const char *, const char *, float);

    static float GetScaledLetterWidthNormal(unsigned int);
    static float GetScaledLetterWidthScript(unsigned int);
    static float GetScaledLetterWidthDrawing(unsigned int);

    static float GetStringWidth(const char *, bool, bool);
    static char *GetNextSpace(char *);

    static short ProcessCurrentString(bool, float, float, char *);

    static void RenderFontBuffer();

    static void PrintCHSChar(float, float, unsigned int);

    static void Init10U();
};
