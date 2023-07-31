#pragma once

#if _ZE_PLATFORM_WINDOWS
#	include "Platform/WinAPI/Window.h"
namespace ZE::Window
{
	// Application window for Windows
	typedef ZE::WinAPI::Window MainWindow;
}
#else
#	error Missing Window platform specific implementation!
#endif