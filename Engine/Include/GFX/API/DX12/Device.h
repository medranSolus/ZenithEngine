#pragma once
#include "GFX/Resource/Texture/Type.h"
#include "GFX/CommandList.h"
#include "Window/MainWindow.h"
#include "AllocatorTier1.h"
#include "AllocatorTier2.h"
#include "DescriptorInfo.h"
#include "WarningGuardOn.h"
#include "amd_ags.h"
#include "WarningGuardOff.h"

namespace ZE::GFX::API::DX12
{
	class Device final
	{
	public:
		enum class AllocTier : bool { Tier1, Tier2 };

	private:
		static constexpr U16 COPY_LIST_GROW_SIZE = 5;

		struct UploadInfo
		{
			D3D12_RESOURCE_STATES FinalState;
			ID3D12Resource* Destination;
			DX::ComPtr<ID3D12Resource> UploadRes;
		};

#if _ZE_DEBUG_GFX_API
		DX::DebugInfoManager debugManager;
#endif
		DX::ComPtr<ID3D12Device8> device;
		DX::ComPtr<ID3D12CommandQueue> mainQueue;
		DX::ComPtr<ID3D12CommandQueue> computeQueue;
		DX::ComPtr<ID3D12CommandQueue> copyQueue;

		U32 commandListsCount = 0;
		Ptr<ID3D12CommandList*> commandLists = nullptr;

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
		U16 copyOffset = 0;
		Ptr<UploadInfo> copyResList = nullptr;

		U32 dynamicDescStart = 0;
		U32 dynamicDescCount = 0;
		U32 scratchDescStart;
		U32 descriptorCount;
		U32 descriptorSize;
		DX::ComPtr<ID3D12DescriptorHeap> descHeap;

		// Hardware specific data
		union
		{
			AGSContext* gpuCtxAMD;
		};

		void WaitCPU(ID3D12Fence1* fence, U64 val);
		void WaitGPU(ID3D12Fence1* fence, ID3D12CommandQueue* queue, U64 val);
		U64 SetFenceCPU(ID3D12Fence1* fence, UA64& fenceVal);
		U64 SetFenceGPU(ID3D12Fence1* fence, ID3D12CommandQueue* queue, UA64& fenceVal);
		void Execute(ID3D12CommandQueue* queue, CommandList& cl) noexcept(!_ZE_DEBUG_GFX_API);

	public:
		Device() noexcept {}
		Device(const Window::MainWindow& window, U32 descriptorCount, U32 scratchDescriptorCount);
		ZE_CLASS_DELETE(Device);
		~Device();

		constexpr std::pair<U32, U32> GetData() const noexcept { return { descriptorCount, descriptorCount - scratchDescStart }; }
		constexpr U32 GetCommandBufferSize() const noexcept { return commandListsCount; }

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

#if _ZE_GFX_MARKERS
		void TagBeginMain(const wchar_t* tag, Pixel color) const noexcept { PIXBeginEvent(mainQueue.Get(), PIX_COLOR(color.Red, color.Blue, color.Green), tag); }
		void TagBeginCompute(const wchar_t* tag, Pixel color) const noexcept { PIXBeginEvent(computeQueue.Get(), PIX_COLOR(color.Red, color.Blue, color.Green), tag); }
		void TagBeginCopy(const wchar_t* tag, Pixel color) const noexcept { PIXBeginEvent(copyQueue.Get(), PIX_COLOR(color.Red, color.Blue, color.Green), tag); }

		void TagEndMain() const noexcept { PIXEndEvent(mainQueue.Get()); }
		void TagEndCompute() const noexcept { PIXEndEvent(computeQueue.Get()); }
		void TagEndCopy() const noexcept { PIXEndEvent(copyQueue.Get()); }
#endif

		void BeginUploadRegion();
		void StartUpload();
		void EndUploadRegion();
		void Execute(GFX::CommandList* cls, U32 count) noexcept(!_ZE_DEBUG_GFX_API);
		void ExecuteMain(GFX::CommandList& cl) noexcept(!_ZE_DEBUG_GFX_API);
		void ExecuteCompute(GFX::CommandList& cl) noexcept(!_ZE_DEBUG_GFX_API);
		void ExecuteCopy(GFX::CommandList& cl) noexcept(!_ZE_DEBUG_GFX_API);

		// Gfx API Internal

#if _ZE_DEBUG_GFX_API
		constexpr DX::DebugInfoManager& GetInfoManager() noexcept { return debugManager; }
#endif
		constexpr AllocTier GetCurrentAllocTier() const noexcept { return allocTier; }
		constexpr U32 GetDescriptorSize() const noexcept { return descriptorSize; }
		constexpr const DX::ComPtr<ID3D12Device8>& GetDev() const noexcept { return device; }

		ID3D12Device8* GetDevice() const noexcept { return device.Get(); }
		ID3D12CommandQueue* GetQueueMain() const noexcept { return mainQueue.Get(); }
		ID3D12CommandQueue* GetQueueCompute() const noexcept { return computeQueue.Get(); }
		ID3D12CommandQueue* GetQueueCopy() const noexcept { return copyQueue.Get(); }
		ID3D12DescriptorHeap* GetDescHeap() const noexcept { return descHeap.Get(); }
		AGSContext* GetAGSContext() const noexcept { ZE_ASSERT(Settings::GetGpuVendor() == VendorGPU::AMD, "Wrong active GPU!"); return gpuCtxAMD; }

		D3D12_RESOURCE_DESC GetBufferDesc(U64 size) const noexcept;
		std::pair<D3D12_RESOURCE_DESC, U32> GetTextureDesc(U32 width, U32 height, U16 count,
			DXGI_FORMAT format, GFX::Resource::Texture::Type type) const noexcept;

		ResourceInfo CreateBuffer(const D3D12_RESOURCE_DESC& desc, bool dynamic);
		ResourceInfo CreateTexture(const std::pair<D3D12_RESOURCE_DESC, U32>& desc);
		DX::ComPtr<ID3D12Resource> CreateTextureUploadBuffer(U64 size);

		void UploadBuffer(ID3D12Resource* dest, const D3D12_RESOURCE_DESC& desc,
			const void* data, U64 size, D3D12_RESOURCE_STATES finalState);
		void UploadTexture(const D3D12_TEXTURE_COPY_LOCATION& dest,
			const D3D12_TEXTURE_COPY_LOCATION& source, D3D12_RESOURCE_STATES finalState);

		void UpdateBuffer(ID3D12Resource* res, const void* data,
			U64 size, D3D12_RESOURCE_STATES currentState);

		void FreeBuffer(ResourceInfo& info) noexcept;
		void FreeDynamicBuffer(ResourceInfo& info) noexcept;
		void FreeTexture(ResourceInfo& info) noexcept;

		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> AddStaticDescs(U32 count) noexcept;
		DescriptorInfo AllocDescs(U32 count) noexcept;
	};
}