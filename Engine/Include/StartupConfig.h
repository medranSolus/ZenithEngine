#pragma once
#include "EngineParams.h"
#include "Settings.h"

namespace ZE
{
	// Engine component responsible for setting up all necessary data for proper engine execution
	class StartupConfig
	{
	public:
		constexpr StartupConfig(const SettingsInitParams& params) noexcept;
		ZE_CLASS_DEFAULT(StartupConfig);
		virtual ~StartupConfig() { Settings::Destroy(); }
	};

#pragma region Functions
	constexpr StartupConfig::StartupConfig(const SettingsInitParams& params) noexcept
	{
		Status stat = Settings::Init(params);
		if (stat)
		{
			ZE_CODE_CRITICAL(stat, "Failed to initialize main settings, aborting!");
			std::abort();
		}
	}
#pragma endregion
}