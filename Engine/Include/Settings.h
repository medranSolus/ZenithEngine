#pragma once
#include "GFX/API/ApiType.h"

namespace ZE
{
	// Global engine settings
	class Settings final
	{
		static inline Settings* instance = nullptr;

		GfxApiType gfxApi;

		Settings(GfxApiType type) noexcept : gfxApi(type) {}

	public:
		Settings(Settings&&) = delete;
		Settings(const Settings&) = delete;
		Settings& operator=(Settings&&) = delete;
		Settings& operator=(const Settings&) = delete;
		~Settings() = default;

		static constexpr GfxApiType GetGfxApi() noexcept { assert(instance); return instance->gfxApi; }
		static void Init(GfxApiType type) noexcept { assert(!instance); instance = new Settings(type); }
	};
}