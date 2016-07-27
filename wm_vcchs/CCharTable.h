#pragma once
#include "CFont.h"

class CCharTable
{
public:
	struct CharPos
	{
		unsigned __int8 rowIndex;
		unsigned __int8 columnIndex;
	};

	static CharPos GetCharPos(unsigned __int16 chr);

	static void InitTable();

private:
	static CharPos m_Table[];
};
