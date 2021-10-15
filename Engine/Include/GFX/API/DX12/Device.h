#pragma once
#include "D3D12.h"
#include "AllocatorTier1.h"
#include "AllocatorTier2.h"
#include "CommandList.h"

namespace ZE::GFX
{
	class CommandList;
}
namespace ZE::GFX::API::DX12
{
	class Device final
	{
	public:
		enum class AllocTier : bool { Tier1, Tier2 };

	private:
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

		static constexpr U16 COPY_LIST_GROW_SIZE = 5;

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

		CommandList copyList;
		TableInfo<U16> copyResInfo;
		DX::ComPtr<ID3D12Resource>* copyResList = nullptr;

		void WaitCPU(ID3D12Fence1* fence, U64 val);
		void WaitGPU(ID3D12Fence1* fence, ID3D12CommandQueue* queue, U64 val);
		U64 SetFence(ID3D12Fence1* fence, ID3D12CommandQueue* queue, UA64& fenceVal);
		void Execute(ID3D12CommandQueue* queue, CommandList& cl) noexcept(ZE_NO_DEBUG);

	public:
		Device();
		Device(Device&&) = default;
		Device(const Device&) = delete;
		Device& operator=(Device&&) = default;
		Device& operator=(const Device&) = delete;
		~Device();

		U64 GetMainFence() const noexcept { return mainFenceVal; }
		U64 GetComputeFence() const noexcept { return computeFenceVal; }
		U64 GetCopyFence() const noexcept { return copyFenceVal; }

		void WaitMain(U64 val) { WaitCPU(mainFence.Get(), val); }
		void WaitCompute(U64 val) { WaitCPU(computeFence.Get(), val); }
		void WaitCopy(U64 val) { WaitCPU(copyFence.Get(), val); }

		void WaitMainFromCompute(U64 val) { WaitGPU(computeFence.Get(), mainQueue.Get(), val); }
		void WaitMainFromCopy(U64 val) { WaitGPU(copyFence.Get(), mainQueue.Get(), val); }
		void WaitComputeFromMain(U64 val) { WaitGPU(mainFence.Get(), computeQueue.Get(), val); }
		void WaitComputeFromCopy(U64 val) { WaitGPU(copyFence.Get(), computeQueue.Get(), val); }
		void WaitCopyFromMain(U64 val) { WaitGPU(mainFence.Get(), copyQueue.Get(), val); }
		void WaitCopyFromCompute(U64 val) { WaitGPU(computeFence.Get(), copyQueue.Get(), val); }

		U64 SetMainFenceFromCompute() { return SetFence(mainFence.Get(), computeQueue.Get(), mainFenceVal); }
		U64 SetMainFenceFromCopy() { return SetFence(mainFence.Get(), copyQueue.Get(), mainFenceVal); }
		U64 SetComputeFenceFromMain() { return SetFence(computeFence.Get(), mainQueue.Get(), computeFenceVal); }
		U64 SetComputeFenceFromCopy() { return SetFence(computeFence.Get(), copyQueue.Get(), computeFenceVal); }
		U64 SetCopyFenceFromMain() { return SetFence(copyFence.Get(), mainQueue.Get(), copyFenceVal); }
		U64 SetCopyFenceFromCompute() { return SetFence(copyFence.Get(), computeQueue.Get(), copyFenceVal); }

		void FinishUpload();
		void ExecuteMain(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG);
		void ExecuteCompute(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG);
		void ExecuteCopy(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG);

		// Gfx API Internal

#ifdef _ZE_MODE_DEBUG
		constexpr DX::DebugInfoManager& GetInfoManager() noexcept { return debugManager; }
#endif
		constexpr AllocTier GetCurrentAllocTier() const noexcept { return allocTier; }

		ID3D12Device8* GetDevice() const noexcept { return device.Get(); }
		ID3D12CommandQueue* GetQueueMain() const noexcept { return mainQueue.Get(); }
		ID3D12CommandQueue* GetQueueCompute() const noexcept { return computeQueue.Get(); }
		ID3D12CommandQueue* GetQueueCopy() const noexcept { return copyQueue.Get(); }

		D3D12_RESOURCE_DESC GetBufferDesc(U32 size);
		ResourceInfo CreateBuffer(const D3D12_RESOURCE_DESC& desc);
		ResourceInfo CreateTexture(U32 width, U32 height, DXGI_FORMAT format);

		void FreeBuffer(ResourceInfo& info) noexcept;
		void FreeBuffer(ResourceInfo& info, U32 size) noexcept;
		void FreeTexture(ResourceInfo& info) noexcept;

		void UploadResource(ID3D12Resource* dest, const D3D12_RESOURCE_DESC& desc, void* data, U64 size);
	};
}