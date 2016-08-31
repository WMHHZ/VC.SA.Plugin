#include "CFont.h"
#include "CSprite2d.h"
#include "CTimer.h"
#include "rwFunc.h"
#include "CTxdStore.h"
#include "../deps/selector/AddressSelector.h"
#include "CFreeType.h"

char CFont::fontPath[MAX_PATH];
char CFont::textPath[MAX_PATH];

CFontSizes *CFont::Size;
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

void CFont::LoadCHSFont()
{
	CFreeType::Init();
}

void CFont::UnloadCHSFont(int dummy)
{
	CTxdStore::fpRemoveTxdSlot(dummy);
	CFreeType::Close();
}

float CFont::GetCharacterSize(CharType arg_char, __int16 nFontStyle, bool bBaseCharset, bool bProp, float fScaleX)
{
	__int16 charWidth;

	if (arg_char >= 0x80)
	{
		return ((CFreeType::GetCharInfo(arg_char).width * 0.8f + 3) * fScaleX);
	}
	else
	{
		if (bBaseCharset)
		{
			arg_char = fpFindNewCharacter(arg_char);
		}

		if (bProp)
		{
			charWidth = Size[nFontStyle].PropValues[arg_char - 0x20];
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

	while (*arg_text != '\0')
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
			if (result == 0.0f || bGetAll)
			{
				do
				{
					++arg_text;
				} while (*arg_text != '~');
			}
			else
			{
				break;
			}
		}
		else if (*arg_text < 0x80)
		{
			result += GetCharacterSizeNormal(*arg_text);
		}
		else
		{
			if (result == 0.0f || bGetAll)
			{
				result += GetCharacterSizeNormal(*arg_text);
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

CharType *CFont::GetNextSpace(CharType *arg_text)
{
	CharType *temp = arg_text;

	while (*temp != ' ' && *temp != '\0')
	{
		if (*temp == '~')
		{
			if (temp == arg_text)
			{
				do
				{
					++temp;
				} while (*temp != '~');

				++temp;
				arg_text = temp;
				continue;
			}
			else
			{
				break;
			}
		}
		else if (*temp >= 0x80)
		{
			if (temp == arg_text)
			{
				++temp;
			}

			break;
		}

		++temp;
	}

	return temp;
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

			*RenderState = *(pbuffer.pdata);

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

		var_char = *pbuffer.ptext;

		CSprite2d::fpSetRenderState(&Sprite[RenderState->FontStyle]);

		rwFunc::fpRwRenderStateSet(RwRenderState::rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);

		PrintCharDispatcher(pos.x, pos.y, var_char);

		if (var_D)
		{
			PrintCharDispatcher(pos.x + 1.0f, pos.y, var_char);
			PrintCharDispatcher(pos.x + 2.0f, pos.y, var_char);
			pos.x += 2.0f;
		}

		CSprite2d::fpRenderVertexBuffer();

		pos.x += GetCharacterSizeDrawing(var_char);
	
		if (var_char == 0)
		{
			pos.x += RenderState->JustifyWrap;
		}
	
		++pbuffer.ptext;
	}	

	FontBufferIter->addr = FontBuffer.addr;
}

void CFont::PrintCHSChar(float arg_x, float arg_y, CharType arg_char)
{
	CRect rect;

	auto &info = CFreeType::GetCharInfo(arg_char);

	if (arg_x >= *rwFunc::RsGlobalW ||
		arg_x <= 0.0f ||
		arg_y <= 0.0f ||
		arg_y >= *rwFunc::RsGlobalH)
	{
		return;
	}

	rect.x1 = arg_x;
	rect.y2 = arg_y;
	rect.x2 = CFont::RenderState->Scale.x * info.bitmap_width + arg_x;
	rect.y1 = CFont::RenderState->Scale.y * info.bitmap_height / 2 + arg_y;

	CSprite2d::fpDraw(&info.sprite, rect, CFont::RenderState->Color);
	return;
}

void CFont::PrintCharDispatcher(float arg_x, float arg_y, CharType arg_char)
{
	if (arg_char < 0x80)
	{
		arg_char -= 0x20;

		if (RenderState->BaseCharset)
		{
			arg_char = fpFindNewCharacter(arg_char);
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
	Size = AddressSelectorVC::SelectAddress<0x696BD8, 0x0, 0x695BE0, CFontSizes>();
	FontBufferIter = AddressSelectorVC::SelectAddress<0x70975C, 0x0, 0x70875C, FontBufferPointer>();
	FontBuffer.pdata = AddressSelectorVC::SelectAddress<0x70935C, 0x0, 0x70835C, CFontRenderState>();
	RenderState = AddressSelectorVC::SelectAddress<0x94B8F8, 0x0, 0x94A900, CFontRenderState>();
	Sprite = AddressSelectorVC::SelectAddress<0xA108B4, 0x0, 0xA0F8BC, CSprite2d>();
	Details = AddressSelectorVC::SelectAddress<0x97F820, 0x0, 0x97E828, CFontDetails>();
	
	fpFindNewCharacter = AddressSelectorVC::SelectAddress<0x54FE70, 0x0, 0x54FD60>();
	fpParseTokenEPt = AddressSelectorVC::SelectAddress<0x5502D0, 0x0, 0x5501C0>();
	fpParseTokenEPtR5CRGBARbRb = AddressSelectorVC::SelectAddress<0x550510, 0x0, 0x550400>();
	fpPrintStringPart = AddressSelectorVC::SelectAddress<0x5516C0, 0x0, 0x5515B0>();
	fpPrintChar = AddressSelectorVC::SelectAddress<0x551E70, 0x0, 0x551D60>();
}

static CFont instance;
