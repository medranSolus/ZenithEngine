#include "StartupConfig.h"

namespace ZE
{
	StartupConfig::StartupConfig(const EngineParams& params) noexcept
	{
		SettingsInitParams init = {};
		init.Type = params.GraphicsAPI;
		init.BackbufferCount = params.BackbufferCount;
		init.AppName = params.AppName;
		init.AppVersion = params.AppVersion;
		init.StaticThreadsCount = params.StaticThreadsCount;
		init.CustomThreadPoolThreadsCount = params.CustomThreadPoolThreadsCount;
		Settings::Init(init);
	}
}