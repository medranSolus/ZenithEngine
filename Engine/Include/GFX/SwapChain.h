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

		static Expected<SwapChain> Create(const Window::MainWindow& window, Device& dev, bool shaderInput) { ZE_RHI_BACKEND_CREATE(SwapChain, window, dev, shaderInput); }
		ZE_RHI_BACKEND_GET(SwapChain);

		// Main Gfx API

		constexpr void StartFrame(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(StartFrame, dev); }
		Status Present(Device& dev) const noexcept { ZE_RHI_BACKEND_CALL_RET(Present, dev); }
	};
}