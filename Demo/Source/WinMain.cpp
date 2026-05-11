#include "App.h"
ZE_ENABLE_AGILITYSDK

int HandleStatus(Status stat) noexcept;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	CmdParser parser;

	SettingsInitParams::SetupParser(parser);
	EngineParams::SetupParser(parser);

	parser.AddOption("lightParamsTest");
	parser.AddNumber("lightParamsTestSize", 10);
	parser.AddOption("cubePerfTest");
	parser.AddNumber("cubePerfTestSize", 300000);
	parser.AddOption("noExternalAssets");
	parser.Parse(lpCmdLine);

	App app(parser);
	Status stat = app.Init(parser);
	if (stat)
		return HandleStatus(stat);

	auto exp = app.Run();
	if (exp)
		return *exp;
	return HandleStatus(exp.error());
}

int HandleStatus(Status stat) noexcept
{
	ZE_CODE_CRITICAL(stat, "Crash detected, closing with last known error code!");
	MessageBoxW(nullptr, Utils::ToUTF16(stat.message()).c_str(), Utils::ToUTF16(stat.category().name()).c_str(), MB_OK | MB_ICONERROR);
	return stat.value();
}