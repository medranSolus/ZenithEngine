#pragma once
#include "GFX/API/ApiType.h"

namespace ZE
{
	// Global engine settings
	class Settings final
	{
		static constexpr PixelFormat BACKBUFFER_FORMAT = PixelFormat::R8G8B8A8_UNorm;
		static inline U64 frameIndex = 0;
		static inline U32 swapChainBufferCount = 0;
		static inline GfxApiType gfxApi;
#ifdef _ZE_MODE_DEBUG
		static inline bool enableGfxTags = true;
#endif

		static constexpr bool Initialized() noexcept { return swapChainBufferCount != 0; }

	public:
		ZE_CLASS_DELETE(Settings);
		~Settings() = default;

		static constexpr PixelFormat GetBackbufferFormat() noexcept { return BACKBUFFER_FORMAT; }
		static constexpr void AdvanceFrame() noexcept { ++frameIndex; }
		static constexpr U64 GetFrameIndex() noexcept { return frameIndex; }

#ifdef _ZE_MODE_DEBUG
		static constexpr void SetGfxTags(bool enabled) noexcept { enableGfxTags = enabled; }
		static constexpr bool GfxTagsActive() noexcept { return enableGfxTags; }
#endif

		static constexpr U64 GetCurrentChainResourceIndex() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return frameIndex % GetChainResourceCount(); }
		static constexpr U64 GetChainResourceCount() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return swapChainBufferCount; }
		static constexpr U32 GetCurrentBackbufferIndex() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return frameIndex % swapChainBufferCount; }
		static constexpr U32 GetBackbufferCount() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return swapChainBufferCount; }
		static constexpr GfxApiType GetGfxApi() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); return gfxApi; }

		static constexpr void Destroy() noexcept { ZE_ASSERT(Initialized(), "Not initialized!"); swapChainBufferCount = 0; }
		static constexpr void Init(GfxApiType type, U32 backBufferCount) noexcept;
	};

#pragma region Functions
	constexpr void Settings::Init(GfxApiType type, U32 backBufferCount) noexcept
	{
		ZE_ASSERT(!Initialized(), "Already initialized!");
		ZE_ASSERT(backBufferCount > 1 && backBufferCount < 17, "Incorrect params!");

		gfxApi = type;
		swapChainBufferCount = backBufferCount;
	}
#pragma endregion
}