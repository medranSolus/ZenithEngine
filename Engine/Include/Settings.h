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

		Settings(GfxApiType type, U32 backBufferCount) noexcept : gfxApi(type), backBufferCount(backBufferCount) { assert(backBufferCount > 1 && backBufferCount < 17); }

	public:
		ZE_CLASS_DELETE(Settings);
		~Settings() = default;

		static constexpr PixelFormat GetBackbufferFormat() noexcept { return BACKBUFFER_FORMAT; }
		static constexpr U32 GetBackbufferCount() noexcept { assert(instance); return instance->backBufferCount; }
		static constexpr GfxApiType GetGfxApi() noexcept { assert(instance); return instance->gfxApi; }
		static void Init(GfxApiType type, U32 backBufferCount) noexcept { assert(!instance); instance = new Settings(type, backBufferCount); }
	};
}