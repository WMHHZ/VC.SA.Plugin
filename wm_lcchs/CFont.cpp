#include "CFont.h"
#include "CTxdStore.h"
#include "CCharTable.h"
#include "rwFunc.h"
#include "../deps/selector/AddressSelector.h"

char CFont::texturePath[MAX_PATH];
char CFont::textPath[MAX_PATH];

CSprite2d CFont::ChsSprite;
CSprite2d CFont::ChsSlantSprite;
void *CFont::fpPrintChar;
void *CFont::fpParseToken;
CFontDetails *CFont::Details;
CFontSizes *CFont::Size;

void CFont::LoadCHSTexture()
{
	CTxdStore::PopCurrentTxd();
	int slot = CTxdStore::AddTxdSlot("wm_lcchs");
	CTxdStore::LoadTxd(slot, texturePath);
	CTxdStore::AddRef(slot);
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(slot);
	CFont::ChsSprite.SetTexture("chs_normal", "chs_normal_mask");
	CFont::ChsSlantSprite.SetTexture("chs_slant", "chs_slant_mask");
	CTxdStore::PopCurrentTxd();
}

void CFont::UnloadCHSTexture(int dummy)
{
	CTxdStore::RemoveTxdSlot(dummy);
	CTxdStore::RemoveTxdSlot(CTxdStore::FindTxdSlot("wm_lcchs"));
	CFont::ChsSprite.Delete();
	CFont::ChsSlantSprite.Delete();
}

float CFont::GetCharacterSize(unsigned __int16 arg_char)
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
		iCharWidth = 28;
	}

	return (iCharWidth * Details->Scale.x);
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

	CharPos pos;

	if (arg_x >= *rwFunc::RsGlobalW ||
		arg_x <= 0.0f ||
		arg_y <= 0.0f ||
		arg_y >= *rwFunc::RsGlobalH)
	{
		return;
	}

	pos = CCharTable::GetCharPos(arg_char);

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
		rwFunc::RwRenderStateSet(rwRENDERSTATETEXTURERASTER, *(RwRaster **)ChsSlantSprite.GetRwTexture());
	}
	else
	{
		rwFunc::RwRenderStateSet(rwRENDERSTATETEXTURERASTER, *(RwRaster **)ChsSprite.GetRwTexture());
	}

	rwFunc::RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);
	rwFunc::RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)2);
	rwFunc::RwIm2DRenderPrimitive(rwPRIMTYPETRILIST, vertices, 6);
	rwFunc::RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, nullptr);
}

void CFont::PrintCharDispatcher(float arg_x, float arg_y, unsigned __int16 arg_char)
{
	if (arg_char < 0x60)
	{
		PrintChar(arg_x, arg_y, arg_char);
	}
	else
	{
		PrintCHSChar(arg_x, arg_y, arg_char);
	}
}

unsigned __int16 *CFont::ParseToken(unsigned __int16 *arg_text, unsigned __int16 *useless)
{
	return ((unsigned __int16 *(__cdecl *)(unsigned __int16 *, unsigned __int16 *))(fpParseToken))(arg_text, useless);
}

CFont::CFont()
{
	Size = AddressSelectorLC::SelectAddress<0x5FD120, 0x0, 0x609F00, CFontSizes>();
	Details = AddressSelectorLC::SelectAddress<0x8F317C, 0x0, 0x903370, CFontDetails>();
	fpPrintChar = AddressSelectorLC::SelectAddress<0x500C30, 0x0, 0x500CA0>();
	fpParseToken = AddressSelectorLC::SelectAddress<0x5019A0, 0x0, 0x501A10>();
}

static CFont instance;
