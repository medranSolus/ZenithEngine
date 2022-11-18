#pragma once
#include "API/DX11/SwapChain.h"
#include "API/DX12/SwapChain.h"
#include "API/VK/SwapChain.h"

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

		constexpr void Init(const Window::MainWindow& window, Device& dev, bool shaderInput) { ZE_API_BACKEND_VAR.Init(window, dev, shaderInput); }
		constexpr void SwitchApi(GfxApiType nextApi, const Window::MainWindow& window, Device& dev, bool shaderInput) { ZE_API_BACKEND_VAR.Switch(nextApi, window, dev, shaderInput); }
		ZE_API_BACKEND_GET(SwapChain);

		// Main Gfx API

		constexpr void Present(Device& dev) const { ZE_API_BACKEND_CALL(Present, dev); }
		constexpr void PrepareBackbuffer(Device& dev, CommandList& cl) const { ZE_API_BACKEND_CALL(PrepareBackbuffer, dev, cl); }
		// Have to be called before destroying the SwapChain
		constexpr void Free(Device& dev) noexcept { ZE_API_BACKEND_CALL(Free, dev); }
	};
}