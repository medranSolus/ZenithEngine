#pragma once
#include "GFX/API/DX11/Device.h"
#include "GFX/API/DX12/Device.h"
#include "GFX/API/Backend.h"

namespace ZE::GFX
{
	// Resource allocation
	class Device final
	{
		ZE_API_BACKEND(Device) backend;

	public:
		Device() = default;
		Device(Device&&) = default;
		Device(const Device&) = delete;
		Device& operator=(Device&&) = default;
		Device& operator=(const Device&) = delete;
		~Device() = default;

		constexpr void Init() { backend.Init(); }
		constexpr void SwitchApi(GfxApiType nextApi) { backend.Switch(nextApi); }
		constexpr ZE_API_BACKEND(Device)& Get() noexcept { return backend; }
	};
}