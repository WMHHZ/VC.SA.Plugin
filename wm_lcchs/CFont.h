#pragma once
#include "game.h"
#include "CSprite2d.h"

class CFontSizes
{
public:
	__int16 PropValues[192];
	__int16 UnpropValue;
};
static_assert(sizeof(CFontSizes) == 0x182, "Class CFontSize is wrong.");

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
	__int8 pad1[2];
	float AlphaFade;
	CRGBA BackgroundColor;
	float WrapX;
	float CentreSize;
	float RightJustifyWrap;
	__int16 FontStyle;
	__int8 pad2[2];
	__int32 unk; //Related to Bank
	__int16 DropShadowPos;
	CRGBA DropColor;
	__int8 pad3[2];
};
static_assert(sizeof(CFontDetails) == 0x44, "Class CFontDetails is wrong.");

class CFont
{
public:
	static char texturePath[];
	static char textPath[];

	static CFontSizes *Size;
	static CFontDetails *Details;
	static CSprite2d ChsSprite;
	static CSprite2d ChsSlantSprite;

	static float __cdecl GetCharacterSize(unsigned __int16 arg_char);

	static float __cdecl GetStringWidth(unsigned __int16 *arg_text, bool bGetAll);
	static unsigned __int16 *__cdecl GetNextSpace(unsigned __int16 *arg_text);

	static __int16 __cdecl GetNumberLines(float arg_x, float arg_y, unsigned __int16 *arg_text);
	static void __cdecl GetTextRect(CRect *result, float arg_x, float arg_y, unsigned __int16 *arg_text);

	static unsigned __int16 * __cdecl ParseToken(unsigned __int16 *arg_text, unsigned __int16 *useless);

	static void PrintChar(float arg_x, float arg_y, unsigned __int16 arg_char);
	static void PrintCHSChar(float arg_x, float arg_y, unsigned __int16 arg_char);
	static void __cdecl PrintCharDispatcher(float arg_x, float arg_y, unsigned __int16 arg_char);

	static void __cdecl LoadCHSTexture();
	static void __cdecl UnloadCHSTexture(int dummy);

	CFont();

private:
	static void *fpPrintChar;
	static void *fpParseToken;
};
