#include "GDIPlusManager.h"
#include <gdiplus.h>

namespace WinAPI
{
	unsigned long long GDIPlusManager::referenceCount = 0U;
	ULONG_PTR GDIPlusManager::accessToken = 0U;

	GDIPlusManager::GDIPlusManager()
	{
		if (referenceCount++ == 0)
		{
			Gdiplus::GdiplusStartupInput input;
			input.GdiplusVersion = 1;
			input.DebugEventCallback = nullptr;
			input.SuppressBackgroundThread = FALSE;
			input.SuppressExternalCodecs = FALSE;
			Gdiplus::GdiplusStartup(&accessToken, &input, nullptr);
		}
	}

	GDIPlusManager::~GDIPlusManager()
	{
		if (--referenceCount == 0)
			Gdiplus::GdiplusShutdown(accessToken);
	}
}
