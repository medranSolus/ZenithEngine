#pragma once
#include <string>
#include <vector>
#include <deque>
#include <boost/locale/encoding.hpp>

namespace Utils
{
	inline std::wstring ToUtf8(const std::string& s)
	{
		return boost::locale::conv::to_utf<wchar_t>(s, "UTF-8");
	}

	inline std::string ToAscii(const std::wstring& s)
	{
		return boost::locale::conv::from_utf<wchar_t>(s, "UTF-8");
	}

	std::vector<std::string> ParseQuoted(const std::string& input);
	std::deque<std::string> SplitString(const std::string& input, const std::string& delimeter);
}