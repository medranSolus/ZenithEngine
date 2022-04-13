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
		DX::ComPtr<ID3D11RenderTargetView1> rtv;
		DX::ComPtr<ID3D11ShaderResourceView1> srv;

	public:
		SwapChain() = default;
		SwapChain(const Window::WinAPI::WindowWinAPI& window, GFX::Device& dev, bool shaderInput);
		ZE_CLASS_MOVE(SwapChain);
		~SwapChain() = default;

		constexpr void PrepareBackbuffer(GFX::Device& dev, GFX::CommandList& cl) const {}

		void Present(GFX::Device& dev) const;

		// Gfx API Internal

		DX::ComPtr<ID3D11RenderTargetView1> GetRTV() const noexcept { return rtv; }
		DX::ComPtr<ID3D11ShaderResourceView1> GetSRV() const noexcept { return srv; }
	};
}