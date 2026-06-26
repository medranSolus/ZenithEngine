#pragma once
#include "GFX/Device.h"

namespace ZE::RHI::DX11
{
	class SwapChain final
	{
		UINT presentFlags = 0;
		DX::ComPtr<DX::ISwapChain> swapChain;
		DX::ComPtr<IResource> backBuffer;
		DX::ComPtr<IRenderTargetView> rtv;
		DX::ComPtr<IShaderResourceView> srv;

	public:
		SwapChain() = default;
		ZE_CLASS_MOVE(SwapChain);
		~SwapChain() = default;

		static Expected<SwapChain> Create(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput) noexcept;

		constexpr void StartFrame(GFX::Device& dev) noexcept {}

		Status Present(GFX::Device& dev) const noexcept;

		// Gfx API Internal

		DX::ComPtr<IResource> GetBuffer() const noexcept { return backBuffer; }
		DX::ComPtr<IRenderTargetView> GetRTV() const noexcept { return rtv; }
		DX::ComPtr<IShaderResourceView> GetSRV() const noexcept { return srv; }
	};
}