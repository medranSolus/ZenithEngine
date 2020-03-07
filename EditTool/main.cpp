#include "ScriptProcess.h"
#include <Windows.h>

int main(int argc, char* argv[])
{
	CoInitializeEx(NULL, COINIT::COINIT_MULTITHREADED);
	std::vector<std::string> input;
	for (int i = 1; i < argc; ++i)
		input.emplace_back(argv[i]);
	const int code = ScriptProcess::Run(input);
	CoUninitialize();
	return code;
}