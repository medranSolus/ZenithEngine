#pragma once
#include "GFX/AOType.h"
#include "GFX/UpscalerType.h"
#include "RHI/ApiType.h"
#include "CmdParser.h"

namespace ZE
{
	// Initial parameters for global settings of Zenith Engine
	struct SettingsInitParams
	{
		// Name of the application to register for external services.
		const char* AppName;
		// Identificator of application current version. For convenience you can use ZE::Utils::MakeVersion()
		U32 AppVersion;
		// Selected API that RHI will be initialized to.
		GfxApiType GraphicsAPI;
		// Number of backbuffers to create for swap chain, must be in range [2:16]
		U32 BackbufferCount;
		// Allocate this number of threads from thread pool, decreasing it's number for static threads not managed by the pool.
		U8 StaticThreadsCount;
		// Override number of threads used for scheduling tasks to thread pool.
		// When set to 0 leaves calculation of optimal thread count to the pool.
		// Set to UINT8_MAX to disable thread pool completly.
		U8 CustomThreadPoolThreadsCount;
		// Loads for DirectX 12 targets WinPixGpuCapturer.dll allowing for attaching PIX for GPU capture.
		bool AllowPIXAttach;
		// Type of upscaler to be used in graphics pipeline.
		GFX::UpscalerType Upscaler;
		// Type of ambient occlusion to be used in graphics pipeline.
		GFX::AOType AmbientOcclusion;

		static void SetupParser(CmdParser& parser) noexcept;
		static SettingsInitParams GetParsedParams(const CmdParser& parser, const char* appName, U32 appVersion, U8 staticThreadsCount, GfxApiType defApi = GfxApiType::DX12) noexcept;
		static GfxApiType GetParsedApi(const CmdParser& parser, GfxApiType defApi = GfxApiType::DX12) noexcept;
	};
}