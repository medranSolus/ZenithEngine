#include "App.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	constexpr UINT ERROR_BOX_STYLE = MB_OK | MB_ICONERROR;
	try
	{
		srand(static_cast<unsigned int>(time(NULL)));
		return App(lpCmdLine).Run();
	}
	catch (const ZE::Exception::BasicException& e)
	{
		MessageBox(nullptr, e.what(), e.GetType(), ERROR_BOX_STYLE);
	}
	catch (const std::exception& e)
	{
		MessageBox(nullptr, e.what(), "std::exception", ERROR_BOX_STYLE);
	}
	catch (...)
	{
		MessageBox(nullptr, "Unknown exception", "No type", ERROR_BOX_STYLE);
	}
	return -1;
}