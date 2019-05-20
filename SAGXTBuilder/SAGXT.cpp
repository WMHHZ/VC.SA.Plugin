#include "SAGXT.h"

#include <iostream>
#include <fstream>
#include <regex>

#include <cstring>
#include <cstdio>

#include "../deps/utf8cpp/utf8.h"

bool SAGXT::LoadText(const char *path)
{
    std::ifstream inputFile;

    std::string lineBuffer;

    std::regex tableFormat(R"-(\[([0-9A-Z_]{1,7})\])-");
    std::regex entryFormat(R"-(([0-9a-fA-F]{1,8})=(.+))-");
    std::smatch matchResult;

    std::map<std::string, std::map<uint32_t, std::string>, TableSortMethod>::iterator tableIt;

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
            tableIt = m_GxtData.emplace(matchResult.str(1), std::map<uint32_t, std::string>()).first;
        }
        else if (std::regex_match(lineBuffer, matchResult, entryFormat))
        {
            if (m_GxtData.empty())
            {
                std::cout << "Key " << matchResult.str(1) << "belongs to no table.\n";
                return false;
            }

            auto hash = std::stoul(matchResult.str(1), nullptr, 16);
            if (tableIt->second.emplace(hash, matchResult.str(2)).second)
            {
                utf8::iterator<std::string::const_iterator> it(matchResult[2].first, matchResult[2].first, matchResult[2].second);

                while (it.base() != matchResult[2].second)
                {
                    m_WideCharCollection.insert(*it);
                    ++it;
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

void SAGXT::SaveAsGXT(const char *path)
{
    FILE *outputFile;
    long foTableBlock, foKeyBlock, foDataBlock;
    int32_t tableBlockSize, keyBlockOffset, keyBlockSize, TDATOffset, dataOffset, dataBlockSize;
    char eightChars[8];

    tableBlockSize = this->m_GxtData.size() * SizeOfTABL;

    outputFile = fopen(path, "wb");

    if (outputFile == nullptr)
    {
        std::cout << "Failed to create output file.\n";
        return;
    }

    fwrite("\x04\x00\x08\x00", 4, 1, outputFile);
    fwrite("TABL", 4, 1, outputFile);
    fwrite(&tableBlockSize, 4, 1, outputFile);
    foTableBlock = 12;
    foKeyBlock = tableBlockSize + 12;
    keyBlockOffset = foKeyBlock;

    for (auto &table : this->m_GxtData)
    {
        keyBlockSize = table.second.size() * SizeOfTKEY;
        dataBlockSize = GetDataBlockSize(table.second);
        fseek(outputFile, foTableBlock, SEEK_SET);
        memset(eightChars, 0, 8);
        table.first.copy(eightChars, 7);
        fwrite(eightChars, 8, 1, outputFile);
        fwrite(&keyBlockOffset, 4, 1, outputFile);
        foTableBlock += SizeOfTABL;
        fseek(outputFile, foKeyBlock, SEEK_SET);
        if (table.first != "MAIN")
        {
            fwrite(eightChars, 8, 1, outputFile);
        }
        fwrite("TKEY", 4, 1, outputFile);
        fwrite(&keyBlockSize, 4, 1, outputFile);
        foKeyBlock = ftell(outputFile);
        fseek(outputFile, keyBlockSize, SEEK_CUR);
        TDATOffset = ftell(outputFile);
        fwrite("TDAT", 4, 1, outputFile);
        fwrite(&dataBlockSize, 4, 1, outputFile);
        foDataBlock = ftell(outputFile);
        for (auto &entry : table.second)
        {
            dataOffset = foDataBlock - TDATOffset - 8;
            fseek(outputFile, foKeyBlock, SEEK_SET);
            fwrite(&dataOffset, 4, 1, outputFile);
            fwrite(&entry.first, 4, 1, outputFile);
            foKeyBlock += SizeOfTKEY;
            fseek(outputFile, foDataBlock, SEEK_SET);
            fwrite(entry.second.c_str(), entry.second.length() + 1, 1, outputFile);
            foDataBlock = ftell(outputFile);
        }

        foKeyBlock = ftell(outputFile);
        keyBlockOffset = foKeyBlock;
    }

    fclose(outputFile);
}

void SAGXT::GenerateWMHHZStuff()
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

size_t SAGXT::GetDataBlockSize(const std::map<uint32_t, std::string> &table)
{
    size_t result = 0;

    for (auto &entry : table)
    {
        result += entry.second.length() + 1;
    }

    return result;
}
