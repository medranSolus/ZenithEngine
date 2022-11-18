#pragma once
#include "GFX/Device.h"

namespace ZE::GFX::API::DX12
{
	class SwapChain final
	{
		UINT presentFlags = 0;
		DX::ComPtr<IDXGISwapChain4> swapChain;
		DX::ComPtr<ID3D12DescriptorHeap> rtvDescHeap;
		Ptr<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>> rtvSrv;
		D3D12_RESOURCE_BARRIER presentBarrier;

	public:
		SwapChain() = default;
		SwapChain(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput);
		ZE_CLASS_MOVE(SwapChain);
		~SwapChain();

		void Free(GFX::Device& dev) noexcept { rtvDescHeap = nullptr; swapChain = nullptr; }

		void Present(GFX::Device& dev) const;
		void PrepareBackbuffer(GFX::Device& dev, GFX::CommandList& cl) const;

		// Gfx API Internal

		constexpr const D3D12_RESOURCE_BARRIER& GetPresentBarrier() const noexcept { return presentBarrier; }
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTV() const noexcept { return rtvSrv[Settings::GetCurrentBackbufferIndex()].first; }

		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> SetCurrentBackbuffer(GFX::Device& dev, DX::ComPtr<ID3D12Resource>& buffer);
	};
}