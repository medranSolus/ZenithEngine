#pragma once
#include "GFX/VendorGPU.h"
#include "SettingsInitParams.h"
#include "ThreadPool.h"

namespace ZE
{
	// Global engine settings
	class Settings final
	{
		static constexpr const char* ENGINE_NAME = "ZenithEngine";
		static constexpr U32 ENGINE_VERSION = Utils::MakeVersion(0, 3, 0);

		static constexpr U64 BUFFERS_HEAP_SIZE = 256 * Math::MEGABYTE;
		static constexpr U64 TEXTURES_HEAP_SIZE = 512 * Math::MEGABYTE;
		static constexpr U64 HOST_HEAP_SIZE = 64 * Math::MEGABYTE;
		static constexpr U64 STAGING_HEAP_SIZE = 64 * Math::MEGABYTE;

		static inline U64 frameIndex = 0;
		static inline U32 swapChainBufferCount = 0;
		static inline UInt2 swapChainSize = { 0, 0 };
		static inline PixelFormat backbufferFormat = PixelFormat::R8G8B8A8_UNorm;
		static inline GFX::VendorGPU gpuVendor = GFX::VendorGPU::Unknown;
		static inline GfxApiType gfxApi;
		static inline const char* applicationName;
		static inline U32 applicationVersion;
		static inline std::bitset<2> flags = 0;
		static inline ThreadPool threadPool;

		static constexpr bool Initialized() noexcept { return swapChainBufferCount != 0; }

	public:
		ZE_CLASS_DELETE(Settings);
		~Settings() = default;

		static constexpr const char* GetEngineName() noexcept { return ENGINE_NAME; }
		static constexpr U32 GetEngineVersion() noexcept { return ENGINE_VERSION; }

		static constexpr U64 GetHeapSizeBuffers() noexcept { return BUFFERS_HEAP_SIZE; }
		static constexpr U64 GetHeapSizeTextures() noexcept { return TEXTURES_HEAP_SIZE; }
		static constexpr U64 GetHeapSizeHost() noexcept { return HOST_HEAP_SIZE; }
		static constexpr U64 GetHeapSizeStaging() noexcept { return STAGING_HEAP_SIZE; }

		static constexpr U64 GetFrameIndex() noexcept { return frameIndex; }
		static constexpr void AdvanceFrame() noexcept { ++frameIndex; }

		static constexpr void SetBackbufferFormat(PixelFormat format) noexcept { backbufferFormat = format; }
		static constexpr PixelFormat GetBackbufferFormat() noexcept { return backbufferFormat; }

		static constexpr void SetGpuVendor(GFX::VendorGPU vendor) noexcept { gpuVendor = vendor; }
		static constexpr GFX::VendorGPU GetGpuVendor() noexcept { return gpuVendor; }
		static constexpr GfxApiType GetGfxApi() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return gfxApi; }

		static constexpr const char* GetAppName() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return applicationName; }
		static constexpr U32 GetAppVersion() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return applicationVersion; }

		static constexpr U32 GetCurrentBackbufferIndex() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return frameIndex % swapChainBufferCount; }
		static constexpr U32 GetBackbufferCount() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return swapChainBufferCount; }
		static constexpr void SetBackbufferSize(U32 width, U32 height) noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); swapChainSize = { width, height }; }
		static constexpr UInt2 GetBackbufferSize() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return swapChainSize; }
		static constexpr U32 GetBackbufferWidth() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return swapChainSize.X; }
		static constexpr U32 GetBackbufferHeight() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return swapChainSize.Y; }

		static constexpr U32 GetCurrentChainResourceIndex() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return frameIndex % GetChainResourceCount(); }
		static constexpr U32 GetChainResourceCount() noexcept;

		static constexpr void Destroy() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); swapChainBufferCount = 0; }
		static constexpr void Init(const SettingsInitParams& params) noexcept;

#if _ZE_GFX_MARKERS
		static constexpr void SetGfxTags(bool enabled) noexcept { flags[0] = enabled; }
		static constexpr bool IsEnabledGfxTags() noexcept { return flags[0]; }
#endif
		static constexpr void SetU8IndexSets(bool enabled) noexcept { flags[1] = enabled; }
		static constexpr bool IsEnabledU8IndexSets() noexcept { return flags[1]; }

		static constexpr ThreadPool& GetThreadPool() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return threadPool; }
	};

#pragma region Functions
	constexpr U32 Settings::GetChainResourceCount() noexcept
	{
		ZE_ASSERT(Initialized(), "Not initialized!");

		switch (GetGfxApi())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GfxApiType::DX11:
		case GfxApiType::OpenGL:
			return 1;
		case GfxApiType::DX12:
		case GfxApiType::Vulkan:
			return swapChainBufferCount;
		}
	}

	constexpr void Settings::Init(const SettingsInitParams& params) noexcept
	{
		ZE_ASSERT(!Initialized(), "Already initialized!");
		ZE_ASSERT(params.BackbufferCount > 1 && params.BackbufferCount < 17, "Incorrect params!");

		gfxApi = params.Type;
		ZE_ASSERT(gfxApi == GfxApiType::DX11 && _ZE_RHI_DX11 || gfxApi != GfxApiType::DX11,
			"DirectX 11 API is not enabled in current build!");
		ZE_ASSERT(gfxApi == GfxApiType::DX12 && _ZE_RHI_DX12 || gfxApi != GfxApiType::DX12,
			"DirectX 12 API is not enabled in current build!");
		ZE_ASSERT(gfxApi == GfxApiType::OpenGL && _ZE_RHI_GL || gfxApi != GfxApiType::OpenGL,
			"OpenGL API is not enabled in current build!");
		ZE_ASSERT(gfxApi == GfxApiType::Vulkan && _ZE_RHI_VK || gfxApi != GfxApiType::Vulkan,
			"Vulkan API is not enabled in current build!");

		swapChainBufferCount = params.BackbufferCount;
		applicationName = params.AppName ? params.AppName : ENGINE_NAME;
		applicationVersion = params.AppVersion;
		threadPool.Init(params.CustomThreadPoolThreadsCount);
	}
#pragma endregion
}