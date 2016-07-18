#include "CFont.h"
#include "CCharTable.h"
#include "rwFunc.h"
#include "../include/hooking/Hooking.Patterns.h"

CSprite2d CFont::ChsSprite;
CSprite2d CFont::ChsSlantSprite;
void *CFont::fpPrintChar;
void *CFont::fpParseToken;
CFontDetails *CFont::Details;
CFontSizes *CFont::Size;

float CFont::GetCharacterWidth(unsigned __int16 arg_char)
{
	__int16 iCharWidth;

	if (arg_char < 0x60)
	{
		if (Details->Prop)
		{
			iCharWidth = Size[Details->FontStyle].PropValues[arg_char];
		}
		else
		{
			iCharWidth = Size[Details->FontStyle].UnpropValue;
		}
	}
	else
	{
		iCharWidth = 32;
	}

	return iCharWidth;
}

float CFont::GetCharacterSize(unsigned __int16 arg_char)
{
	return (GetCharacterWidth(arg_char) * Details->Scale.x);
}

float CFont::GetStringWidth(unsigned __int16 *arg_text, bool bGetAll)
{
	float result = 0.0f;

	while (true)
	{
		if (*arg_text == 0)
		{
			break;
		}
		else if (*arg_text == '~')
		{
			do
			{
				++arg_text;
			} while (*arg_text != '~');
		}
		else if (*arg_text == ' ')
		{
			if (!bGetAll)
			{
				break;
			}
		}
		else if (*arg_text < 0x80)
		{
			result += GetCharacterSize(*arg_text - 0x20);
		}
		else
		{
			
			if (result == 0.0f || bGetAll)
			{
				result += GetCharacterSize(*arg_text - 0x20);
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

unsigned __int16 *CFont::GetNextSpace(unsigned __int16 *arg_text)
{
	unsigned __int16 *temp = arg_text;

	bool succeeded = false;

	while (*temp != ' ' && *temp != 0)
	{
		if (*temp == '~')
		{
			do
			{
				++temp;
			} while (*temp != '~');
		}
		else if (*temp < 0x80)
		{
			succeeded = true;
		}
		else
		{
			if (temp == arg_text || !succeeded)
			{
				++temp;
			}

			break;
		}

		++temp;
	}

	return temp;
}

__int16 CFont::GetNumberLines(float arg_x, float arg_y, unsigned __int16 *arg_text)
{
	__int16 result = 0;
	float xBound, yBound, strWidth, widthLimit;

	if (Details->Centre|| Details->RightJustify)
	{
		xBound = 0.0f;
	}
	else
	{
		xBound = arg_x;
	}

	yBound = arg_y;

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
				xBound += GetCharacterSize(0);
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

void CFont::GetTextRect(CRect *result, float arg_x, float arg_y, unsigned __int16 *arg_text)
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

void CFont::PrintString(float arg_x, float arg_y, unsigned __int16 *arg_text)
{
	CRect textBoxRect;

	float xBound, yBound = arg_y, var_24 = 0.0f, print_x = 0.0f, strWidth, widthLimit, justifyWrap;

	bool emptyLine = true;

	__int16 numSpaces = 0;

	unsigned __int16 *strbeg = arg_text;

	if (*arg_text == '*')
	{
		return;
	}

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

	while (*arg_text != 0)
	{
		strWidth = GetStringWidth(arg_text, false);

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
			arg_text = GetNextSpace(arg_text);
			xBound += strWidth;

			if (*arg_text != 0)
			{
				if (*arg_text == ' ')
				{
					if (*(arg_text + 1) == 0)
					{
						*arg_text = 0;
					}
					else
					{
						if (!emptyLine)
						{
							++numSpaces;
						}

						xBound += GetCharacterSize(0);
						++arg_text;
					}
				}

				emptyLine = false;

				var_24 = xBound;
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

				PrintStringPart(print_x, yBound, strbeg, arg_text, 0.0f);
			}
		}
		else
		{
			if (Details->Justify && !(Details->Centre))
			{
				justifyWrap = (Details->WrapX - var_24) / numSpaces;
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

			PrintStringPart(print_x, yBound, strbeg, arg_text, justifyWrap);

			strbeg = arg_text;

			if (Details->Centre || Details->RightJustify)
			{
				xBound = 0.0f;
			}
			else
			{
				xBound = arg_x;
			}

			yBound += Details->Scale.y * 18.0f;
			var_24 = 0.0f;
			numSpaces = 0;
			emptyLine = true;
		}
	}
}

unsigned __int16 *CFont::ParseToken(unsigned __int16 *arg_text, unsigned __int16 *useless)
{
	return ((unsigned __int16 *(__cdecl *)(unsigned __int16 *, unsigned __int16 *))(fpParseToken))(arg_text, useless);
}

void CFont::PrintChar(float arg_x, float arg_y, unsigned __int16 arg_char)
{
	((void(__cdecl *)(float, float, unsigned __int16))(fpPrintChar))(arg_x, arg_y, arg_char);
}

void CFont::PrintCHSChar(float arg_x, float arg_y, unsigned __int16 arg_char)
{
	static const float rRowsCount = 1.0f / 51.2f;
	static const float rColumnsCount = 1.0f / 64.0f;
	static const float ufix = 0.001f / 4.0f;
	static const float vfix = 0.0021f / 4.0f;

	RwD3D8Vertex vertices[6];

	CRect rect;

	float u1, v1, u2, v2, u3, v3, u4, v4;

	CharPos pos = CCharTable::GetCharPos(arg_char);

	u1 = pos.columnIndex * rColumnsCount;
	v1 = pos.rowIndex * rRowsCount;
	u2 = rColumnsCount + pos.columnIndex * rColumnsCount - ufix;
	v2 = v1;
	u3 = u1;
	v3 = rRowsCount + pos.rowIndex * rRowsCount - vfix;
	u4 = u2;
	v4 = v3;

	rect.x1 = arg_x;
	rect.y1 = arg_y + Details->Scale.y * 20.0f;
	rect.x2 = arg_x + Details->Scale.x * 32.0f;
	rect.y2 = arg_y;

	CSprite2d::SetVertices(vertices, rect, Details->Color, Details->Color, Details->Color, Details->Color, u1, v1, u2, v2, u3, v3, u4, v4);

	if (Details->FontStyle == 0)
	{
		rwFunc::RwRenderStateSet(rwRENDERSTATETEXTURERASTER, *(RwRaster **)(ChsSlantSprite.GetRwTexture()));
	}
	else
	{
		rwFunc::RwRenderStateSet(rwRENDERSTATETEXTURERASTER, *(RwRaster **)(ChsSprite.GetRwTexture()));
	}

	rwFunc::RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);
	rwFunc::RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)2);
	rwFunc::RwIm2DRenderPrimitive(rwPRIMTYPETRILIST, vertices, 6);
	rwFunc::RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, nullptr);
}

void CFont::PrintStringPart(float arg_x, float arg_y, unsigned __int16 *strbeg, unsigned __int16 *strend, float justifywrap)
{
	while (strbeg < strend)
	{
		if (*strbeg == '~')
		{
			strbeg = ParseToken(strbeg, nullptr);
		}

		if (Details->Slant != 0.0f)
		{
			arg_y = (Details->SlantRefPoint.x - arg_x) * Details->Slant + Details->SlantRefPoint.y;
		}

		if (*strbeg < 0x80)
		{
			PrintChar(arg_x, arg_y, *strbeg - 0x20);
		}
		else
		{
			PrintCHSChar(arg_x, arg_y, *strbeg - 0x20);
		}

		arg_x += GetCharacterSize(*strbeg - 0x20);

		if (*strbeg == ' ')
		{
			arg_x += justifywrap;
		}

		++strbeg;
	}
}

void CFont::GetAddresses()
{
	hook::pattern ParseTokenPattern("53 83 EC 20 8B 5C 24 28 83 C3 02 80 3D ? ? ? ? 00");

	Size = *hook::pattern("0F BF 1C 5D ? ? ? ?").get(0).get<CFontSizes *>(4);
	fpParseToken = ParseTokenPattern.get(0).get();
	Details = *ParseTokenPattern.get(0).get<CFontDetails *>(13);
	fpPrintChar = hook::pattern("8B 15 ? ? ? ? 53 83 EC 48").get(0).get();
}
