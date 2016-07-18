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
	static CharPos GetCharPos(unsigned __int16 chr);

	static void InitTable();

private:
	static CharPos m_Table[];
};
