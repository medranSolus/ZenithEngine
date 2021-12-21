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
		static constexpr U16 COPY_LIST_GROW_SIZE = 5;

#ifdef _ZE_MODE_DEBUG
		DX::DebugInfoManager debugManager;
#endif
		DX::ComPtr<ID3D12Device8> device;
		DX::ComPtr<ID3D12CommandQueue> mainQueue;
		DX::ComPtr<ID3D12CommandQueue> computeQueue;
		DX::ComPtr<ID3D12CommandQueue> copyQueue;

		U32 commandListsCount = 0;
		ID3D12CommandList** commandLists = nullptr;

		UA64 mainFenceVal = 0;
		DX::ComPtr<ID3D12Fence1> mainFence;
		UA64 computeFenceVal = 0;
		DX::ComPtr<ID3D12Fence1> computeFence;
		UA64 copyFenceVal = 0;
		DX::ComPtr<ID3D12Fence1> copyFence;

		AllocTier allocTier;
		union
		{
			AllocatorTier1 allocTier1;
			AllocatorTier2 allocTier2;
		};

		CommandList copyList;
		TableInfo<U16> copyResInfo;
		DX::ComPtr<ID3D12Resource>* copyResList = nullptr;

		U32 dynamicDescStart = 0;
		U32 scratchDescStart;
		U32 descriptorCount;
		U32 descriptorSize;
		DX::ComPtr<ID3D12DescriptorHeap> descHeap;

		void WaitCPU(ID3D12Fence1* fence, U64 val);
		void WaitGPU(ID3D12Fence1* fence, ID3D12CommandQueue* queue, U64 val);
		U64 SetFenceCPU(ID3D12Fence1* fence, UA64& fenceVal);
		U64 SetFenceGPU(ID3D12Fence1* fence, ID3D12CommandQueue* queue, UA64& fenceVal);
		void Execute(ID3D12CommandQueue* queue, CommandList& cl) noexcept(ZE_NO_DEBUG);

	public:
		Device(U32 descriptorCount, U32 scratchDescriptorCount);
		ZE_CLASS_DELETE(Device);
		~Device();

		constexpr std::pair<U32, U32> GetData() const noexcept { return { descriptorCount, descriptorCount - scratchDescStart }; }
		constexpr U32 GetCommandBufferSize() const noexcept { return commandListsCount; }
		constexpr void SetCommandBufferSize(U32 count) noexcept { commandLists = new ID3D12CommandList * [3 * static_cast<U64>(count)]; commandListsCount = count; }

		U64 GetMainFence() const noexcept { return mainFenceVal; }
		U64 GetComputeFence() const noexcept { return computeFenceVal; }
		U64 GetCopyFence() const noexcept { return copyFenceVal; }

		void WaitMain(U64 val) { WaitCPU(mainFence.Get(), val); }
		void WaitCompute(U64 val) { WaitCPU(computeFence.Get(), val); }
		void WaitCopy(U64 val) { WaitCPU(copyFence.Get(), val); }

		U64 SetMainFenceCPU() { return SetFenceCPU(mainFence.Get(), mainFenceVal); }
		U64 SetComputeFenceCPU() { return SetFenceCPU(computeFence.Get(), computeFenceVal); }
		U64 SetCopyFenceCPU() { return SetFenceCPU(copyFence.Get(), copyFenceVal); }

		void WaitMainFromCompute(U64 val) { WaitGPU(computeFence.Get(), mainQueue.Get(), val); }
		void WaitMainFromCopy(U64 val) { WaitGPU(copyFence.Get(), mainQueue.Get(), val); }
		void WaitComputeFromMain(U64 val) { WaitGPU(mainFence.Get(), computeQueue.Get(), val); }
		void WaitComputeFromCopy(U64 val) { WaitGPU(copyFence.Get(), computeQueue.Get(), val); }
		void WaitCopyFromMain(U64 val) { WaitGPU(mainFence.Get(), copyQueue.Get(), val); }
		void WaitCopyFromCompute(U64 val) { WaitGPU(computeFence.Get(), copyQueue.Get(), val); }

		U64 SetMainFence() { return SetFenceGPU(mainFence.Get(), mainQueue.Get(), mainFenceVal); }
		U64 SetComputeFence() { return SetFenceGPU(computeFence.Get(), computeQueue.Get(), computeFenceVal); }
		U64 SetCopyFence() { return SetFenceGPU(copyFence.Get(), copyQueue.Get(), copyFenceVal); }

		void FinishUpload();
		void Execute(GFX::CommandList* cls, U32 count) noexcept(ZE_NO_DEBUG);
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
		ID3D12DescriptorHeap* GetDescHeap() const noexcept { return descHeap.Get(); }

		D3D12_RESOURCE_DESC GetBufferDesc(U32 size);
		ResourceInfo CreateBuffer(const D3D12_RESOURCE_DESC& desc);
		ResourceInfo CreateTexture(U32 width, U32 height, DXGI_FORMAT format);

		void FreeBuffer(ResourceInfo& info) noexcept;
		void FreeBuffer(ResourceInfo& info, U32 size) noexcept;
		void FreeTexture(ResourceInfo& info) noexcept;

		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> AddStaticDescs(U32 count) noexcept;
		void UploadResource(ID3D12Resource* dest, const D3D12_RESOURCE_DESC& desc, void* data, U64 size);
	};
}