#pragma once
#include "GFX/CommandList.h"

namespace ZE::RHI::DX12
{
	class SwapChain final
	{
	public:
		struct DescEntry
		{
			D3D12_CPU_DESCRIPTOR_HANDLE RTV;
			D3D12_CPU_DESCRIPTOR_HANDLE SRVCpu;
			D3D12_GPU_DESCRIPTOR_HANDLE SRVGpu;
		};

	private:
		UINT presentFlags = 0;
		DX::ComPtr<DX::ISwapChain> swapChain;
		DX::ComPtr<IDescriptorHeap> rtvDescHeap;
		Ptr<DescEntry> rtvSrv;
		D3D12_RESOURCE_BARRIER presentBarrier;
		DescriptorInfo srvHandle;

	public:
		SwapChain() = default;
		SwapChain(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput);
		ZE_CLASS_MOVE(SwapChain);
		~SwapChain();

		constexpr void StartFrame(GFX::Device& dev) {}

		void Present(GFX::Device& dev) const;
		void PrepareBackbuffer(GFX::Device& dev, GFX::CommandList& cl) const;
		void Free(GFX::Device& dev) noexcept;

		// Gfx API Internal

		constexpr const D3D12_RESOURCE_BARRIER& GetPresentBarrier() const noexcept { return presentBarrier; }
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTV() const noexcept { return rtvSrv[Settings::GetCurrentBackbufferIndex()].RTV; }

		DescEntry SetCurrentBackbuffer(GFX::Device& dev, DX::ComPtr<IResource>& buffer);
	};
}