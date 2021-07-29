#pragma once
#include "D3D12.h"

namespace ZE::GFX::API::DX12
{
	class Device final
	{
#ifdef _ZE_MODE_DEBUG
		DX::DebugInfoManager debugManager;
#endif
		DX::ComPtr<ID3D12Device8> device;
		DX::ComPtr<ID3D12CommandQueue> mainQueue;
		U64 mainFenceVal = 0;
		DX::ComPtr<ID3D12Fence1> mainFence;
		DX::ComPtr<ID3D12CommandQueue> computeQueue;
		U64 computeFenceVal = 0;
		DX::ComPtr<ID3D12Fence1> computeFence;
		DX::ComPtr<ID3D12CommandQueue> copyQueue;
		U64 copyFenceVal = 0;
		DX::ComPtr<ID3D12Fence1> copyFence;

	public:
		Device();
		Device(Device&&) = delete;
		Device(const Device&) = delete;
		Device& operator=(Device&&) = delete;
		Device& operator=(const Device&) = delete;
		~Device();

#ifdef _ZE_MODE_DEBUG
		constexpr DX::DebugInfoManager& GetInfoManager() noexcept { return debugManager; }
#endif
		ID3D12Device8* GetDevice() const noexcept { return device.Get(); }
		ID3D12CommandQueue* GetMainQueue() const noexcept { return mainQueue.Get(); }
		ID3D12CommandQueue* GetComputeQueue() const noexcept { return computeQueue.Get(); }
		ID3D12CommandQueue* GetCopyQueue() const noexcept { return copyQueue.Get(); }
	};
}