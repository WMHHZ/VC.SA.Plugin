#pragma once
#include "CFont.h"
#include <cstdint>

class CCharTable
{
public:
	struct CharPos
	{
		uint8_t rowIndex;
		uint8_t columnIndex;
	};

	static CharPos GetCharPos(uint16_t chr);

	static void InitTable();

private:
	static CharPos m_Table[];
};
