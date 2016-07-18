#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <set>
#include <vector>

class LCGXT
{
public:
	static const unsigned int SizeOfTKEY = 12;

	bool LoadText(const char *path);
	void SaveAsGXT(const char *path);
	size_t GetDataBlockSize();
	void GenerateWMHHZStuff();

private:
	static inline void UTF8ToUTF16(const std::string &, std::vector<uint16_t> &);

	std::map<std::string, std::vector<uint16_t> > m_GxtData;
	std::set<uint16_t> m_WideCharCollection;
};
