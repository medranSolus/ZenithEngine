#include "App.h"
ZE_ENABLE_AGILITYSDK

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	constexpr UINT ERROR_BOX_STYLE = MB_OK | MB_ICONERROR;
	try
	{
		srand(static_cast<unsigned int>(time(nullptr)));
		CmdParser parser;
		SettingsInitParams::SetupParser(parser);
		EngineParams::SetupParser(parser);
		parser.Parse(lpCmdLine);
		return App(parser).Run();
	}
	catch (const ZE::Exception::BasicException& e)
	{
		MessageBoxW(nullptr, Utils::ToUTF16(e.what()).c_str(), Utils::ToUTF16(e.GetType()).c_str(), ERROR_BOX_STYLE);
	}
	catch (const std::exception& e)
	{
		MessageBoxW(nullptr, Utils::ToUTF16(e.what()).c_str(), L"std::exception", ERROR_BOX_STYLE);
	}
	catch (...)
	{
		MessageBoxW(nullptr, L"Unknown exception", L"No type", ERROR_BOX_STYLE);
	}
	return -1;
}