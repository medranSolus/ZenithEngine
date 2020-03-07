#include "Utils.h"
#include <sstream>
#include <iomanip>

std::vector<std::string> parseQuoted(const std::string& input)
{
	std::istringstream stream(input);
	std::vector<std::string> output;
	std::string element;
	while (stream >> std::quoted(element))
		output.emplace_back(std::move(element));
	return output;
}
