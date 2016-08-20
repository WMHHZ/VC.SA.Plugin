#pragma once
#include "CSprite2d.h"

struct RwImage;

struct CachedCharInfo
{
	unsigned __int8 widthInPixels;
	unsigned __int8 heightInPixels;
	CSprite2d sprite;
};

class CFreeType
{
	static CachedCharInfo m_Sprites[];

	static void InitCharSprite(wchar_t arg_char);

public:
	static void Init();
	static void Close();

	static const CachedCharInfo &GetCharSprite(wchar_t arg_char);
};
