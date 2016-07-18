#include "VCGXT.h"

#include <iostream>
#include <fstream>
#include <regex>

#include <cstring>
#include <cstdio>

#include "../include/utf8cpp/utf8.h"
#include <iterator>

bool VCGXT::LoadText(const char *path)
{
	std::ifstream inputFile;

	std::string lineBuffer;

	std::vector<uint16_t> wideLineBuffer;

	std::regex tableFormat(R"-(\[([0-9A-Z_]{1,7})\])-");
	std::regex entryFormat(R"-(([0-9A-Z_]{1,7})=(.+))-");
	std::smatch matchResult;

	std::map<std::string, std::map<std::string, std::vector<uint16_t> >, TableSortMethod>::iterator tableIt;

	m_GxtData.clear();
	m_WideCharCollection.clear();

	inputFile.open(path);

	if (!inputFile)
	{
		std::cout << "Open input file failed.\n";
		return false;
	}

	while (std::getline(inputFile, lineBuffer))
	{
		if (lineBuffer.empty() || lineBuffer.front() == ';')
		{
			continue;
		}
		else if (std::regex_match(lineBuffer, matchResult, tableFormat))
		{
			tableIt = m_GxtData.emplace(matchResult.str(1), std::map<std::string, std::vector<uint16_t> >()).first;
		}
		else if (std::regex_match(lineBuffer, matchResult, entryFormat))
		{
			if (m_GxtData.empty())
			{
				std::cout << "Key " << matchResult.str(1) << "belongs to no table.\n";
				return false;
			}

			UTF8ToUTF16(matchResult.str(2), wideLineBuffer);

			if (matchResult.str(1) == "CHS2500" || matchResult.str(1) == "CHS3000" || tableIt->second.emplace(matchResult.str(1), wideLineBuffer).second)
			{
				for (uint16_t chr : wideLineBuffer)
				{
					if (chr >= 0x80)
					{
						m_WideCharCollection.insert(chr);
					}
				}
			}
			else
			{
				std::cout << "Repeated entry:\n" << matchResult.str(1) << "\nin table:\n" << tableIt->first << '\n' << '\n';
				return false;
			}
		}
		else
		{
			std::cout << "Invalid line:\n" << lineBuffer << '\n' << '\n';
			return false;
		}
	}

	return true;
}

void VCGXT::SaveAsGXT(const char *path)
{
	FILE *outputFile;
	long foTableBlock, foKeyBlock, foDataBlock;
	int32_t tableBlockSize, keyBlockOffset, keyBlockSize, TDATOffset, firstTDATEntryOffset, dataBlockSize;
	char eightChars[8];

	tableBlockSize = this->m_GxtData.size() * SizeOfTABL;

	outputFile = fopen(path, "wb");

	if (outputFile == nullptr)
	{
		std::cout << "Failed to create output file.\n";
		return;
	}

	fwrite("TABL", 4, 1, outputFile);
	fwrite(&tableBlockSize, 4, 1, outputFile);
	foTableBlock = 8;
	fseek(outputFile, tableBlockSize, SEEK_CUR);

	for (auto &table : this->m_GxtData)
	{
		foKeyBlock = ftell(outputFile);

		keyBlockSize = table.second.size() * SizeOfTKEY;
		dataBlockSize = GetDataBlockSize(table.second);

		memset(eightChars, 0, 8);
		table.first.copy(eightChars, 7);
		fseek(outputFile, foTableBlock, SEEK_SET);
		fwrite(eightChars, 1, 8, outputFile);
		fwrite(&foKeyBlock, 4, 1, outputFile);
		foTableBlock += SizeOfTABL;

		keyBlockOffset = foKeyBlock;
		fseek(outputFile, foKeyBlock, SEEK_SET);
		if (table.first != "MAIN")
		{
			fwrite(eightChars, 1, 8, outputFile);
		}
		fwrite("TKEY", 1, 4, outputFile);
		fwrite(&keyBlockSize, 4, 1, outputFile);
		foKeyBlock = ftell(outputFile);

		fseek(outputFile, keyBlockSize, SEEK_CUR);
		fwrite("TDAT", 1, 4, outputFile);
		fwrite(&dataBlockSize, 4, 1, outputFile);

		firstTDATEntryOffset = ftell(outputFile);
		
		for (auto &entry : table.second)
		{
			foDataBlock = ftell(outputFile);

			memset(eightChars, 0, 8);
			entry.first.copy(eightChars, 7);
			TDATOffset = foDataBlock - firstTDATEntryOffset;
			fseek(outputFile, foKeyBlock, SEEK_SET);
			fwrite(&TDATOffset, 4, 1, outputFile);
			fwrite(eightChars, 1, 8, outputFile);
			foKeyBlock += SizeOfTKEY;
			fseek(outputFile, foDataBlock, SEEK_SET);
			fwrite(entry.second.data(), 2, entry.second.size(), outputFile);
			
		}
	}

	fclose(outputFile);
}

void VCGXT::GenerateWMHHZStuff()
{
	FILE *charactersSet;
	std::ofstream convCode;

	int row = 0;
	int column = 0;

	convCode.open("TABLE.txt", std::ios::trunc);

	charactersSet = fopen("CHARACTERS.txt", "wb");

	if (!convCode.is_open() || charactersSet == nullptr)
	{
		std::cout << "Failed to create output file.\n";
		return;
	}

	fwrite("\xFF\xFE", 2, 1, charactersSet);

	for (uint16_t chr : this->m_WideCharCollection)
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

size_t VCGXT::GetDataBlockSize(const std::map<std::string,std::vector<uint16_t> > &table)
{
	size_t result = 0;

	for (auto &entry : table)
	{
		result += (entry.second.size() * 2);
	}

	return result;
}

void VCGXT::UTF8ToUTF16(const std::string &str, std::vector<uint16_t> &result)
{
	result.clear();

	utf8::unchecked::utf8to16(str.begin(), str.end(), std::back_inserter(result));

	result.push_back(0);
}
