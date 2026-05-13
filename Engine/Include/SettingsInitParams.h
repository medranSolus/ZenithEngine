#pragma once
#include "AHI/ApiType.h"
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

	// Description of heap sizes to be allocated by the allocators
	struct HeapParams
	{
		// GPU heap for buffers only
		U64 BuffersHeapSize = 256 * Math::MEGABYTE;
		// GPU heap for textures only
		U64 TexturesHeapSize = 512 * Math::MEGABYTE;
		// CPU side heap for frequently uploaded buffers
		U64 HostHeapSize = 64 * Math::MEGABYTE;
		// CPU-mappable heap for upload data
		U64 UploadHeapSize = 64 * Math::MEGABYTE;
		// Single CPU staging buffer for preparing data before upload to GPU
		U32 StagingBufferSize = 384 * Math::MEGABYTE;
	};

	// Initial parameters for global settings of Zenith Engine
	struct SettingsInitParams
	{
		// Name of the application to register for external services.
		const char* AppName = nullptr;
		// Identificator of application current version. For convenience you can use ZE::Utils::MakeVersion()
		U32 AppVersion = 0;
		// Initial flags that enable engine features.
		SettingsInitFlags Flags = 0;
		// Selected API that RHI will be initialized to.
		GfxApiType GraphicsAPI = GfxApiType::None;
		// Selected API that AHI will be initialized to.
		AudioApiType AudioAPI = AudioApiType::None;
		// Number of backbuffers to create for swap chain, must be in range [2:16]
		U32 BackbufferCount = 2;
		// Allocate this number of threads from thread pool, decreasing it's number for static threads not managed by the pool.
		U8 StaticThreadsCount = 0;
		// Override number of threads used for scheduling tasks to thread pool.
		// When set to 0 leaves calculation of optimal thread count to the pool.
		// Set to UINT8_MAX to disable thread pool completly.
		U8 CustomThreadPoolThreadsCount = 0;
		// Type of upscaler to be used in graphics pipeline.
		GFX::UpscalerType Upscaler = GFX::UpscalerType::None;
		// Type of ambient occlusion to be used in graphics pipeline.
		GFX::AOType AmbientOcclusion = GFX::AOType::None;
		// Type of tonemapper to be applied on the rendered image.
		GFX::TonemapperType Tonemapper = GFX::TonemapperType::GranTurismo7;
		// Custom sizes for heaps allocated by the engine
		HeapParams HeapSizes = {};

		static void SetupParser(CmdParser& parser) noexcept;
		static SettingsInitParams GetParsedParams(const CmdParser& parser, const char* appName, U32 appVersion,
			U8 staticThreadsCount, GfxApiType defGfxApi = GfxApiType::DX12, AudioApiType defAudioApi = AudioApiType::XAudio2) noexcept;
		static GfxApiType GetParsedApi(const CmdParser& parser, GfxApiType defApi = GfxApiType::DX12) noexcept;
		static AudioApiType GetParsedApi(const CmdParser& parser, AudioApiType defApi = AudioApiType::XAudio2) noexcept;
	};
}