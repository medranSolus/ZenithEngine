#pragma once

#if _ZE_PLATFORM_WINDOWS
namespace ZE::Platform::WinAPI
{
	class Window;
}
#else
#	error Missing Window platform specific window forward declaration!
#endif