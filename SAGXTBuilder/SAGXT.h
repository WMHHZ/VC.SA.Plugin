#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <set>

class SAGXT
{
public:
	static const unsigned int SizeOfTABL = 12;
	static const unsigned int SizeOfTKEY = 8;

	bool LoadText(const char *);
	void SaveAsGXT(const char *);
	void GenerateWMHHZStuff();

private:
	struct TableSortMethod
	{
		bool operator()(const std::string &lhs, const std::string &rhs) const
		{
			if		(rhs == "MAIN")	return false;
			else if (lhs == "MAIN")	return true;
			else					return (lhs < rhs);
		}
	};

	static size_t GetDataBlockSize(const std::map<uint32_t, std::string> &);

	std::map<std::string, std::map<uint32_t, std::string>, TableSortMethod> m_GxtData;
	std::set<wchar_t> m_WideCharCollection;
};
