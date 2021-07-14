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

	public:
		SwapChain(const Window::WinAPI::WindowWinAPI& window, GFX::Device& dev);
		SwapChain(SwapChain&&) = delete;
		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(SwapChain&&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;
		~SwapChain() = default;

		void Present(GFX::Device& dev) const;
	};
}