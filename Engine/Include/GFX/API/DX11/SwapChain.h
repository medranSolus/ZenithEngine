#pragma once
#include "Window/Platform/WindowWinAPI.h"
#include "GFX/Device.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11
{
	class SwapChain final
	{
		UINT presentFlags = 0;
		DX::ComPtr<IDXGISwapChain4> swapChain = nullptr;

		SwapChain(const Window::WinAPI::WindowWinAPI& window, Device& dev);

	public:
		SwapChain(const Window::WinAPI::WindowWinAPI& window, GFX::Device& dev) : SwapChain(window, dev.Get().dx11) {}
		SwapChain(SwapChain&&) = delete;
		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(SwapChain&&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;
		~SwapChain() = default;

		void Present(GFX::Device& dev) const;
	};
}