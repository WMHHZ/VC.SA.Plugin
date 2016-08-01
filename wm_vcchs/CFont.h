#pragma once

#include "game.h"
#include "../include/func_wrapper/func_wrapper.hpp"

typedef CharType CharType;

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
	CVector2D Scale;
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
	CharType *ptext;
	unsigned __int32 addr;
};
static_assert(sizeof(FontBufferPointer) == 4, "FontBufferPointer is wrong.");

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

	static CharacterSize *Size;

	static FontBufferPointer FontBuffer;
	static FontBufferPointer *FontBufferIter;
	static CFontRenderState *RenderState;

	static CSprite2d *Sprite;

	static CFontDetails *Details;

	static cdecl_func_wrapper<CharType(CharType arg_char)>
		fpFindNewCharacter;

	static cdecl_func_wrapper<CharType *(CharType *)>
		fpParseTokenEPt;

	static cdecl_func_wrapper<CharType *(CharType *arg_text, CRGBA &result_color, bool &result_blip, bool &result_bold)>
		fpParseTokenEPtR5CRGBARbRb;

	static cdecl_func_wrapper<void(float arg_x, float arg_y, CharType arg_char)>
		fpPrintChar;

	static cdecl_func_wrapper<void(float arg_x, float arg_y, unsigned int useless, CharType *arg_strbeg, CharType *arg_strend, float justifywrap)>
		fpPrintStringPart;

	static float GetCharacterSize(CharType arg_char, __int16 nFontStyle, bool bBaseCharset, bool bProp, float fScaleX);
	static float GetCharacterSizeNormal(CharType arg_char);
	static float GetCharacterSizeDrawing(CharType arg_char);

	static float __cdecl GetStringWidth(CharType *arg_text, bool arg_getall);
	static CharType *GetNextSpace(CharType *arg_pointer);

	static __int16 __cdecl GetNumberLines(float arg_x, float arg_y, CharType *arg_text);
	static void __cdecl GetTextRect(CRect *result, float arg_x, float arg_y, CharType *arg_text);

	static void __cdecl PrintString(float arg_x, float arg_y, CharType *arg_text);
	static void __cdecl RenderFontBuffer();
	static void __cdecl PrintCHSChar(float arg_x, float arg_y, CharType arg_char);
	static void __cdecl PrintCharDispatcher(float arg_x, float arg_y, CharType arg_char);

	static void __cdecl LoadCHSTexture();
	static void __cdecl UnloadCHSTexture(int dummy);

	CFont();
};
