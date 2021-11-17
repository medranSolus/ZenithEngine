#pragma once
#include "API/DX11/SwapChain.h"
#include "API/DX12/SwapChain.h"
#include "Device.h"

namespace ZE::GFX
{
	// Managing backbuffers
	class SwapChain final
	{
		ZE_API_BACKEND(SwapChain) backend;

	public:
		SwapChain() = default;
		SwapChain(SwapChain&&) = default;
		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(SwapChain&&) = default;
		SwapChain& operator=(const SwapChain&) = delete;
		~SwapChain() = default;

		constexpr void Init(const Window::MainWindow& window, Device& dev) { backend.Init(window, dev); }
		constexpr void SwitchApi(GfxApiType nextApi, const Window::MainWindow& window, Device& dev) { backend.Switch(nextApi, window, dev); }
		constexpr ZE_API_BACKEND(SwapChain)& Get() noexcept { return backend; }

		// Main Gfx API

		constexpr void Present(Device& dev) const { ZE_API_BACKEND_CALL(backend, Present, dev); }
	};
}