#pragma once

#include "game.h"
#include <cstdint>

class CSprite2d;

struct CharacterSize
{
	int16_t PropValues[209];
	int16_t UnpropValue;
};

class CFontRenderState
{
public:
	int32_t Useless;
	CVector2D Pos;
	CVector2D LetterSize;
	CRGBA Color;
	float JustifyWrap;
	float Slant;
	CVector2D SlantRefPoint;
	bool KeepColor;
	bool BaseCharset;
	bool Prop;
	int16_t FontStyle;
};
static_assert(sizeof(CFontRenderState) == 0x30, "CFontRenderState is wrong.");

union FontBufferPointer
{
	CFontRenderState *pdata;
	uint16_t *ptext;
	uintptr_t addr;
};
static_assert(sizeof(FontBufferPointer) == 4, "FontBufferPointer is wrong.");

class CFontDetails
{
public:
	CRGBA LetterColor;
	CVector2D LetterSize;
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
	int8_t pad1[3];
	float AlphaFade;
	CRGBA BackgroundColor;
	float WrapX;
	float CentreSize;
	float RightJustifyWrap;
	int16_t FontStyle;
	bool BaseCharset;
	int8_t pad2[5];
	int16_t DropShadowPos;
	CRGBA DropColor;
	bool IsBlip;
	int8_t pad3;
	uint32_t BlipStartTime;
	bool UselessFlag3;
	int8_t pad4[3];
	int32_t TextCount;
};
static_assert(sizeof(CFontDetails) == 0x54, "CFontDetails is wrong.");

class CFont
{
public:
	static const CharacterSize *Size;

	static FontBufferPointer FontBuffer;
	static FontBufferPointer *FontBufferIter;
	static CFontRenderState *RenderState;

	static CSprite2d *Sprite;

	static CFontDetails *Details;

	static uint16_t __cdecl FindNewCharacter(uint16_t arg_char);
	static uint16_t *__cdecl ParseToken(uint16_t *arg_text);
	static uint16_t *__cdecl ParseToken(uint16_t *arg_text, CRGBA &result_color, bool &result_blip, bool &result_bold);
	static void __cdecl PrintStringPart(float arg_x, float arg_y, unsigned int useless, uint16_t *arg_strbeg, uint16_t *arg_strend, float justifywrap);

	static float GetCharacterSizeNormal(uint16_t arg_letter);
	static float GetCharacterSizeDrawing(uint16_t arg_letter);

	static float __cdecl GetStringWidth(uint16_t *arg_text, bool arg_getall);
	static uint16_t *GetNextSpace(uint16_t *arg_pointer);

	static __int16 __cdecl GetNumberLines(float arg_x, float arg_y, uint16_t *arg_text);
	static void __cdecl GetTextRect(CRect *result, float arg_x, float arg_y, uint16_t *arg_text);

	static void __cdecl PrintString(float arg_x, float arg_y, uint16_t *arg_text);
	static void __cdecl RenderFontBuffer();
	static void __cdecl PrintChar(float arg_x, float arg_y, uint16_t arg_char);

	static void GetAddresses();

private:
	static void *fpFindNewCharacter;
	static void *fpParseTokenEPt;
	static void *fpParseTokenEPtR5CRGBARbRb;
	static void *fpPrintStringPart;
};
