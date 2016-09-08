#pragma once
#include "CSprite2d.h"
#include <freetype/ftimage.h>

struct CachedCharInfo
{
	unsigned int bitmap_width;
	unsigned int bitmap_height;
	float width;

	CSprite2d sprite;
};

class CFreeType
{
	static void InitCharInfo(wchar_t arg_char);

public:
	static void Init();
	static void Close();

	static const CachedCharInfo &GetCharInfo(wchar_t arg_char);
};
