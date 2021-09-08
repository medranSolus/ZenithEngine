#pragma once
#include "D3D12.h"
#include "AllocatorTier1.h"
#include "AllocatorTier2.h"

namespace ZE::GFX
{
	class CommandList;
}
namespace ZE::GFX::API::DX12
{
	class Device final
	{
		enum class AllocTier : bool { Tier1, Tier2 };
		union AllocVer
		{
			AllocatorTier1 Tier1;
			AllocatorTier2 Tier2;

			constexpr AllocVer() {}
			AllocVer(AllocVer&&) = delete;
			AllocVer(const AllocVer&) = delete;
			AllocVer& operator=(AllocVer&&) = delete;
			AllocVer& operator=(const AllocVer&) = delete;
			~AllocVer() {}
		};

		static constexpr U32 ALLOC_INIT_FREE_LIST_SIZE = 100;

#ifdef _ZE_MODE_DEBUG
		DX::DebugInfoManager debugManager;
#endif
		DX::ComPtr<ID3D12Device8> device;
		DX::ComPtr<ID3D12CommandQueue> mainQueue;
		DX::ComPtr<ID3D12CommandQueue> computeQueue;
		DX::ComPtr<ID3D12CommandQueue> copyQueue;

		UA64 mainFenceVal = 0;
		DX::ComPtr<ID3D12Fence1> mainFence;
		UA64 computeFenceVal = 0;
		DX::ComPtr<ID3D12Fence1> computeFence;
		UA64 copyFenceVal = 0;
		DX::ComPtr<ID3D12Fence1> copyFence;

		AllocVer allocator;
		AllocTier allocTier;

		void Wait(ID3D12Fence1* fence, U64 val);
		void Execute(ID3D12CommandQueue* queue, GFX::CommandList& cl) noexcept(ZE_NO_DEBUG);

	public:
		Device();
		Device(Device&&) = default;
		Device(const Device&) = delete;
		Device& operator=(Device&&) = default;
		Device& operator=(const Device&) = delete;
		~Device();

		void WaitMain() { Wait(mainFence.Get(), mainFenceVal); }
		void WaitCompute() { Wait(computeFence.Get(), computeFenceVal); }
		void WaitCopy() { Wait(copyFence.Get(), copyFenceVal); }

		void ExecuteMain(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG) { Execute(mainQueue.Get(), cl); }
		void ExecuteCompute(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG) { Execute(computeQueue.Get(), cl); }
		void ExecuteCopy(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG) { Execute(copyQueue.Get(), cl); }

		// Gfx API Internal

#ifdef _ZE_MODE_DEBUG
		constexpr DX::DebugInfoManager& GetInfoManager() noexcept { return debugManager; }
#endif
		ID3D12Device8* GetDevice() const noexcept { return device.Get(); }
		ID3D12CommandQueue* GetQueueMain() const noexcept { return mainQueue.Get(); }
		ID3D12CommandQueue* GetQueueCompute() const noexcept { return computeQueue.Get(); }
		ID3D12CommandQueue* GetQueueCopy() const noexcept { return copyQueue.Get(); }

		ResourceInfo CreateBuffer(U32 size);
		ResourceInfo CreateTexture(U32 width, U32 height, DXGI_FORMAT format);
		void FreeBuffer(const ResourceInfo& info) noexcept;
		void FreeTexture(const ResourceInfo& info) noexcept;
	};
}