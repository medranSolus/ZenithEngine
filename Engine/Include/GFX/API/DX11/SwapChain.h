#pragma once
#include "GFX/Device.h"

namespace ZE::GFX::API::DX11
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
		SwapChain(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput);
		ZE_CLASS_MOVE(SwapChain);
		~SwapChain() = default;

		constexpr void StartFrame(GFX::Device& dev) {}
		constexpr void PrepareBackbuffer(GFX::Device& dev, GFX::CommandList& cl) const {}
		void Free(GFX::Device& dev) noexcept { srv = nullptr; rtv = nullptr; backBuffer = nullptr; swapChain = nullptr; }

		void Present(GFX::Device& dev) const;

		// Gfx API Internal

		DX::ComPtr<IResource> GetBuffer() const noexcept { return backBuffer; }
		DX::ComPtr<IRenderTargetView> GetRTV() const noexcept { return rtv; }
		DX::ComPtr<IShaderResourceView> GetSRV() const noexcept { return srv; }
	};
}