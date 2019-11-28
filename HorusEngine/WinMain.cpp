#include "App.h"
#include <ctime>
#include <cstdlib>

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	constexpr UINT errorBoxStyle = MB_OK | MB_ICONERROR;
	try
	{
		srand(static_cast<unsigned int>(time(NULL)));
		return static_cast<int>(App{}.Run());
	}
	catch (const Exception::BasicException & e)
	{
		MessageBox(nullptr, e.what(), e.GetType(), errorBoxStyle);
	}
	catch (const std::exception & e)
	{
		MessageBox(nullptr, e.what(), "std::exception", errorBoxStyle);
	}
	catch (...)
	{
		MessageBox(nullptr, "Unknown exception", "No type", errorBoxStyle);
	}
	return -1;
}