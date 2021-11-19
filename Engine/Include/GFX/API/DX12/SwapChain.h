#pragma once
#include "Window/Platform/WindowWinAPI.h"
#include "GFX/CommandList.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12
{
	class SwapChain final
	{
		UINT presentFlags = 0;
		DX::ComPtr<IDXGISwapChain4> swapChain;
		DX::ComPtr<ID3D12DescriptorHeap> rtvDescHeap;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>* rtvSrv = nullptr;
		D3D12_RESOURCE_BARRIER presentBarrier;

	public:
		SwapChain(const Window::WinAPI::WindowWinAPI& window, GFX::Device& dev);
		SwapChain(SwapChain&&) = default;
		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(SwapChain&&) = default;
		SwapChain& operator=(const SwapChain&) = delete;
		~SwapChain();

		void Present(GFX::Device& dev) const;
		void PrepareBackbuffer(GFX::Device& dev, GFX::CommandList& cl) const;

		// Gfx API Internal

		constexpr const D3D12_RESOURCE_BARRIER& GetPresentBarrier() const noexcept { return presentBarrier; }
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTV() const noexcept { return rtvSrv[swapChain->GetCurrentBackBufferIndex()].first; }

		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE> SetCurrentBackbuffer(GFX::Device& dev, DX::ComPtr<ID3D12Resource>& buffer);
	};
}