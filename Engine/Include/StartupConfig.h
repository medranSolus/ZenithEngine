#pragma once
#include "EngineParams.h"
#include "Settings.h"

namespace ZE
{
	// Engine component responsible for setting up all necessary data for proper engine execution
	class StartupConfig
	{
	public:
		constexpr StartupConfig(const SettingsInitParams& params) noexcept { Settings::Init(params); }
		ZE_CLASS_DEFAULT(StartupConfig);
		virtual ~StartupConfig() { Settings::Destroy(); }
	};
}