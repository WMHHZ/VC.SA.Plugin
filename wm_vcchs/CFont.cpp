#include "CFont.h"
#include "CSprite2d.h"
#include "CTimer.h"
#include "CCharTable.h"
#include "rwFunc.h"
#include "CTxdStore.h"
#include "../include/hooking/Hooking.Patterns.h"

char CFont::texturePath[MAX_PATH];
char CFont::textPath[MAX_PATH];

CharacterSize *CFont::Size;
FontBufferPointer CFont::FontBuffer;
FontBufferPointer *CFont::FontBufferIter;
CFontRenderState *CFont::RenderState;
CSprite2d *CFont::Sprite;
CFontDetails *CFont::Details;

cdecl_func_wrapper<CharType(CharType arg_char)>
CFont::fpFindNewCharacter;

cdecl_func_wrapper<CharType *(CharType *)>
CFont::fpParseTokenEPt;

cdecl_func_wrapper<CharType *(CharType *arg_text, CRGBA &result_color, bool &result_blip, bool &result_bold)>
CFont::fpParseTokenEPtR5CRGBARbRb;

cdecl_func_wrapper<void(float arg_x, float arg_y, CharType arg_char)>
CFont::fpPrintChar;

cdecl_func_wrapper<void(float arg_x, float arg_y, unsigned int useless, CharType *arg_strbeg, CharType *arg_strend, float justifywrap)>
CFont::fpPrintStringPart;

void CFont::LoadCHSTexture()
{
	CTxdStore::fpPopCurrentTxd();

	CFont::Sprite[2].m_pRwTexture = rwFunc::LoadTextureFromPNG(texturePath);
}

void CFont::UnloadCHSTexture(int dummy)
{
	CTxdStore::fpRemoveTxdSlot(dummy);

	CSprite2d::fpDelete(&CFont::Sprite[2]);
}

float CFont::GetCharacterSize(CharType arg_char, __int16 nFontStyle, bool bBaseCharset, bool bProp, float fScaleX)
{
	__int16 charWidth;

	if (arg_char >= 0x60)
	{
		switch (arg_char + 0x20)
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
		if (bBaseCharset)
		{
			arg_char = fpFindNewCharacter(arg_char);
		}

		if (bProp)
		{
			charWidth = Size[nFontStyle].PropValues[arg_char];
		}
		else
		{
			charWidth = Size[nFontStyle].UnpropValue;
		}
	}

	return (charWidth * fScaleX);
}

float CFont::GetCharacterSizeNormal(CharType arg_char)
{
	return GetCharacterSize(arg_char, Details->FontStyle, Details->BaseCharset, Details->Prop, Details->Scale.x);
}

float CFont::GetCharacterSizeDrawing(CharType arg_char)
{
	return GetCharacterSize(arg_char, RenderState->FontStyle, RenderState->BaseCharset, RenderState->Prop, RenderState->Scale.x);
}

float CFont::GetStringWidth(CharType *arg_text, bool bGetAll)
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

CharType *CFont::GetNextSpace(CharType *arg_pointer)
{
	CharType *var_pointer = arg_pointer;

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

__int16 CFont::GetNumberLines(float arg_x, float arg_y, CharType *arg_text)
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
			yBound += Details->Scale.y * 18.0f;
		}
	}

	return result;
}

void CFont::GetTextRect(CRect *result, float arg_x, float arg_y, CharType *arg_text)
{
	__int16 numLines = GetNumberLines(arg_x, arg_y, arg_text);

	if (Details->Centre)
	{
		if (Details->BackGroundOnlyText)
		{
			result->x1 = arg_x - 4.0f;
			result->x2 = arg_x + 4.0f;
			result->y1 = (18.0f * Details->Scale.y) * numLines + arg_y + 2.0f;
			result->y2 = arg_y - 2.0f;
		}
		else
		{
			result->x1 = arg_x - (Details->CentreSize * 0.5f) - 4.0f;
			result->x2 = arg_x + (Details->CentreSize * 0.5f) + 4.0f;
			result->y1 = arg_y + (18.0f * Details->Scale.y * numLines) + 2.0f;
			result->y2 = arg_y - 2.0f;
		}
	}
	else
	{
		result->x1 = arg_x - 4.0f;
		result->x2 = Details->WrapX;
		result->y1 = arg_y;
		result->y2 = (18.0f * Details->Scale.y) * numLines + arg_y + 4.0f;
	}
}

void CFont::PrintString(float arg_x, float arg_y, CharType *arg_text)
{
	CRect textBoxRect;

	float xBound;
	float yBound = arg_y;
	float strWidth, widthLimit;
	float var_38 = 0.0f;
	float print_x;
	float justifyWrap;

	CharType *ptext = arg_text;
	CharType *strHead = arg_text;

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
		CSprite2d::fpDrawRect(textBoxRect, Details->BackgroundColor);
	}

	if (Details->Centre || Details->RightJustify)
	{
		xBound = 0.0f;
	}
	else
	{
		xBound = arg_x;
	}

	while (*ptext != 0)
	{
		strWidth = GetStringWidth(ptext, false);

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
			ptext = GetNextSpace(ptext);
			xBound += strWidth;

			if (*ptext != 0)
			{
				if (*ptext == ' ')
				{
					if (*(ptext + 1) == 0)
					{
						*ptext = 0;
					}
					else
					{
						if (!emptyLine)
						{
							++numSpaces;
						}

						xBound += GetCharacterSizeNormal(0);
						++ptext;
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

				fpPrintStringPart(print_x, yBound, 0, strHead, ptext, 0.0f);
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

			fpPrintStringPart(print_x, yBound, 0, strHead, ptext, justifyWrap);
		
			strHead = ptext;

			if (Details->Centre || Details->RightJustify)
			{
				xBound = 0.0f;
			}
			else
			{
				xBound = arg_x;
			}

			yBound += Details->Scale.y * 18.0f;
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

	CharType var_char;

	FontBufferPointer pbuffer;

	if (FontBufferIter->addr == FontBuffer.addr)
	{
		return;
	}

	*RenderState = *(FontBuffer.pdata);
	var_14 = FontBuffer.pdata->Color;

	pos = RenderState->Pos;

	pbuffer.addr = FontBuffer.addr + 0x30;

	while (pbuffer.addr < FontBufferIter->addr)
	{
		if (*(pbuffer.ptext) == 0)
		{
			++pbuffer.ptext;

			if (((pbuffer.addr) & 3) != 0)
			{
				++pbuffer.ptext;
			}

			if (pbuffer.addr >= FontBufferIter->addr)
			{
				break;
			}

			*RenderState = *pbuffer.pdata;

			var_14 = RenderState->Color;

			pos = RenderState->Pos;

			pbuffer.addr += 0x30;
		}

		if (*pbuffer.ptext == '~')
		{
			pbuffer.ptext = fpParseTokenEPtR5CRGBARbRb(pbuffer.ptext, var_14, var_E, var_D);

			if (var_E)
			{
				if ((*CTimer::m_nTimeInMilliseconds - Details->BlipStartTime) > 300)
				{
					Details->IsBlip = true;
					Details->BlipStartTime = *CTimer::m_nTimeInMilliseconds;
				}

				if (Details->IsBlip)
				{
					Details->Color.alpha = 0;
				}
				else
				{
					Details->Color.alpha = 255;
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

		var_char = *pbuffer.ptext - 0x20;

		PrintCharDispatcher(pos.x, pos.y, var_char);

		if (var_D)
		{
			PrintCharDispatcher(pos.x + 1.0f, pos.y, var_char);
			PrintCharDispatcher(pos.x + 2.0f, pos.y, var_char);
			pos.x += 2.0f;
		}

		pos.x += GetCharacterSizeDrawing(var_char);
	
		if (var_char == 0)
		{
			pos.x += RenderState->JustifyWrap;
		}
	
		++pbuffer.ptext;
	}

	CSprite2d::fpSetRenderState(&Sprite[RenderState->FontStyle]);
	rwFunc::fpRwRenderStateSet(RwRenderState::rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);
	CSprite2d::fpRenderVertexBuffer();

	FontBufferIter->addr = FontBuffer.addr;
}

void CFont::PrintCHSChar(float arg_x, float arg_y, CharType arg_char)
{
	static const float rRowsCount = 1.0f / 51.2f;
	static const float rColumnsCount = 1.0f / 64.0f;

	static const float Fix_697200_CHS = 0.0021f / 4.0f;
	static const float Fix_6971FC_CHS = 0.001f / 4.0f;
	static const float Fix_697208_CHS = 0.015f / 4.0f;
	static const float Fix_69720C_CHS = 0.01f / 4.0f;
	static const float Fix_697210_CHS = 0.009f / 4.0f;
	static const float Fix_697214_CHS = 0.00055f / 4.0f;

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
			rect.x2 = RenderState->Scale.x * 32.0f + arg_x;
			rect.y1 = RenderState->Scale.y * 20.0f + arg_y;

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
			rect.x2 = RenderState->Scale.x * 32.0f + arg_x;
			rect.y1 = RenderState->Scale.y * 20.0f + arg_y;

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
			rect.x2 = RenderState->Scale.x * 32.0f + arg_x;
			rect.y1 = RenderState->Scale.y * 20.0f + arg_y;

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
			rect.x2 = RenderState->Scale.x * 32.0f + arg_x;
			rect.y1 = RenderState->Scale.y * 20.0f + arg_y;

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

void CFont::PrintCharDispatcher(float arg_x, float arg_y, CharType arg_char)
{
	if (arg_char < 0x60)
	{
		if (RenderState->BaseCharset)
		{
			arg_char = fpFindNewCharacter(arg_char);

			if (arg_char == 0xD0)
			{
				arg_char = 0;
			}
		}

		fpPrintChar(arg_x, arg_y, arg_char);
	}
	else
	{
		PrintCHSChar(arg_x, arg_y, arg_char);
	}
}

CFont::CFont()
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
	fpPrintChar = hook::pattern("53 56 55 8B 2D ? ? ? ? 83 EC 48").get(0).get();
}

static CFont instance;
