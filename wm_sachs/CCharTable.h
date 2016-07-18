#pragma once
#include "CFont.h"

class CCharTable
{
public:
	struct CharPos
	{
		uint8_t rowIndex;
		uint8_t columnIndex;
	};

	static CharPos GetCharPos(uint32_t chr);

	static void InitTable();

private:
	static CharPos m_Table[];
};
