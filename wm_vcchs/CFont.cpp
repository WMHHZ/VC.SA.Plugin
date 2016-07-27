#include "CFont.h"
#include "CSprite2d.h"
#include "CTimer.h"
#include "CCharTable.h"
#include "rwFunc.h"
#include "CTxdStore.h"
#include "../include/injector/calling.hpp"
#include "../include/hooking/Hooking.Patterns.h"

char CFont::texturePath[MAX_PATH];
char CFont::textPath[MAX_PATH];

const CharacterSize *CFont::Size;
FontBufferPointer CFont::FontBuffer;
FontBufferPointer *CFont::FontBufferIter;
CFontRenderState *CFont::RenderState;
CSprite2d *CFont::Sprite;
CFontDetails *CFont::Details;

void *CFont::fpFindNewCharacter;
void *CFont::fpParseTokenEPt;
void *CFont::fpParseTokenEPtR5CRGBARbRb;
void *CFont::fpPrintStringPart;

void CFont::LoadCHSTexture()
{
	CTxdStore::PopCurrentTxd();

	CFont::Sprite[2].m_pRwTexture = rwFunc::LoadTextureFromPNG(texturePath);
}

void CFont::UnloadCHSTexture(int dummy)
{
	CTxdStore::RemoveTxdSlot(dummy);

	CFont::Sprite[2].Delete();
}


unsigned __int16 CFont::FindNewCharacter(unsigned __int16 arg_char)
{
	return injector::cstd<unsigned __int16(unsigned __int16)>::call(fpFindNewCharacter, arg_char);
}

unsigned __int16 *CFont::ParseToken(unsigned __int16 *arg_text)
{
	return injector::cstd<unsigned __int16 *(unsigned __int16 *)>::call(fpParseTokenEPt, arg_text);
}

unsigned __int16 *CFont::ParseToken(unsigned __int16 *arg_text, CRGBA &result_color, bool &result_bool1, bool &result_bool2)
{
	return injector::cstd<unsigned __int16 *(unsigned __int16 *, CRGBA &, bool &, bool &)>::call(fpParseTokenEPtR5CRGBARbRb, arg_text, result_color, result_bool1, result_bool2);
}

void CFont::PrintStringPart(float arg_x, float arg_y, unsigned int useless, unsigned __int16 *arg_strbeg, unsigned __int16 *arg_strend, float justifywrap)
{
	return injector::cstd<void(float, float, unsigned int, unsigned __int16 *, unsigned __int16 *, float)>::call(fpPrintStringPart, arg_x, arg_y, useless, arg_strbeg, arg_strend, justifywrap);
}

float CFont::GetCharacterSizeNormal(unsigned __int16 arg_letter)
{
	__int16 charWidth;

	if (arg_letter >= 0x60)
	{
		switch (arg_letter + 0x20)
		{
		case L'·':
			charWidth = 24;
			break;
		case L'—':
			charWidth = 32;
			break;
		case L'“':
			charWidth = 16;
			break;
		case L'”':
			charWidth = 16;
			break;
		case L'…':
			charWidth = 24;
			break;
		case L'←':
			charWidth = 24;
			break;
		case L'→':
			charWidth = 24;
			break;
		case L'。':
			charWidth = 14;
			break;
		case L'！':
			charWidth = 10;
			break;
		case L'（':
			charWidth = 16;
			break;
		case L'）':
			charWidth = 16;
			break;
		case L'，':
			charWidth = 12;
			break;
		case L'／':
			charWidth = 18;
			break;
		case L'：':
			charWidth = 10;
			break;
		case L'？':
			charWidth = 18;
			break;

		default:
			charWidth = 26;
			break;
		}
	}
	else
	{
		if (Details->BaseCharset)
		{
			arg_letter = FindNewCharacter(arg_letter);
		}

		if (Details->Prop)
		{
			charWidth = Size[Details->FontStyle].PropValues[arg_letter];
		}
		else
		{
			charWidth = Size[Details->FontStyle].UnpropValue;
		}
	}

	return (charWidth * Details->LetterSize.x);
}

float CFont::GetCharacterSizeDrawing(unsigned __int16 arg_letter)
{
	__int16 charWidth;

	if (arg_letter >= 0x60)
	{
		switch (arg_letter + 0x20)
		{
		case L'·':
			charWidth = 24;
			break;
		case L'—':
			charWidth = 32;
			break;
		case L'“':
			charWidth = 16;
			break;
		case L'”':
			charWidth = 16;
			break;
		case L'…':
			charWidth = 24;
			break;
		case L'←':
			charWidth = 24;
			break;
		case L'→':
			charWidth = 24;
			break;
		case L'。':
			charWidth = 14;
			break;
		case L'！':
			charWidth = 10;
			break;
		case L'（':
			charWidth = 16;
			break;
		case L'）':
			charWidth = 16;
			break;
		case L'，':
			charWidth = 12;
			break;
		case L'／':
			charWidth = 18;
			break;
		case L'：':
			charWidth = 10;
			break;
		case L'？':
			charWidth = 18;
			break;

		default:
			charWidth = 26;
			break;
		}
	}
	else
	{
		if (RenderState->BaseCharset)
		{
			arg_letter = FindNewCharacter(arg_letter);
		}

		if (RenderState->Prop)
		{
			charWidth = Size[RenderState->FontStyle].PropValues[arg_letter];
		}
		else
		{
			charWidth = Size[RenderState->FontStyle].UnpropValue;
		}
	}

	return (charWidth * RenderState->LetterSize.x);
}


float CFont::GetStringWidth(unsigned __int16 *arg_text, bool bGetAll)
{
	float result = 0.0f;

	while (*arg_text != 0)
	{
		if (*arg_text == ' ')
		{
			if (bGetAll)
			{
				result += GetCharacterSizeNormal(0);
			}
			else
			{
				break;
			}
		}
		else if (*arg_text == '~')
		{
			do
			{
				++arg_text;
			} while (*arg_text != '~');
		}
		else if (*arg_text < 0x80)
		{
			result += GetCharacterSizeNormal(*arg_text - 0x20);
		}
		else
		{
			if (result == 0.0f || bGetAll)
			{
				result += GetCharacterSizeNormal(*arg_text - 0x20);
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

unsigned __int16 *CFont::GetNextSpace(unsigned __int16 *arg_pointer)
{
	unsigned __int16 *var_pointer = arg_pointer;

	bool succeeded = false;

	while (*var_pointer != ' ' && *var_pointer != 0)
	{
		if (*var_pointer == '~')
		{
			do 
			{
				++var_pointer;
			} while (*var_pointer != '~');
		}
		else if (*var_pointer < 0x80)
		{
			succeeded = true;
		}
		else
		{
			if (var_pointer == arg_pointer || !succeeded)
			{
				++var_pointer;
			}

			break;
		}

		++var_pointer;
	}

	return var_pointer;
}

__int16 CFont::GetNumberLines(float arg_x, float arg_y, unsigned __int16 *arg_text)
{
	__int16 result = 0;
	float xBound;
	float yBound = arg_y;
	float strWidth, widthLimit;

	if (Details->Centre || Details->RightJustify)
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

		if (Details->Centre)
		{
			widthLimit = Details->CentreSize;
		}
		else
		{
			widthLimit = Details->WrapX;
		}

		if ((xBound + strWidth) <= widthLimit)
		{
			xBound += strWidth;
			arg_text = GetNextSpace(arg_text);

			if (*arg_text == ' ')
			{
				xBound += GetCharacterSizeNormal(0);
				++arg_text;
			}
			else if (*arg_text == 0)
			{
				++result;
			}
		}
		else
		{
			if (Details->Centre || Details->RightJustify)
			{
				xBound = 0.0f;
			}
			else
			{
				xBound = arg_x;
			}

			++result;
			yBound += Details->LetterSize.y * 18.0f;
		}
	}

	return result;
}

void CFont::GetTextRect(CRect *result, float arg_x, float arg_y, unsigned __int16 *arg_text)
{
	__int16 numLines = GetNumberLines(arg_x, arg_y, arg_text);

	if (Details->Centre)
	{
		if (Details->BackGroundOnlyText)
		{
			result->x1 = arg_x - 4.0f;
			result->x2 = arg_x + 4.0f;
			result->y1 = (18.0f * Details->LetterSize.y) * numLines + arg_y + 2.0f;
			result->y2 = arg_y - 2.0f;
		}
		else
		{
			result->x1 = arg_x - (Details->CentreSize * 0.5f) - 4.0f;
			result->x2 = arg_x + (Details->CentreSize * 0.5f) + 4.0f;
			result->y1 = arg_y + (18.0f * Details->LetterSize.y * numLines) + 2.0f;
			result->y2 = arg_y - 2.0f;
		}
	}
	else
	{
		result->x1 = arg_x - 4.0f;
		result->x2 = Details->WrapX;
		result->y1 = arg_y;
		result->y2 = (18.0f * Details->LetterSize.y) * numLines + arg_y + 4.0f;
	}
}

void CFont::PrintString(float arg_x, float arg_y, unsigned __int16 *arg_text)
{
	CRect textBoxRect;

	float xBound;
	float yBound = arg_y;
	float strWidth, widthLimit;
	float var_38 = 0.0f;
	float print_x;
	float justifyWrap;

	unsigned __int16 *esi = arg_text;
	unsigned __int16 *strHead = arg_text;

	bool emptyLine = true;

	__int16 numSpaces = 0;

	Details->UselessFlag1 = false;

	if (*arg_text == '*')
	{
		return;
	}

	++Details->TextCount;

	if (Details->Background)
	{
		GetTextRect(&textBoxRect, arg_x, arg_y, arg_text);
		CSprite2d::DrawRect(textBoxRect, Details->BackgroundColor);
	}

	if (Details->Centre || Details->RightJustify)
	{
		xBound = 0.0f;
	}
	else
	{
		xBound = arg_x;
	}

	while (*esi != 0)
	{
		strWidth = GetStringWidth(esi, false);

		if (Details->Centre)
		{
			widthLimit = Details->CentreSize;
		}
		else if (Details->RightJustify)
		{
			widthLimit = arg_x - Details->RightJustifyWrap;
		}
		else
		{
			widthLimit = Details->WrapX;
		}

		if (((xBound + strWidth) <= widthLimit) || emptyLine)
		{
			esi = GetNextSpace(esi);
			xBound += strWidth;

			if (*esi != 0)
			{
				if (*esi == ' ')
				{
					if (*(esi + 1) == 0)
					{
						*esi = 0;
					}
					else
					{
						if (!emptyLine)
						{
							++numSpaces;
						}

						xBound += GetCharacterSizeNormal(0);
						++esi;
					}
				}

				emptyLine = false;
				
				var_38 = xBound;
			}
			else
			{
				if (Details->Centre)
				{
					print_x = arg_x - xBound * 0.5f;
				}
				else if (Details->RightJustify)
				{
					print_x = arg_x - xBound;
				}
				else
				{
					print_x = arg_x;
				}

				PrintStringPart(print_x, yBound, 0, strHead, esi, 0.0f);
			}
		}
		else
		{
			if (Details->Justify && !(Details->Centre))
			{
				justifyWrap = (Details->WrapX - var_38) / numSpaces;
			}
			else
			{
				justifyWrap = 0.0f;
			}

			if (Details->Centre)
			{
				print_x = arg_x - xBound * 0.5f;
			}
			else if (Details->RightJustify)
			{
				print_x = arg_x - xBound;
			}
			else
			{
				print_x = arg_x;
			}

			PrintStringPart(print_x, yBound, 0, strHead, esi, justifyWrap);
		
			strHead = esi;

			if (Details->Centre || Details->RightJustify)
			{
				xBound = 0.0f;
			}
			else
			{
				xBound = arg_x;
			}

			yBound += Details->LetterSize.y * 18.0f;
			var_38 = 0.0f;
			numSpaces = 0;
			emptyLine = true;
		}
	}
}

void CFont::RenderFontBuffer()
{
	bool var_D = false;
	bool var_E = false;

	CRGBA var_14;

	CVector2D pos;

	unsigned __int16 bx;

	FontBufferPointer esi;

	if (FontBufferIter->addr == FontBuffer.addr)
	{
		return;
	}

	*RenderState = *(FontBuffer.pdata);
	var_14 = FontBuffer.pdata->Color;

	pos = RenderState->Pos;

	esi.addr = FontBuffer.addr + 0x30;

	while (esi.addr < FontBufferIter->addr)
	{
		if (*(esi.ptext) == 0)
		{
			++esi.ptext;

			if (((esi.addr) & 3) != 0)
			{
				++esi.ptext;
			}

			if (esi.addr >= FontBufferIter->addr)
			{
				break;
			}

			*RenderState = *esi.pdata;

			var_14 = RenderState->Color;

			pos = RenderState->Pos;

			esi.addr += 0x30;
		}

		if (*esi.ptext == '~')
		{
			esi.ptext = ParseToken(esi.ptext, var_14, var_E, var_D);

			if (var_E)
			{
				if ((*CTimer::m_nTimeInMilliseconds - Details->BlipStartTime) > 300)
				{
					Details->IsBlip = true;
					Details->BlipStartTime = *CTimer::m_nTimeInMilliseconds;
				}

				if (Details->IsBlip)
				{
					Details->LetterColor.alpha = 0;
				}
				else
				{
					Details->LetterColor.alpha = 255;
				}
			}

			if (!RenderState->KeepColor)
			{
				RenderState->Color = var_14;
			}
		}

		if (RenderState->Slant != 0.0f)
		{
			pos.y = (RenderState->SlantRefPoint.x - pos.x) * RenderState->Slant + RenderState->SlantRefPoint.y;
		}

		bx = *esi.ptext - 0x20;

		if (bx < 0x60)
		{
			Sprite[RenderState->FontStyle].SetRenderState();
		}
		else
		{
			Sprite[2].SetRenderState();
		}

		rwFunc::RwRenderStateSet(RwRenderState::rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);

		PrintChar(pos.x, pos.y, bx);

		if (var_D)
		{
			PrintChar(pos.x + 1.0f, pos.y, bx);
			PrintChar(pos.x + 2.0f, pos.y, bx);
			pos.x += 2.0f;
		}

		CSprite2d::RenderVertexBuffer();

		pos.x += GetCharacterSizeDrawing(bx);
	
		if (bx == 0)
		{
			pos.x += RenderState->JustifyWrap;
		}
	
		++esi.ptext;
	}

	FontBufferIter->addr = FontBuffer.addr;
}

void CFont::PrintChar(float arg_x, float arg_y, unsigned __int16 arg_char)
{
	static const float RowCountReciprocal = 1.0f / 12.8f ;
	static const float ColumnCountReciprocal = 1.0f / 16.0f;
	static const float Fix_697200 = 0.0021f;
	static const float Fix_6971FC = 0.001f;
	static const float Fix_697208 = 0.015f;
	static const float Fix_69720C = 0.01f;
	static const float Fix_697210 = 0.009f;
	static const float Fix_697214 = 0.00055f;

	static const float ScaleRatio = 4.0f;
	static const float RowCountReciprocal_CHS = 1.0f / 12.8f / ScaleRatio;
	static const float ColumnCountReciprocal_CHS = 1.0f / 16.0f / ScaleRatio;
	static const float Fix_697200_CHS = 0.0021f / ScaleRatio;
	static const float Fix_6971FC_CHS = 0.001f / ScaleRatio;
	static const float Fix_697208_CHS = 0.015f / ScaleRatio;
	static const float Fix_69720C_CHS = 0.01f / ScaleRatio;
	static const float Fix_697210_CHS = 0.009f / ScaleRatio;
	static const float Fix_697214_CHS = 0.00055f / ScaleRatio;

	CRect rect;

	CCharTable::CharPos cpos;

	if (arg_x >= *rwFunc::RsGlobalW ||
		arg_x <= 0.0f ||
		arg_y <= 0.0f ||
		arg_y >= *rwFunc::RsGlobalH)
	{
		return;
	}

	if (arg_char == 0)
	{
		return;
	}

	cpos = CCharTable::GetCharPos(arg_char);

	if (arg_char < 0x60)
	{
		if (RenderState->Slant == 0.0f)
		{
			rect.x1 = arg_x;
			rect.y2 = arg_y;
			rect.x2 = RenderState->LetterSize.x * 32.0f + arg_x;
			rect.y1 = RenderState->LetterSize.y * 20.0f + arg_y;

			CSprite2d::AddToBuffer(rect, RenderState->Color,
				cpos.columnIndex * ColumnCountReciprocal,
				Fix_697200 + cpos.rowIndex * RowCountReciprocal,
				(cpos.columnIndex + 1) * ColumnCountReciprocal - Fix_6971FC,
				Fix_697200 + cpos.rowIndex * RowCountReciprocal,
				cpos.columnIndex * ColumnCountReciprocal,
				(cpos.rowIndex + 1) * RowCountReciprocal - Fix_697200,
				(cpos.columnIndex + 1) * ColumnCountReciprocal - Fix_6971FC,
				(cpos.rowIndex + 1) * RowCountReciprocal - Fix_697200);
		}
		else
		{
			rect.x1 = arg_x;
			rect.y2 = arg_y + 0.015f;
			rect.x2 = RenderState->LetterSize.x * 32.0f + arg_x;
			rect.y1 = RenderState->LetterSize.y * 20.0f + arg_y;

			CSprite2d::AddToBuffer(rect, RenderState->Color,
				cpos.columnIndex * ColumnCountReciprocal,
				cpos.rowIndex * RowCountReciprocal + Fix_697214,
				(cpos.columnIndex + 1) * ColumnCountReciprocal - Fix_6971FC,
				cpos.rowIndex * RowCountReciprocal + Fix_697200 + Fix_69720C,
				cpos.columnIndex * ColumnCountReciprocal,
				(cpos.rowIndex + 1) * RowCountReciprocal - Fix_697210,
				(cpos.columnIndex + 1) * ColumnCountReciprocal - Fix_6971FC,
				(cpos.rowIndex + 1) * RowCountReciprocal + Fix_69720C - Fix_697200);
		}
	}
	else
	{
		if (RenderState->Slant == 0.0f)
		{
			rect.x1 = arg_x;
			rect.y2 = arg_y;
			rect.x2 = RenderState->LetterSize.x * 32.0f + arg_x;
			rect.y1 = RenderState->LetterSize.y * 20.0f + arg_y;

			CSprite2d::AddToBuffer(rect, RenderState->Color,
				cpos.columnIndex * ColumnCountReciprocal_CHS,
				Fix_697200_CHS + cpos.rowIndex * RowCountReciprocal_CHS,
				(cpos.columnIndex + 1) * ColumnCountReciprocal_CHS - Fix_6971FC_CHS,
				Fix_697200_CHS + cpos.rowIndex * RowCountReciprocal_CHS,
				cpos.columnIndex * ColumnCountReciprocal_CHS,
				(cpos.rowIndex + 1) * RowCountReciprocal_CHS - Fix_697200_CHS,
				(cpos.columnIndex + 1) * ColumnCountReciprocal_CHS - Fix_6971FC_CHS,
				(cpos.rowIndex + 1) * RowCountReciprocal_CHS - Fix_697200_CHS);
		}
		else
		{
			rect.x1 = arg_x;
			rect.y2 = arg_y + 0.015f;
			rect.x2 = RenderState->LetterSize.x * 32.0f + arg_x;
			rect.y1 = RenderState->LetterSize.y * 20.0f + arg_y;

			CSprite2d::AddToBuffer(rect, RenderState->Color,
				cpos.columnIndex * ColumnCountReciprocal_CHS,
				cpos.rowIndex * RowCountReciprocal_CHS + Fix_697214_CHS,
				(cpos.columnIndex + 1) * ColumnCountReciprocal_CHS - Fix_6971FC_CHS,
				cpos.rowIndex * RowCountReciprocal_CHS + Fix_697200_CHS + Fix_69720C_CHS,
				cpos.columnIndex * ColumnCountReciprocal_CHS,
				(cpos.rowIndex + 1) * RowCountReciprocal_CHS - Fix_697210_CHS,
				(cpos.columnIndex + 1) * ColumnCountReciprocal_CHS - Fix_6971FC_CHS,
				(cpos.rowIndex + 1) * RowCountReciprocal_CHS + Fix_69720C_CHS - Fix_697200_CHS);
		}
	}
}

void CFont::GetAddresses()
{
	hook::pattern ParseTokenPattern("8B 44 24 04 C6 05 ? ? ? ? 00 83 C0 02");
	hook::pattern FontBufferPattern("81 3D ? ? ? ? ? ? ? ? 75 0E");

	Size = *hook::pattern("0F BF 04 4D ? ? ? ? EB 17").get(0).get<CharacterSize *>(4);
	FontBufferIter = *FontBufferPattern.get(0).get<FontBufferPointer *>(2);
	FontBuffer.pdata = *FontBufferPattern.get(0).get<CFontRenderState *>(6);
	RenderState = *hook::pattern("A3 ? ? ? ? A0 ? ? ? ? D9 1D ? ? ? ?").get(0).get<CFontRenderState *>(1);
	Sprite = *hook::pattern("8D 0C 8D 00 00 00 00 81 C1 ? ? ? ? E8 1D 61 02 00").get(0).get<CSprite2d *>(9);
	Details = *hook::pattern("8A 13 88 15 ? ? ? ?").get(2).get<CFontDetails *>(4);
	
	fpFindNewCharacter = hook::pattern("8B 44 24 04 66 83 F8 10").get(0).get();
	fpParseTokenEPt = ParseTokenPattern.get(0).get();
	fpParseTokenEPtR5CRGBARbRb = ParseTokenPattern.get(1).get();
	fpPrintStringPart = hook::pattern("66 A1 ? ? ? ? 53 66 8B 0D ? ? ? ?").get(0).get();
}
