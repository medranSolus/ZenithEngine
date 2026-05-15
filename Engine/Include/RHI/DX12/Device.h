#pragma once
#include "GFX/Resource/Texture/Type.h"
#include "GFX/DisplayProperties.h"
#include "GFX/ShaderModel.h"
#include "Window/MainWindow.h"
#include "AllocatorGPU.h"
#include "CommandList.h"
#include "DescriptorInfo.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_types.h"
#include "amd_ags.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	class CommandList;
}
namespace ZE::RHI::DX12
{
	class Device final
	{
		static_assert(Settings::MAX_RENDER_TARGETS <= D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT, "Incorrect number of max render targets for pass!");

		static constexpr U16 COPY_LIST_GROW_SIZE = 5;
		static constexpr D3D_FEATURE_LEVEL MINIMAL_D3D_LEVEL = D3D_FEATURE_LEVEL_12_1;
		static constexpr U64 BLOCK_DESCRIPTOR_ALLOC_CAPACITY = 1000;
		static constexpr U64 CHUNK_DESCRIPTOR_ALLOC_CAPACITY = 20;
		static constexpr U32 CPU_DESCRIPTOR_CHUNK_SIZE = 1000;

		struct DescHeap
		{
			DX::ComPtr<IDescriptorHeap> Heap = nullptr;

			static Status Init(DescHeap& chunk, Allocator::TLSFMemoryChunkFlags flags, U64 size, void* userData) noexcept;
			static void Destroy(DescHeap& chunk, void* userData) noexcept { chunk.Heap = nullptr; }
		};
		typedef Allocator::ChunkedTLSF<DescHeap, 4, 2> DescriptorAllocator;

#if _ZE_DEBUG_GFX_API
		DX::DebugInfoManager debugManager;
#endif
		DX::ComPtr<IDevice> device;
		DX::ComPtr<ICommandQueue> mainQueue;
		DX::ComPtr<ICommandQueue> computeQueue;
		DX::ComPtr<ICommandQueue> copyQueue;

		UA64 mainFenceVal = 0;
		DX::ComPtr<IFence> mainFence;
		UA64 computeFenceVal = 0;
		DX::ComPtr<IFence> computeFence;
		UA64 copyFenceVal = 0;
		DX::ComPtr<IFence> copyFence;

		AllocatorGPU allocator;

		std::shared_ptr<DescriptorAllocator::BlockAllocator> blockDescAllocator;
		std::shared_ptr<DescriptorAllocator::ChunkAllocator> chunkDescAllocator;
		DescriptorAllocator descriptorGpuAllocator;
		DescriptorAllocator descriptorCpuAllocator;
		U32 descriptorSize = 0;

		// Hardware specific data
		union HWData
		{
			AGSContext* AMD;

			constexpr HWData() noexcept : AMD(nullptr) {}
			ZE_CLASS_NO_COPY(HWData);
			constexpr HWData(HWData&& hw) noexcept : AMD(hw.AMD) { hw.AMD = nullptr; }
			constexpr HWData& operator=(HWData&& hw) noexcept { AMD = hw.AMD; hw.AMD = nullptr; return *this; }
			~HWData() {}

		} gpuCtx = {};
#if !_ZE_MODE_RELEASE
		HMODULE pixCapturer = nullptr;
#endif
		bool featureExistingHeap = false;
		GFX::DisplayProperties displayProps = {};

		static Status WaitCPU(IFence* fence, U64 val) noexcept;
		static Status WaitGPU(IFence* fence, ICommandQueue* queue, U64 val) noexcept;
		static Expected<U64> SetFenceCPU(IFence* fence, UA64& fenceVal) noexcept;
		static Expected<U64> SetFenceGPU(IFence* fence, ICommandQueue* queue, UA64& fenceVal) noexcept;
		static void Execute(ICommandQueue* queue, CommandList& cl) noexcept;

		void MoveFrom(Device& dev) noexcept;

	public:
		Device() noexcept;
		ZE_CLASS_NO_COPY(Device);
		Device(Device&& dev) noexcept;
		Device& operator=(Device&& dev) noexcept;
		~Device();

		static Expected<Device> Create(const Window::MainWindow& window, U32 descriptorCount) noexcept;

		constexpr bool IsCoherentMemorySupported() const noexcept { return false; }
		constexpr bool IsDedicatedAllocSupported() const noexcept { return true; }
		constexpr bool IsBufferMarkersSupported() const noexcept { return false; }
		constexpr bool IsExtendedSynchronizationSupported() const noexcept { return false; }
		constexpr bool IsUavNonUniformIndexing() const noexcept { return true; }

		void* GetHandle() const noexcept { return GetDevice(); }
		const GFX::DisplayProperties* GetDisplayProperties() const noexcept { return &displayProps; }

		U64 GetMainFence() const noexcept { return mainFenceVal; }
		U64 GetComputeFence() const noexcept { return computeFenceVal; }
		U64 GetCopyFence() const noexcept { return copyFenceVal; }

		Status WaitMain(U64 val) const noexcept { return WaitCPU(mainFence.Get(), val); }
		Status WaitCompute(U64 val) const noexcept { return WaitCPU(computeFence.Get(), val); }
		Status WaitCopy(U64 val) const noexcept { return WaitCPU(copyFence.Get(), val); }

		Expected<U64> SetMainFenceCPU() noexcept { return SetFenceCPU(mainFence.Get(), mainFenceVal); }
		Expected<U64> SetComputeFenceCPU() noexcept { return SetFenceCPU(computeFence.Get(), computeFenceVal); }
		Expected<U64> SetCopyFenceCPU() noexcept { return SetFenceCPU(copyFence.Get(), copyFenceVal); }

		Status WaitMainFromCompute(U64 val) const noexcept { return WaitGPU(computeFence.Get(), mainQueue.Get(), val); }
		Status WaitMainFromCopy(U64 val) const noexcept { return WaitGPU(copyFence.Get(), mainQueue.Get(), val); }
		Status WaitComputeFromMain(U64 val) const noexcept { return WaitGPU(mainFence.Get(), computeQueue.Get(), val); }
		Status WaitComputeFromCopy(U64 val) const noexcept { return WaitGPU(copyFence.Get(), computeQueue.Get(), val); }
		Status WaitCopyFromMain(U64 val) const noexcept { return WaitGPU(mainFence.Get(), copyQueue.Get(), val); }
		Status WaitCopyFromCompute(U64 val) const noexcept { return WaitGPU(computeFence.Get(), copyQueue.Get(), val); }

		Expected<U64> SetMainFence() noexcept { return SetFenceGPU(mainFence.Get(), mainQueue.Get(), mainFenceVal); }
		Expected<U64> SetComputeFence() noexcept { return SetFenceGPU(computeFence.Get(), computeQueue.Get(), computeFenceVal); }
		Expected<U64> SetCopyFence() noexcept { return SetFenceGPU(copyFence.Get(), copyQueue.Get(), copyFenceVal); }

#if _ZE_GFX_MARKERS
		void TagBeginMain(std::string_view tag, Pixel color) const noexcept { PIXBeginEvent(mainQueue.Get(), PIX_COLOR(color.Red, color.Blue, color.Green), tag.data()); }
		void TagBeginCompute(std::string_view tag, Pixel color) const noexcept { PIXBeginEvent(computeQueue.Get(), PIX_COLOR(color.Red, color.Blue, color.Green), tag.data()); }
		void TagBeginCopy(std::string_view tag, Pixel color) const noexcept { PIXBeginEvent(copyQueue.Get(), PIX_COLOR(color.Red, color.Blue, color.Green), tag.data()); }

		void TagEndMain() const noexcept { PIXEndEvent(mainQueue.Get()); }
		void TagEndCompute() const noexcept { PIXEndEvent(computeQueue.Get()); }
		void TagEndCopy() const noexcept { PIXEndEvent(copyQueue.Get()); }
#endif

		void OnMonitorChanged(const Window::MainWindow& window) noexcept;

		GFX::ShaderModel GetMaxShaderModel() const noexcept;
		std::pair<U32, U32> GetWaveLaneCountRange() const noexcept;
		bool IsShaderFloat16Supported() const noexcept;

		void Execute(GFX::CommandList* cls, U32 count) const noexcept;
		void ExecuteMain(GFX::CommandList& cl) const noexcept;
		void ExecuteCompute(GFX::CommandList& cl) const noexcept;
		void ExecuteCopy(GFX::CommandList& cl) const noexcept;

		Expected<FfxBreadcrumbsBlockData> AllocBreadcrumbsBlock(U64 bytes) noexcept;
		void FreeBreadcrumbsBlock(FfxBreadcrumbsBlockData& block) noexcept;
		void EndFrame() noexcept;

		// Gfx API Internal

#if _ZE_DEBUG_GFX_API
		constexpr DX::DebugInfoManager& GetInfoManager() noexcept { return debugManager; }
#endif
		constexpr AllocatorGPU::AllocTier GetCurrentAllocTier() const noexcept { return allocator.GetCurrentTier(); }
		constexpr bool IsGpuUploadHeap() const noexcept { return allocator.IsGpuUploadHeap(); }
		constexpr bool IsTightAlignment() const noexcept { return allocator.IsTightAlignmentEnabled(); }
		// Get size of CBV/SRV/UAV descriptor
		constexpr U32 GetDescriptorSize() const noexcept { return descriptorSize; }
		constexpr const DX::ComPtr<IDevice>& GetDev() const noexcept { return device; }

		IDevice* GetDevice() const noexcept { return device.Get(); }
		ICommandQueue* GetQueueMain() const noexcept { return mainQueue.Get(); }
		ICommandQueue* GetQueueCompute() const noexcept { return computeQueue.Get(); }
		ICommandQueue* GetQueueCopy() const noexcept { return copyQueue.Get(); }
		IDescriptorHeap* GetDescHeap() const noexcept { return descriptorGpuAllocator.GetMemory(nullptr).Heap.Get(); }
		AGSContext* GetAGSContext() const noexcept { ZE_ASSERT(Settings::GpuVendor == GFX::VendorGPU::AMD, "Wrong active GPU!"); return gpuCtx.AMD; }

		void FreeBuffer(ResourceInfo& info) noexcept { allocator.RemoveBuffer(info); }
		void FreeDynamicBuffer(ResourceInfo& info) noexcept { allocator.RemoveDynamicBuffer(info); }
		void FreeTexture(ResourceInfo& info) noexcept { allocator.RemoveTexture(info); }

		U32 GetAllocatedDescsCount(const DescriptorInfo& info) const noexcept { return Utils::SafeCast<U32>((info.GpuSide ? descriptorGpuAllocator : descriptorCpuAllocator).GetSize(info.Handle)); }

		D3D12_RESOURCE_DESC1 GetBufferDesc(U64 size) const noexcept;
		D3D12_RESOURCE_DESC1 GetTextureDesc(U32 width, U32 height, U16 count,
			DXGI_FORMAT format, GFX::Resource::Texture::Type type) const noexcept;

		Expected<ResourceInfo> CreateBuffer(const D3D12_RESOURCE_DESC1& desc, bool dynamic) noexcept;
		Expected<ResourceInfo> CreateTexture(const D3D12_RESOURCE_DESC1& desc) noexcept;

		Expected<DescriptorInfo> AllocDescs(U32 count, bool gpuHeap = true) noexcept;
		void FreeDescs(DescriptorInfo& descInfo) noexcept;
	};
}