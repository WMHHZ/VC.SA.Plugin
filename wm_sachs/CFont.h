#pragma once
#include "game.h"
#include "../deps/func_wrapper/func_wrapper.hpp"

class CSprite2d;

struct LetterWidths
{
	unsigned __int8 PropValues[209];
	unsigned __int8 UnpropValue;
};

class CFontRenderState
{
public:
	float		Useless;
	CVector2D	Pos;
	CVector2D	Scale;
	CRGBA		Color;
	float		JustifyWrap;
	float		Slant;
	CVector2D	SlantRefPoint;
	bool		IsBlip;
	__int8		FontStyle;
	bool		Prop;
	__int8		_pad1;
	__int16		TextureID;
	__int8		OutlineSize;
	__int8		_pad2;
};
static_assert(sizeof(CFontRenderState) == 0x30, "Class CFontRenderState is wrong.");

class CFontDetails
{
	CRGBA Color;
	CVector2D Scale;
	float Slant;
	CVector2D SlantRefPoint;
	bool Justify;
	bool Centre;
	bool RightJustify;
	bool Background;
	bool EnlargeBackground;
	bool Prop;
	bool IsBlip;
	__int8 pad1;
	float Alpha;
	CRGBA BackgroundColor;
	float WrapX;
	float CentreSize;
	float RightJustifyWrap;
	__int8 TextureID;
	__int8 FontStyle;
	__int8 Shadow;
	CRGBA DropColor;
	__int8 OutlineSize;
	__int8 pad2[3];
};
static_assert(sizeof(CFontDetails) == 0x40, "Struct CFontDetails is wrong.");

union FontBufferPointer
{
	CFontRenderState *pdata;
	const char *ptext;
	unsigned __int32 addr;
};
static_assert(sizeof(FontBufferPointer) == 0x4, "Union FontBufferPointer is wrong.");

class CFont
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

	static CSprite2d *m_Sprites;
	static CSprite2d m_ChsSprite;
	static CSprite2d *m_ButtonSprites;

	static LetterWidths *m_FontWidths;

	static CFontRenderState *m_FontBuffer;
	static FontBufferPointer *m_FontBufferIter;
	static CFontRenderState *RenderState;

	static unsigned __int8 *m_nPS2SymbolId;
	static bool *m_bNewLine;
	static CRGBA *m_Color;
	static CVector2D *m_fScale;
	static bool *m_bFontJustify;
	static bool *m_bFontCentreAlign;
	static bool *m_bFontRightAlign;
	static bool *m_bFontPropOn;
	static float *m_fWrapx;
	static float *m_fFontCentreSize;
	static float *m_fRightJustifyWrap;
	static unsigned __int8 *m_nFontTextureID;
	static unsigned __int8 *m_nFontStyle;
	static __int8 *m_nFontOutlineSize;

	static unsigned char(__cdecl *FindSubFontCharacter)(unsigned char, unsigned char);
	static const char *(__cdecl *ParseToken)(const char *, CRGBA &, bool, char *);
	static void(__cdecl *PrintKeyTokenFormat)(char *);
	static void(__cdecl *RenderString)(float, float, const char *, const char *, float);
	static void(__cdecl *SetColor)(CRGBA);

	static float GetScaledLetterWidthNormal(unsigned __int32);
	static float GetScaledLetterWidthScript(unsigned __int32);
	static float GetScaledLetterWidthDrawing(unsigned __int32);

	static float GetStringWidth(const char *, bool, bool);
	static const char *GetNextSpace(const char *);

	static short ProcessCurrentString(bool, float, float, const char *);

	static void RenderFontBuffer();

	static void PrintCHSChar(float, float, unsigned __int32);

	CFont();
};
