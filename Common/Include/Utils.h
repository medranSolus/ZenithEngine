#pragma once
#include <string>
#include <locale>
#include <codecvt>
#include <vector>
#include <deque>

namespace Utils
{
	inline std::wstring ToUtf8(const std::string& s)
	{
		return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes(s);
	}

	inline std::string ToAscii(const std::wstring& s)
	{
		return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.to_bytes(s);
	}

	std::vector<std::string> ParseQuoted(const std::string& input);
	std::deque<std::string> SplitString(const std::string& input, const std::string& delimeter);
}