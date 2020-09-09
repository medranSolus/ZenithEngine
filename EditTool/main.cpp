#include "ScriptProcess.h"
#define _USE_WINDOWS_DEFINES
#include "WinApiExceptionMacros.h"

int main(int argc, char* argv[])
{
	WIN_ENABLE_EXCEPT();

	WIN_THROW_FAILED(CoInitializeEx(NULL, COINIT::COINIT_MULTITHREADED));
	std::vector<std::string> input;
	for (int i = 1; i < argc; ++i)
		input.emplace_back(argv[i]);
	const int code = ScriptProcess::Run(input);
	CoUninitialize();
	return code;
}