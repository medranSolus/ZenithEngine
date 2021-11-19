#pragma once
#include "Window/Platform/WindowWinAPI.h"
#include "GFX/Device.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12
{
	class SwapChain final
	{
		UINT presentFlags = 0;
		DX::ComPtr<IDXGISwapChain4> swapChain;
		DX::ComPtr<ID3D12DescriptorHeap> rtvDescHeap;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>* rtvSrv = nullptr;

	public:
		SwapChain(const Window::WinAPI::WindowWinAPI& window, GFX::Device& dev);
		SwapChain(SwapChain&&) = default;
		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(SwapChain&&) = default;
		SwapChain& operator=(const SwapChain&) = delete;
		~SwapChain();

		void Present(GFX::Device& dev) const;

		// Gfx API Internal

		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE> GetCurrentBackbuffer(GFX::Device& dev, DX::ComPtr<ID3D12Resource>& buffer) const;
	};
}