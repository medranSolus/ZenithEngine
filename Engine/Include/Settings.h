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

		Settings(GfxApiType type) noexcept : gfxApi(type) {}

	public:
		Settings(Settings&&) = delete;
		Settings(const Settings&) = delete;
		Settings& operator=(Settings&&) = delete;
		Settings& operator=(const Settings&) = delete;
		~Settings() = default;

		static constexpr PixelFormat GetBackbufferFormat() noexcept { return BACKBUFFER_FORMAT; }
		static constexpr GfxApiType GetGfxApi() noexcept { assert(instance); return instance->gfxApi; }
		static void Init(GfxApiType type) noexcept { assert(!instance); instance = new Settings(type); }
	};
}