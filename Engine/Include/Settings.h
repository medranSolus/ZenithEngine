#pragma once
#include "GFX/API/Factory.h"

namespace ZE
{
	// Global engine settings
	class Settings final
	{
		static inline Settings* instance = nullptr;

		GFX::API::Factory backendFactory;

		Settings(GFX::API::Backend type) : backendFactory(type) {}

	public:
		Settings(Settings&&) = delete;
		Settings(const Settings&) = delete;
		Settings& operator=(Settings&&) = delete;
		Settings& operator=(const Settings&) = delete;
		~Settings() = default;

		static constexpr GFX::API::Factory& GetGfxApi() noexcept { assert(instance); return instance->backendFactory; }
		static void Init(GFX::API::Backend type) { assert(!instance); instance = new Settings(type); }
	};
}