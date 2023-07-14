#pragma once
#include "GFX/API/ApiType.h"

namespace ZE
{
	// Initial parameters for global settings
	struct SettingsInitParams
	{
		GfxApiType Type;
		U32 BackbufferCount;
		const char* AppName;
		U32 AppVersion;
		U8 CustomThreadPoolThreadsCount;
	};
}