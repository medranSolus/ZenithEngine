#pragma once
#include "GFX/API/DX11/Device.h"
#include "GFX/API/Backend.h"

namespace ZE::GFX
{
	// Resource allocation
	class Device final
	{
		ZE_API_BACKEND(Device, DX11, DX11, DX11, DX11) backend;

	public:
		Device() = default;
		Device(Device&&) = delete;
		Device(const Device&) = delete;
		Device& operator=(Device&&) = delete;
		Device& operator=(const Device&) = delete;
		~Device() = default;

		constexpr void Init() { backend.Init(); }
		constexpr void SwitchApi(GfxApiType nextApi) { backend.Switch(nextApi); }
		constexpr ZE_API_BACKEND(Device, DX11, DX11, DX11, DX11)& Get() noexcept { return backend; }
	};
}