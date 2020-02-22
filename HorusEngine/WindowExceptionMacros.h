#pragma once
#include "Window.h"

#define	WND_NO_GFX_EXCEPT() WinAPI::Window::NoGfxException(__LINE__, __FILE__)
#define	WND_EXCEPT_LAST() WinAPI::Window::WindowException(__LINE__, __FILE__, GetLastError())
#define	WND_EXCEPT(code) WinAPI::Window::WindowException(__LINE__, __FILE__, code)

// Enables useage of WND_THROW_FAILED macro in current scope
#define WND_ENABLE_EXCEPT() HRESULT __hResult
// Before using needs call to WND_ENABLE_EXCEPT()
#define	WND_THROW_FAILED(call) if( FAILED(__hResult = (call))) throw WND_EXCEPT(__hResult)