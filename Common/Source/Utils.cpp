#include "Utils.h"
#include <iomanip>

namespace Utils
{
	std::vector<std::string> ParseQuoted(const std::string& input)
	{
		std::istringstream stream(input);
		std::vector<std::string> output;
		std::string element;
		while (stream >> std::quoted(element))
			output.emplace_back(std::move(element));
		return output;
	}

	std::deque<std::string> SplitString(const std::string& input, const std::string& delimiter)
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