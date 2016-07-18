#pragma once
#include "game.h"
#include "../include/utf8cpp/utf8.h"

class CSprite2d;

struct LetterWidths
{
	uint8_t PropValues[209];
	uint8_t UnpropValue;
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
	int8_t		FontStyle;
	bool		Prop;
	int8_t		_pad1;
	int16_t		TextureID;
	int8_t		OutlineSize;
	int8_t		_pad2;
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
	int8_t pad1;
	float Alpha;
	CRGBA BackgroundColor;
	float WrapX;
	float CentreSize;
	float RightJustifyWrap;
	int8_t TextureID;
	int8_t FontStyle;
	int8_t Shadow;
	CRGBA DropColor;
	int8_t OutlineSize;
	int8_t pad2[3];
};
static_assert(sizeof(CFontDetails) == 0x40, "Struct CFontDetails is wrong.");

union FontBufferPointer
{
	CFontRenderState *pdata;
	const char *ptext;
	uintptr_t addr;
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

	static float GetScaledLetterWidthNormal(uint32_t);
	static float GetScaledLetterWidthScript(uint32_t);
	static float GetScaledLetterWidthDrawing(uint32_t);

	static float GetStringWidth(const char *, bool, bool);
	static const char *GetNextSpace(const char *);

	static short ProcessCurrentString(bool, float, float, const char *);

	static void RenderFontBuffer();

	static void PrintChar(float, float, uint32_t);

	static void Init10U();
	static void InitSteam();


};
