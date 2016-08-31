#include "VCGXT.h"

#include <iostream>
#include <fstream>
#include <regex>
#include <locale>
#include <codecvt>
#include <algorithm>

#include <cstring>
#include <cstdio>

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

			if (!tableIt->second.emplace(matchResult.str(1), wideLineBuffer).second)
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

	outputFile = std::fopen(path, "wb");

	if (outputFile == nullptr)
	{
		std::cout << "Failed to create output file.\n";
		return;
	}

	std::fwrite("TABL", 4, 1, outputFile);
	std::fwrite(&tableBlockSize, 4, 1, outputFile);
	foTableBlock = 8;
	std::fseek(outputFile, tableBlockSize, SEEK_CUR);

	for (auto &table : this->m_GxtData)
	{
		foKeyBlock = std::ftell(outputFile);

		keyBlockSize = table.second.size() * SizeOfTKEY;
		dataBlockSize = GetDataBlockSize(table.second);

		memset(eightChars, 0, 8);
		table.first.copy(eightChars, 7);
		std::fseek(outputFile, foTableBlock, SEEK_SET);
		std::fwrite(eightChars, 1, 8, outputFile);
		std::fwrite(&foKeyBlock, 4, 1, outputFile);
		foTableBlock += SizeOfTABL;

		keyBlockOffset = foKeyBlock;
		std::fseek(outputFile, foKeyBlock, SEEK_SET);
		if (table.first != "MAIN")
		{
			std::fwrite(eightChars, 1, 8, outputFile);
		}
		std::fwrite("TKEY", 1, 4, outputFile);
		std::fwrite(&keyBlockSize, 4, 1, outputFile);
		foKeyBlock = std::ftell(outputFile);

		std::fseek(outputFile, keyBlockSize, SEEK_CUR);
		std::fwrite("TDAT", 1, 4, outputFile);
		std::fwrite(&dataBlockSize, 4, 1, outputFile);

		firstTDATEntryOffset = ftell(outputFile);
		
		for (auto &entry : table.second)
		{
			foDataBlock = ftell(outputFile);

			memset(eightChars, 0, 8);
			entry.first.copy(eightChars, 7);
			TDATOffset = foDataBlock - firstTDATEntryOffset;
			std::fseek(outputFile, foKeyBlock, SEEK_SET);
			std::fwrite(&TDATOffset, 4, 1, outputFile);
			std::fwrite(eightChars, 1, 8, outputFile);
			foKeyBlock += SizeOfTKEY;
			std::fseek(outputFile, foDataBlock, SEEK_SET);
			std::fwrite(entry.second.data(), 2, entry.second.size(), outputFile);
			
		}
	}

	std::fclose(outputFile);
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
	std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;

	std::wstring wstr = converter.from_bytes(str);
	result.resize(wstr.length() + 1);
	std::copy(wstr.begin(), wstr.end(), result.begin());
	result.back() = 0;
}
