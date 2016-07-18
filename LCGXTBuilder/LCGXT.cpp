#include "LCGXT.h"

#include <iostream>
#include <fstream>
#include <regex>

#include <cstdio>
#include <cstring>

#include "../include/utf8cpp/utf8.h"
#include <iterator>

bool LCGXT::LoadText(const char *path)
{
	std::ifstream inputFile;

	std::string lineBuffer;

	std::vector<uint16_t> wideLineBuffer;

	std::regex entryFormat(R"-(([0-9a-zA-Z_]{1,7})=(.+))-");
	std::smatch matchResult;

	m_GxtData.clear();
	m_WideCharCollection.clear();

	inputFile.open(path);

	if (!inputFile.is_open())
	{
		std::cout << "Open input file failed." << std::endl;
		return false;
	}

	while (std::getline(inputFile, lineBuffer))
	{
		if (lineBuffer.empty() || lineBuffer.front() == ';') continue;

		if (std::regex_match(lineBuffer, matchResult, entryFormat))
		{
			UTF8ToUTF16(matchResult.str(2), wideLineBuffer);

			if (matchResult.str(1) == "CHS2500" || matchResult.str(1) == "CHS3000" || m_GxtData.emplace(matchResult.str(1), wideLineBuffer).second)
			{
				for (uint16_t chr : wideLineBuffer)
				{
					if (chr >= 0x80)
					{
						m_WideCharCollection.insert(chr);
					}
				}
			}
		}
		else
		{
			std::cout << "Invalid line:" << std::endl << lineBuffer << std::endl << std::endl;
			return false;
		}
	}

	return true;
}

void LCGXT::SaveAsGXT(const char *path)
{
	FILE *outputFile;

	char keyName[8];

	long fpKeyBlock, fpDataBlock;
	uint32_t keyBlockSize, dataBlockSize, offset;

	if (this->m_GxtData.empty())
		return;

	keyBlockSize = m_GxtData.size() * SizeOfTKEY;
	dataBlockSize = GetDataBlockSize();

	fpKeyBlock = 8;
	fpDataBlock = keyBlockSize + 8;

	outputFile = fopen(path, "wb");

	if (outputFile == nullptr)
	{
		std::cout << "Create output file failed." << std::endl;
		return;
	}

	fwrite("TKEY", 4, 1, outputFile);
	fwrite(&keyBlockSize, 4, 1, outputFile);
	fseek(outputFile, fpDataBlock, SEEK_SET);
	fwrite("TDAT", 4, 1, outputFile);
	fwrite(&dataBlockSize, 4, 1, outputFile);
	fpDataBlock += 8;

	for (auto &entry : m_GxtData)
	{
		fseek(outputFile, fpKeyBlock, SEEK_SET);
		offset = fpDataBlock - keyBlockSize - 16;
		fwrite(&offset, 4, 1, outputFile);
		memset(keyName, 0, 8);
		strncpy(keyName, entry.first.c_str(), 7);
		fwrite(keyName, 8, 1, outputFile);
		fpKeyBlock += SizeOfTKEY;
		fseek(outputFile, fpDataBlock, SEEK_SET);
		fwrite(entry.second.data(), 2, entry.second.size(), outputFile);
		fpDataBlock += entry.second.size() * 2;
	}

	fclose(outputFile);
}

size_t LCGXT::GetDataBlockSize()
{
	size_t result = 0;

	for (auto &entry : m_GxtData)
		result += entry.second.size() * 2;

	return result;
}

void LCGXT::GenerateWMHHZStuff()
{
	FILE *charactersSet;
	std::ofstream convCode;

	int row = 0;
	int column = 0;

	convCode.open("TABLE.txt", std::ios::trunc);

	charactersSet = fopen("CHARACTERS.txt", "wb");

	if (!convCode.is_open() || charactersSet == nullptr)
	{
		std::cout << "Failed to create output file." << std::endl;
		return;
	}

	fwrite("\xFF\xFE", 2, 1, charactersSet);

	for (auto chr : this->m_WideCharCollection)
	{
		convCode << std::hex << "m_Table[0x" << chr << std::dec << "] = {" << row << ',' << column << "};" << '\n';

		fwrite(&chr, 2, 1, charactersSet);

		if (column < 63)
		{
			column += 1;
		}
		else
		{
			row += 1;
			fwrite(L"\n", 2, 1, charactersSet);
			column = 0;
		}
	}

	fclose(charactersSet);
}

void LCGXT::UTF8ToUTF16(const std::string &str, std::vector<uint16_t> &result)
{
	result.clear();

	utf8::unchecked::utf8to16(str.begin(), str.end(), std::back_inserter(result));

	result.push_back(0);
}
