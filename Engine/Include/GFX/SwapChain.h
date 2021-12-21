#pragma once
#include "API/DX11/SwapChain.h"
#include "API/DX12/SwapChain.h"
#include "Device.h"

namespace ZE::GFX
{
	// Managing backbuffers
	class SwapChain final
	{
		ZE_API_BACKEND(SwapChain);

	public:
		SwapChain() = default;
		ZE_CLASS_MOVE(SwapChain);
		~SwapChain() = default;

		constexpr void Init(const Window::MainWindow& window, Device& dev) { ZE_API_BACKEND_VAR.Init(window, dev); }
		constexpr void SwitchApi(GfxApiType nextApi, const Window::MainWindow& window, Device& dev) { ZE_API_BACKEND_VAR.Switch(nextApi, window, dev); }
		ZE_API_BACKEND_GET(SwapChain);

		// Main Gfx API

		constexpr void Present(Device& dev) const { ZE_API_BACKEND_CALL(Present, dev); }
		constexpr void PrepareBackbuffer(Device& dev, CommandList& cl) const { ZE_API_BACKEND_CALL(PrepareBackbuffer, dev, cl); }
	};
}