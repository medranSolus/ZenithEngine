#pragma once
#include "GFX/API/ApiType.h"
#include "GFX/VendorGPU.h"

namespace ZE
{
	// Global engine settings
	class Settings final
	{
		static constexpr const char* ENGINE_NAME = "ZenithEngine";
		static constexpr uint32_t ENGINE_VERSION = Utils::MakeVersion(0, 3, 0);
		static constexpr PixelFormat BACKBUFFER_FORMAT = PixelFormat::R8G8B8A8_UNorm;

		static inline U64 frameIndex = 0;
		static inline U32 swapChainBufferCount = 0;
		static inline GFX::VendorGPU gpuVendor = GFX::VendorGPU::Unknown;
		static inline GfxApiType gfxApi;
		static inline const char* applicationName;
		static inline uint32_t applicationVersion;
#ifdef _ZE_MODE_DEBUG
		static inline bool enableGfxTags = true;
#endif

		static constexpr bool Initialized() noexcept { return swapChainBufferCount != 0; }

	public:
		ZE_CLASS_DELETE(Settings);
		~Settings() = default;

		static constexpr const char* GetEngineName() noexcept { return ENGINE_NAME; }
		static constexpr uint32_t GetEngineVersion() noexcept { return ENGINE_VERSION; }
		static constexpr PixelFormat GetBackbufferFormat() noexcept { return BACKBUFFER_FORMAT; }
		static constexpr U64 GetFrameIndex() noexcept { return frameIndex; }
		static constexpr void AdvanceFrame() noexcept { ++frameIndex; }

#ifdef _ZE_MODE_DEBUG
		static constexpr void SetGfxTags(bool enabled) noexcept { enableGfxTags = enabled; }
		static constexpr bool GfxTagsActive() noexcept { return enableGfxTags; }
#endif

		static constexpr void SetGpuVendor(GFX::VendorGPU vendor) noexcept { gpuVendor = vendor; }
		static constexpr GFX::VendorGPU GetGpuVendor() noexcept { return gpuVendor; }
		static constexpr GfxApiType GetGfxApi() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return gfxApi; }
		static constexpr const char* GetAppName() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return applicationName; }
		static constexpr uint32_t GetAppVersion() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return applicationVersion; }
		static constexpr U32 GetCurrentBackbufferIndex() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return frameIndex % swapChainBufferCount; }
		static constexpr U32 GetBackbufferCount() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return swapChainBufferCount; }
		static constexpr U64 GetCurrentChainResourceIndex() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return frameIndex % GetChainResourceCount(); }
		static constexpr U64 GetChainResourceCount() noexcept;

		static constexpr void Destroy() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); swapChainBufferCount = 0; }
		static constexpr void Init(GfxApiType type, U32 backBufferCount, const char* appName, uint32_t appVersion) noexcept;
	};

#pragma region Functions
	constexpr U64 Settings::GetChainResourceCount() noexcept
	{
		ZE_ASSERT(Initialized(), "Not initialized!");

		switch (GetGfxApi())
		{
		default:
			ZE_ASSERT(false, "Unhandled enum value!");
		case GfxApiType::DX11:
		case GfxApiType::OpenGL:
			return 1;
		case GfxApiType::DX12:
		case GfxApiType::Vulkan:
			return swapChainBufferCount;
		}
	}

	constexpr void Settings::Init(GfxApiType type, U32 backBufferCount, const char* appName, uint32_t appVersion) noexcept
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