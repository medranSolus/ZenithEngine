#pragma once
#include "GFX/API/ApiType.h"

namespace ZE
{
	// Global engine settings
	class Settings final
	{
		static constexpr PixelFormat BACKBUFFER_FORMAT = PixelFormat::R8G8B8A8_UNorm;
		static inline Settings* instance = nullptr;

		GfxApiType gfxApi;
		U32 backBufferCount;

		Settings(GfxApiType type, U32 backBufferCount) noexcept
			: gfxApi(type), backBufferCount(backBufferCount) { ZE_ASSERT(backBufferCount > 1 && backBufferCount < 17, "Incorrect params!"); }

	public:
		ZE_CLASS_DELETE(Settings);
		~Settings() = default;

		static constexpr PixelFormat GetBackbufferFormat() noexcept { return BACKBUFFER_FORMAT; }
		static constexpr U32 GetBackbufferCount() noexcept { ZE_ASSERT(instance, "Not initialized!"); return instance->backBufferCount; }
		static constexpr GfxApiType GetGfxApi() noexcept { ZE_ASSERT(instance, "Not initialized!"); return instance->gfxApi; }
		static void Init(GfxApiType type, U32 backBufferCount) noexcept { ZE_ASSERT(!instance, "Already initialized!"); instance = new Settings(type, backBufferCount); }
	};
}