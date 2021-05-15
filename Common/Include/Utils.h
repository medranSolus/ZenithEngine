#pragma once
#include "Types.h"
#include <string>
#include <vector>
#include <deque>

namespace ZE::Utils
{
	inline std::wstring ToUtf8(const std::string& s) noexcept
	{
		return std::wstring(s.begin(), s.end());
	}

	std::string ToAscii(const std::wstring& s) noexcept;
	std::vector<std::string> ParseQuoted(const std::string& input) noexcept;
	std::deque<std::string> SplitString(const std::string& input, const std::string& delimeter) noexcept;
}