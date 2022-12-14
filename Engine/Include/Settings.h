#pragma once
#include "GFX/API/ApiType.h"
#include "GFX/VendorGPU.h"

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

		static inline U64 frameIndex = 0;
		static inline U32 swapChainBufferCount = 0;
		static inline PixelFormat backbufferFormat = PixelFormat::R8G8B8A8_UNorm;
		static inline GFX::VendorGPU gpuVendor = GFX::VendorGPU::Unknown;
		static inline GfxApiType gfxApi;
		static inline const char* applicationName;
		static inline U32 applicationVersion;
		static inline std::bitset<2> flags = 0;

		static constexpr bool Initialized() noexcept { return swapChainBufferCount != 0; }

	public:
		ZE_CLASS_DELETE(Settings);
		~Settings() = default;

		static constexpr const char* GetEngineName() noexcept { return ENGINE_NAME; }
		static constexpr U32 GetEngineVersion() noexcept { return ENGINE_VERSION; }

		static constexpr U64 GetHeapSizeBuffers() noexcept { return BUFFERS_HEAP_SIZE; }
		static constexpr U64 GetHeapSizeTextures() noexcept { return TEXTURES_HEAP_SIZE; }
		static constexpr U64 GetHeapSizeHost() noexcept { return HOST_HEAP_SIZE; }

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
		static constexpr U32 GetCurrentChainResourceIndex() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return frameIndex % GetChainResourceCount(); }
		static constexpr U32 GetChainResourceCount() noexcept;

		static constexpr void Destroy() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); swapChainBufferCount = 0; }
		static constexpr void Init(GfxApiType type, U32 backBufferCount, const char* appName, U32 appVersion) noexcept;

#if _ZE_GFX_MARKERS
		static void SetGfxTags(bool enabled) noexcept { flags[0] = enabled; }
		static bool IsEnabledGfxTags() noexcept { return flags[0]; }
#endif
		static void SetU8IndexSets(bool enabled) noexcept { flags[1] = enabled; }
		static bool IsEnabledU8IndexSets() noexcept { return flags[1]; }
	};

#pragma region Functions
	constexpr U32 Settings::GetChainResourceCount() noexcept
	{
		ZE_ASSERT(Initialized(), "Not initialized!");

		switch (GetGfxApi())
		{
		default:
			ZE_FAIL("Unhandled enum value!");
		case GfxApiType::DX11:
		case GfxApiType::OpenGL:
			return 1;
		case GfxApiType::DX12:
		case GfxApiType::Vulkan:
			return swapChainBufferCount;
		}
	}

	constexpr void Settings::Init(GfxApiType type, U32 backBufferCount, const char* appName, U32 appVersion) noexcept
	{
		ZE_ASSERT(!Initialized(), "Already initialized!");
		ZE_ASSERT(backBufferCount > 1 && backBufferCount < 17, "Incorrect params!");

		gfxApi = type;
		swapChainBufferCount = backBufferCount;
		applicationName = appName ? appName : ENGINE_NAME;
		applicationVersion = appVersion;
	}
#pragma endregion
}