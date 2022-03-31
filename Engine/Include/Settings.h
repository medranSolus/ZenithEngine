#pragma once
#include "GFX/API/ApiType.h"

namespace ZE
{
	// Global engine settings
	class Settings final
	{
		static constexpr PixelFormat BACKBUFFER_FORMAT = PixelFormat::R8G8B8A8_UNorm;
		static inline Settings* instance = nullptr;
		static inline U64 frameIndex = 0;

		GfxApiType gfxApi;
		U32 backBufferCount;

		Settings(GfxApiType type, U32 backBufferCount) noexcept : gfxApi(type), backBufferCount(backBufferCount)
		{
			ZE_ASSERT(backBufferCount > 1 && backBufferCount < 17, "Incorrect params!");
		}

	public:
		ZE_CLASS_DELETE(Settings);
		~Settings() = default;

		static constexpr PixelFormat GetBackbufferFormat() noexcept { return BACKBUFFER_FORMAT; }
		static constexpr void AdvanceFrame() noexcept { ++frameIndex; }
		static constexpr U64 GetFrameIndex() noexcept { return frameIndex; }

		static constexpr U64 GetCurrentChainResourceIndex() noexcept { ZE_ASSERT(instance, "Not initialized!"); return frameIndex % instance->backBufferCount; }
		static constexpr U32 GetBackbufferCount() noexcept { ZE_ASSERT(instance, "Not initialized!"); return instance->backBufferCount; }
		static constexpr GfxApiType GetGfxApi() noexcept { ZE_ASSERT(instance, "Not initialized!"); return instance->gfxApi; }

		static void Init(GfxApiType type, U32 backBufferCount) noexcept { ZE_ASSERT(!instance, "Already initialized!"); instance = new Settings(type, backBufferCount); }
		static void Destroy() noexcept { ZE_ASSERT(instance, "Not initialized!"); delete instance; }
	};
}