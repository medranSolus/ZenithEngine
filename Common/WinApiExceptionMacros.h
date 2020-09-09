#pragma once
#include "WinApiException.h"

#define	WIN_EXCEPT_LAST() Exception::WinApiException(__LINE__, __FILE__, GetLastError())
#define	WIN_EXCEPT(code) Exception::WinApiException(__LINE__, __FILE__, code)

// Enables useage of WND_THROW_FAILED macro in current scope
#define WIN_ENABLE_EXCEPT() HRESULT __hResult
// Before using needs call to WND_ENABLE_EXCEPT()
#define	WIN_THROW_FAILED(call) if( FAILED(__hResult = (call))) throw WIN_EXCEPT(__hResult)