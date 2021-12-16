#include "ScriptProcess.h"
#include <combaseapi.h>

int main(int argc, char* argv[])
{
	ZE_WIN_ENABLE_EXCEPT();

	ZE_WIN_THROW_FAILED(CoInitializeEx(NULL, COINIT::COINIT_MULTITHREADED));
	std::deque<std::string> input;
	for (int i = 1; i < argc; ++i)
		input.emplace_back(argv[i]);
	const int code = ScriptProcess::Run(input);
	CoUninitialize();
	return code;
}