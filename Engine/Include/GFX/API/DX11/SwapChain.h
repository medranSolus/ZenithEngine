#pragma once
#include "Window/Platform/WindowWinAPI.h"
#include "GFX/Device.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11
{
	class SwapChain final
	{
		UINT presentFlags = 0;
		DX::ComPtr<IDXGISwapChain4> swapChain;

	public:
		SwapChain() = default;
		SwapChain(const Window::WinAPI::WindowWinAPI& window, GFX::Device& dev);
		ZE_CLASS_MOVE(SwapChain);
		~SwapChain() = default;

		constexpr void PrepareBackbuffer(GFX::Device& dev, GFX::CommandList& cl) const {}

		void Present(GFX::Device& dev) const;
	};
}