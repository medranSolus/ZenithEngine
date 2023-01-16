#include "Utils.h"
#include <iomanip>

namespace ZE::Utils
{
	std::wstring ToUtf8(std::string_view s) noexcept
	{
		std::wstring str;
		str.reserve(s.size());
		for (char c : s)
			str += static_cast<wchar_t>(c);
		return str;
	}

	std::string ToAscii(std::wstring_view s) noexcept
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

	std::deque<std::string_view> SplitString(std::string_view input, std::string_view delimiter) noexcept
	{
		size_t pos = 0, offset = 0, step = delimiter.size();
		std::deque<std::string_view> output;
		while ((pos = input.find(delimiter, offset)) != std::string::npos)
		{
			if (pos != offset)
			{
				output.emplace_back(input.substr(offset, pos - offset));
				// Remove trailing spaces
				while (output.back().at(output.back().size() - 1) == ' ')
					output.back() = output.back().substr(0, output.back().size() - 1);
			}
			offset = pos + step;
		}
		output.emplace_back(input.substr(offset, input.size() - offset));
		// Remove trailing spaces
		while (output.back().at(output.back().size() - 1) == ' ')
			output.back() = output.back().substr(0, output.back().size() - 1);
		return output;
	}
}