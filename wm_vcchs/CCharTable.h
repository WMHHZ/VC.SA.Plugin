#pragma once
#include "CFont.h"

struct CharPos
{
	unsigned __int8 rowIndex;
	unsigned __int8 columnIndex;
};

class CCharTable
{
public:
	static char datPath[];

	static const CharPos &GetCharPos(CharType chr);

	static void ReadTable();
};
