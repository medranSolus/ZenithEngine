#pragma once
#include "GFX/Device.h"

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
		std::unique_ptr<DescEntry[]> rtvSrv;
		DescriptorInfo srvHandle = {};

	public:
		SwapChain() = default;
		ZE_CLASS_MOVE(SwapChain);
		~SwapChain();

		static Expected<SwapChain> Create(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput) noexcept;

		constexpr void StartFrame(GFX::Device& dev) noexcept {}

		Status Present(GFX::Device& dev) const noexcept;

		// Gfx API Internal

		constexpr D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTV() const noexcept { return rtvSrv[Settings::GetCurrentBackbufferIndex()].RTV; }

		Expected<DescEntry> GetCurrentBackbuffer(Device& dev, DX::ComPtr<IResource>& buffer) noexcept;
	};
}