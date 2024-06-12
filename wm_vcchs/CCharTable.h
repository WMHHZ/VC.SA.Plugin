#pragma once
#include "CFontPatch.h"

struct CharPos
{
	unsigned char rowIndex;
	unsigned char columnIndex;
};

class CCharTable
{
public:
	static char datPath[];

	static const CharPos &GetCharPos(CharType chr);

	static void ReadTable();
};
