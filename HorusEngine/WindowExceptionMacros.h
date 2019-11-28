#pragma once
#include "Window.h"

#define	WND_NO_GFX_EXCEPT() WinAPI::Window::NoGfxException(__LINE__, __FILE__)
#define	WND_EXCEPT(code) WinAPI::Window::WindowException(__LINE__, __FILE__, code)
#define	WND_EXCEPT_LAST() WinAPI::Window::WindowException(__LINE__, __FILE__, GetLastError())
