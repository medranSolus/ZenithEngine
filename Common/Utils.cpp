#include "Utils.h"
#include <sstream>
#include <iomanip>
#include <boost\algorithm\string.hpp>

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

	std::vector<std::string> SplitString(const std::string& input, const std::string& delimeter)
	{
		std::vector<std::string> output;
		boost::split(output, input, boost::is_any_of(delimeter));
		return output;
	}
}