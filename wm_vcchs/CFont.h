#pragma once

#include "game.h"

class CSprite2d;

struct CharacterSize
{
	__int16 PropValues[209];
	__int16 UnpropValue;
};

class CFontRenderState
{
public:
	__int32 Useless;
	CVector2D Pos;
	CVector2D LetterSize;
	CRGBA Color;
	float JustifyWrap;
	float Slant;
	CVector2D SlantRefPoint;
	bool KeepColor;
	bool BaseCharset;
	bool Prop;
	__int16 FontStyle;
};
static_assert(sizeof(CFontRenderState) == 0x30, "CFontRenderState is wrong.");

union FontBufferPointer
{
	CFontRenderState *pdata;
	unsigned __int16 *ptext;
	unsigned __int32 addr;
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
	__int8 pad1[3];
	float AlphaFade;
	CRGBA BackgroundColor;
	float WrapX;
	float CentreSize;
	float RightJustifyWrap;
	__int16 FontStyle;
	bool BaseCharset;
	__int8 pad2[5];
	__int16 DropShadowPos;
	CRGBA DropColor;
	bool IsBlip;
	__int8 pad3;
	unsigned __int32 BlipStartTime;
	bool UselessFlag3;
	__int8 pad4[3];
	__int32 TextCount;
};
static_assert(sizeof(CFontDetails) == 0x54, "CFontDetails is wrong.");

class CFont
{
public:
	static char texturePath[];
	static char textPath[];

	static const CharacterSize *Size;

	static FontBufferPointer FontBuffer;
	static FontBufferPointer *FontBufferIter;
	static CFontRenderState *RenderState;

	static CSprite2d *Sprite;

	static CFontDetails *Details;

	static unsigned __int16 __cdecl FindNewCharacter(unsigned __int16 arg_char);
	static unsigned __int16 *__cdecl ParseToken(unsigned __int16 *arg_text);
	static unsigned __int16 *__cdecl ParseToken(unsigned __int16 *arg_text, CRGBA &result_color, bool &result_blip, bool &result_bold);
	static void __cdecl PrintStringPart(float arg_x, float arg_y, unsigned int useless, unsigned __int16 *arg_strbeg, unsigned __int16 *arg_strend, float justifywrap);

	static float GetCharacterSizeNormal(unsigned __int16 arg_letter);
	static float GetCharacterSizeDrawing(unsigned __int16 arg_letter);

	static float __cdecl GetStringWidth(unsigned __int16 *arg_text, bool arg_getall);
	static unsigned __int16 *GetNextSpace(unsigned __int16 *arg_pointer);

	static __int16 __cdecl GetNumberLines(float arg_x, float arg_y, unsigned __int16 *arg_text);
	static void __cdecl GetTextRect(CRect *result, float arg_x, float arg_y, unsigned __int16 *arg_text);

	static void __cdecl PrintString(float arg_x, float arg_y, unsigned __int16 *arg_text);
	static void __cdecl RenderFontBuffer();
	static void __cdecl PrintChar(float arg_x, float arg_y, unsigned __int16 arg_char);

	static void __cdecl LoadCHSTexture();
	static void __cdecl UnloadCHSTexture(int dummy);

	static void GetAddresses();

private:
	static void *fpFindNewCharacter;
	static void *fpParseTokenEPt;
	static void *fpParseTokenEPtR5CRGBARbRb;
	static void *fpPrintStringPart;
};
