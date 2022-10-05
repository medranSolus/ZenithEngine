#pragma once
#include "EngineParams.h"
#include "Settings.h"

namespace ZE
{
	// Engine component responsible for setting up all necessary data for proper engine execution
	class StartupConfig
	{
	public:
		StartupConfig(const EngineParams& params) noexcept { Settings::Init(params.GraphicsAPI, params.BackbufferCount, params.AppName, params.AppVersion); }
		ZE_CLASS_DEFAULT(StartupConfig);
		virtual ~StartupConfig() { Settings::Destroy(); }
	};
}