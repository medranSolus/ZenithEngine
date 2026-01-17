#pragma once
#include "GFX/AOType.h"
#include "GFX/UpscalerType.h"
#include "GFX/TonemapperType.h"
#include "RHI/ApiType.h"
#include "CmdParser.h"

namespace ZE
{
	// Set of flags used to enable various engine features.
	typedef U8 SettingsInitFlags;
	// Possible engine features to be enabled by the application.
	enum class SettingsInitFlag : SettingsInitFlags
	{
		// Loads for DirectX 12 targets WinPixGpuCapturer.dll allowing for attaching PIX for GPU capture. Disabled in release builds.
		AllowPIXAttach = 1,
		// Enable additional GPU validation for supported APIs on debug and development builds for more verbose checks. May slow rendering considerably.
		EnableGPUValidation = 2,
		// Enable Screen Space Reflections, only available for DirectX 12 or Vulkan RHI.
		EnableSSSR = 4,
		// Run Ambient Occlusion on async compute queue for supported RHI.
		AsyncAO = 8,
		// When uploading data to the GPU ignore possible optimizations and always copy source data. Disabled in release builds.
		AlwaysCopySourceGPUData = 16,
		// Don't perform any type of culling. Disabled in release builds.
		DisableCulling = 32,
		// Submit every GPU workload as separate bundle for execution. Only for debug and development builds.
		SplitRenderSubmissions = 64,
		// Enable Image Based Lighting as handler of ambient lighting.
		EnableIBL = 128,
	};
	ZE_ENUM_OPERATORS(SettingsInitFlag, SettingsInitFlags);

	// Initial parameters for global settings of Zenith Engine
	struct SettingsInitParams
	{
		// Name of the application to register for external services.
		const char* AppName;
		// Identificator of application current version. For convenience you can use ZE::Utils::MakeVersion()
		U32 AppVersion;
		// Initial flags that enable engine features.
		SettingsInitFlags Flags;
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
		// Type of upscaler to be used in graphics pipeline.
		GFX::UpscalerType Upscaler;
		// Type of ambient occlusion to be used in graphics pipeline.
		GFX::AOType AmbientOcclusion;
		// Type of tonemapper to be applied on the rendered image.
		GFX::TonemapperType Tonemapper;

		static void SetupParser(CmdParser& parser) noexcept;
		static SettingsInitParams GetParsedParams(const CmdParser& parser, const char* appName, U32 appVersion, U8 staticThreadsCount, GfxApiType defApi = GfxApiType::DX12) noexcept;
		static GfxApiType GetParsedApi(const CmdParser& parser, GfxApiType defApi = GfxApiType::DX12) noexcept;
	};
}