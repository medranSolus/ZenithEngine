#pragma once
#include <string>
#include <vector>
#include <boost/locale/encoding.hpp>

inline std::wstring toUtf8(const std::string& s)
{
	return boost::locale::conv::to_utf<wchar_t>(s, "UTF-8");
}

inline std::string toAscii(const std::wstring& s)
{
	return boost::locale::conv::from_utf<wchar_t>(s, "UTF-8");
}

std::vector<std::string> parseQuoted(const std::string& input);

std::vector<std::string> splitString(const std::string& input, const std::string& delimeter);