#pragma once
#include "API/DX11/SwapChain.h"
#include "Device.h"

namespace ZE::GFX
{
	// Managing backbuffers
	class SwapChain final
	{
		ZE_API_BACKEND(SwapChain, DX11, DX11, DX11, DX11) backend;

	public:
		SwapChain() = default;
		SwapChain(SwapChain&&) = delete;
		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(SwapChain&&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;
		~SwapChain() = default;

		constexpr void Init(const Window::MainWindow& window, Device& dev) { backend.Init(window, dev); }
		constexpr void SwitchApi(GfxApiType nextApi, const Window::MainWindow& window, Device& dev) { backend.Switch(nextApi, window, dev); }

		void Present(Device& dev) const { ZE_API_BACKEND_CALL(backend, Present, dev); }
	};
}