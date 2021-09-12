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

		// Main Gfx API

		constexpr void WaitMain() { ZE_API_BACKEND_CALL(backend, WaitMain); }
		constexpr void WaitCompute() { ZE_API_BACKEND_CALL(backend, WaitCompute); }
		constexpr void WaitCopy() { ZE_API_BACKEND_CALL(backend, WaitCopy); }

		constexpr void FinishUpload() { ZE_API_BACKEND_CALL(backend, FinishUpload); }

		constexpr void ExecuteMain(CommandList& cl) noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, ExecuteMain, cl); }
		constexpr void ExecuteCompute(CommandList& cl) noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, ExecuteCompute, cl); }
		constexpr void ExecuteCopy(CommandList& cl) noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, ExecuteCopy, cl); }
	};
}