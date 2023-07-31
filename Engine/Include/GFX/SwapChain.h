#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/SwapChain.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/SwapChain.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/SwapChain.h"
#endif

namespace ZE::GFX
{
	// Managing backbuffers
	class SwapChain final
	{
		ZE_RHI_BACKEND(SwapChain);

	public:
		SwapChain() = default;
		ZE_CLASS_MOVE(SwapChain);
		~SwapChain() = default;

		constexpr void Init(const Window::MainWindow& window, Device& dev, bool shaderInput) { ZE_RHI_BACKEND_VAR.Init(window, dev, shaderInput); }
		constexpr void SwitchApi(GfxApiType nextApi, const Window::MainWindow& window, Device& dev, bool shaderInput) { ZE_RHI_BACKEND_VAR.Switch(nextApi, window, dev, shaderInput); }
		ZE_RHI_BACKEND_GET(SwapChain);

		// Main Gfx API

		constexpr void StartFrame(Device& dev) { ZE_RHI_BACKEND_CALL(StartFrame, dev); }
		constexpr void Present(Device& dev) const { ZE_RHI_BACKEND_CALL(Present, dev); }
		constexpr void PrepareBackbuffer(Device& dev, CommandList& cl) const { ZE_RHI_BACKEND_CALL(PrepareBackbuffer, dev, cl); }
		// Have to be called before destroying the SwapChain
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
	};
}