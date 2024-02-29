#include "ScriptProcess.h"

int main(int argc, char* argv[])
{
	std::deque<std::string> input;
	for (int i = 1; i < argc; ++i)
		input.emplace_back(argv[i]);
	const int code = ScriptProcess::Run(input);
	return code;
}