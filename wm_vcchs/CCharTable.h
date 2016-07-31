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

	static CharPos GetCharPos(CharType chr);

	static void InitTable();

	CCharTable();

private:
	static CharPos m_Table[];
};
