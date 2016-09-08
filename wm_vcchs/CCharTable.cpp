#include "CCharTable.h"
#include <array>
#include <cstdio>

static std::array<CharPos, 0x10000> sTable;

char CCharTable::datPath[260];

const CharPos &CCharTable::GetCharPos(CharType chr)
{
	return sTable[chr];
}

void CCharTable::ReadTable()
{
	sTable.fill({ 63, 63 });

	FILE *hfile = std::fopen(datPath, "rb");

	if (hfile != nullptr)
	{
		std::fseek(hfile, 0, SEEK_END);

		if (std::ftell(hfile) == 131072)
		{
			std::fseek(hfile, 0, SEEK_SET);
			std::fread(&sTable.front(), 2, 0x10000, hfile);
		}
		
		std::fclose(hfile);
	}
}
