#pragma once

#if _ZE_PLATFORM_WINDOWS
#include "Platform/WindowWinAPI.h"
namespace ZE::Window
{
	// Application window for Windows
	typedef WinAPI::WindowWinAPI MainWindow;
}
#else
#	error Missing Window platform specific implementation!
#endif