#include "Utils.h"
#include <iomanip>

namespace Utils
{
	std::string ToAscii(const std::wstring& s) noexcept
	{
		std::string str;
		str.reserve(s.size());
		for (wchar_t c : s)
		{
			str.push_back(static_cast<char>(c));
			if (c > 0xFF)
			{
				str.push_back(static_cast<char>(c >> 8));
				// wchar_t is 16 bit only on Windows
#ifndef _WINDOWS
				str.push_back(static_cast<char>(c >> 16));
				str.push_back(static_cast<char>(c >> 24));
#endif
			}
		}
		return str;
	}

	std::vector<std::string> ParseQuoted(const std::string& input) noexcept
	{
		std::istringstream stream(input);
		std::vector<std::string> output;
		std::string element;
		while (stream >> std::quoted(element))
			output.emplace_back(std::move(element));
		return output;
	}

	std::deque<std::string> SplitString(const std::string& input, const std::string& delimiter) noexcept
	{
		size_t pos = 0, offset = 0, step = delimiter.size();
		std::deque<std::string> output;
		while ((pos = input.find(delimiter, offset)) != std::string::npos)
		{
			output.emplace_back(input.substr(offset, pos - offset));
			offset = pos + step;
		}
		output.emplace_back(input.substr(offset, input.size() - offset));
		return output;
	}
}