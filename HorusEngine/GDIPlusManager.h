#pragma once
#define USE_WINDOWS_DEFINES
#include "WinAPI.h"

namespace WinAPI
{
	// Object to use as access for gdiplus functions
	class GDIPlusManager
	{
		static unsigned long long referenceCount;
		static ULONG_PTR accessToken;

	public:
		GDIPlusManager();
		~GDIPlusManager();
	};
}
