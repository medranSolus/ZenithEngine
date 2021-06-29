#pragma once
#include "GFX/SwapChain.h"
#include "Window/Platform/WindowWinAPI.h"
#include "Device.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11
{
	class SwapChain : public GFX::SwapChain
	{
		UINT presentFlags = 0;
		DX::ComPtr<IDXGISwapChain4> swapChain = nullptr;

	public:
		SwapChain(const Window::WinAPI::WindowWinAPI& window, Device& dev);
		virtual ~SwapChain() = default;

		void Present(GFX::Device& dev) const override;
	};
}