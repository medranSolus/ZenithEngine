#pragma once
#include "GFX/Resource/Texture/Type.h"
#include "GFX/ShaderModel.h"
#include "Window/MainWindow.h"
#include "AllocatorGPU.h"
#include "CommandList.h"
#include "DescriptorInfo.h"
ZE_WARNING_PUSH
#include "amd_ags.h"
#include "xess/xess_d3d12.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	class CommandList;
	namespace Resource
	{
		class Generic;
	}
}
namespace ZE::RHI::DX12
{
	class Device final
	{
		static constexpr U16 COPY_LIST_GROW_SIZE = 5;
		static constexpr D3D_FEATURE_LEVEL MINIMAL_D3D_LEVEL = D3D_FEATURE_LEVEL_12_1;
		static constexpr U64 BLOCK_DESCRIPTOR_ALLOC_CAPACITY = 1000;
		static constexpr U64 CHUNK_DESCRIPTOR_ALLOC_CAPACITY = 20;
		static constexpr U32 CPU_DESCRIPTOR_CHUNK_SIZE = 1000;

		struct DescHeap
		{
			DX::ComPtr<IDescriptorHeap> Heap = nullptr;

			static void Init(DescHeap& chunk, Allocator::TLSFMemoryChunkFlags flags, U64 size, void* userData);
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

		U32 commandListsCount = 0;
		Ptr<ICommandList*> commandLists = nullptr;

		UA64 mainFenceVal = 0;
		DX::ComPtr<IFence> mainFence;
		UA64 computeFenceVal = 0;
		DX::ComPtr<IFence> computeFence;
		UA64 copyFenceVal = 0;
		DX::ComPtr<IFence> copyFence;

		AllocatorGPU allocator;
		FfxInterface ffxInterface;
		xess_context_handle_t xessCtx = nullptr;

		DescriptorAllocator::BlockAllocator blockDescAllocator;
		DescriptorAllocator::ChunkAllocator chunkDescAllocator;
		DescriptorAllocator descriptorGpuAllocator;
		DescriptorAllocator descriptorCpuAllocator;
		U32 descriptorSize = 0;

		// Hardware specific data
		union
		{
			AGSContext* gpuCtxAMD;
		};
#if !_ZE_MODE_RELEASE
		HMODULE pixCapturer = nullptr;
#endif

		void WaitCPU(IFence* fence, U64 val);
		void WaitGPU(IFence* fence, ICommandQueue* queue, U64 val);
		U64 SetFenceCPU(IFence* fence, UA64& fenceVal);
		U64 SetFenceGPU(IFence* fence, ICommandQueue* queue, UA64& fenceVal);
		void Execute(ICommandQueue* queue, CommandList& cl);

	public:
		Device() noexcept
			: blockDescAllocator(BLOCK_DESCRIPTOR_ALLOC_CAPACITY), chunkDescAllocator(CHUNK_DESCRIPTOR_ALLOC_CAPACITY),
			descriptorGpuAllocator(blockDescAllocator, chunkDescAllocator, true),
			descriptorCpuAllocator(blockDescAllocator, chunkDescAllocator) {}
		Device(const Window::MainWindow& window, U32 descriptorCount);
		ZE_CLASS_DELETE(Device);
		~Device();

		constexpr U32 GetData() const noexcept { return Utils::SafeCast<U32>(descriptorGpuAllocator.GetChunkSize()); }
		constexpr FfxInterface* GetFfxInterface() noexcept { return &ffxInterface; }
		constexpr U32 GetCommandBufferSize() const noexcept { return commandListsCount; }
		constexpr void EndFrame() noexcept {}

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
		void TagBeginMain(std::string_view tag, Pixel color) const noexcept { PIXBeginEvent(mainQueue.Get(), PIX_COLOR(color.Red, color.Blue, color.Green), tag.data()); }
		void TagBeginCompute(std::string_view tag, Pixel color) const noexcept { PIXBeginEvent(computeQueue.Get(), PIX_COLOR(color.Red, color.Blue, color.Green), tag.data()); }
		void TagBeginCopy(std::string_view tag, Pixel color) const noexcept { PIXBeginEvent(copyQueue.Get(), PIX_COLOR(color.Red, color.Blue, color.Green), tag.data()); }

		void TagEndMain() const noexcept { PIXEndEvent(mainQueue.Get()); }
		void TagEndCompute() const noexcept { PIXEndEvent(computeQueue.Get()); }
		void TagEndCopy() const noexcept { PIXEndEvent(copyQueue.Get()); }
#endif

		xess_context_handle_t GetXeSSCtx();
		void InitializeXeSS(UInt2 targetRes, xess_quality_settings_t quality, U32 initFlags);
		void ExecuteXeSS(GFX::CommandList& cl, GFX::Resource::Generic& color, GFX::Resource::Generic& motionVectors,
			GFX::Resource::Generic* depth, GFX::Resource::Generic* exposure, GFX::Resource::Generic* responsive,
			GFX::Resource::Generic& output, float jitterX, float jitterY, UInt2 renderSize, bool reset);
		GFX::ShaderModel GetMaxShaderModel() const noexcept;
		std::pair<U32, U32> GetWaveLaneCountRange() const noexcept;
		bool IsShaderFloat16Supported() const noexcept;

		void Execute(GFX::CommandList* cls, U32 count);
		void ExecuteMain(GFX::CommandList& cl);
		void ExecuteCompute(GFX::CommandList& cl);
		void ExecuteCopy(GFX::CommandList& cl);

		// Gfx API Internal

#if _ZE_DEBUG_GFX_API
		constexpr DX::DebugInfoManager& GetInfoManager() noexcept { return debugManager; }
#endif
		constexpr AllocatorGPU::AllocTier GetCurrentAllocTier() const noexcept { return allocator.GetCurrentTier(); }
		constexpr bool IsGpuUploadHeap() const noexcept { return allocator.IsGpuUploadHeap(); }
		// Get size of CBV/SRV/UAV descriptor
		constexpr U32 GetDescriptorSize() const noexcept { return descriptorSize; }
		constexpr const DX::ComPtr<IDevice>& GetDev() const noexcept { return device; }

		IDevice* GetDevice() const noexcept { return device.Get(); }
		ICommandQueue* GetQueueMain() const noexcept { return mainQueue.Get(); }
		ICommandQueue* GetQueueCompute() const noexcept { return computeQueue.Get(); }
		ICommandQueue* GetQueueCopy() const noexcept { return copyQueue.Get(); }
		IDescriptorHeap* GetDescHeap() const noexcept { return descriptorGpuAllocator.GetMemory(nullptr).Heap.Get(); }
		AGSContext* GetAGSContext() const noexcept { ZE_ASSERT(Settings::GpuVendor == GFX::VendorGPU::AMD, "Wrong active GPU!"); return gpuCtxAMD; }

		void FreeBuffer(ResourceInfo& info) { allocator.RemoveBuffer(info); }
		void FreeDynamicBuffer(ResourceInfo& info) { allocator.RemoveDynamicBuffer(info); }
		void FreeTexture(ResourceInfo& info) { allocator.RemoveTexture(info); }

		D3D12_RESOURCE_DESC1 GetBufferDesc(U64 size) const noexcept;
		D3D12_RESOURCE_DESC1 GetTextureDesc(U32 width, U32 height, U16 count,
			DXGI_FORMAT format, GFX::Resource::Texture::Type type) const noexcept;

		ResourceInfo CreateBuffer(const D3D12_RESOURCE_DESC1& desc, bool dynamic);
		ResourceInfo CreateTexture(const D3D12_RESOURCE_DESC1& desc);

		DescriptorInfo AllocDescs(U32 count, bool gpuHeap = true) noexcept;
		void FreeDescs(DescriptorInfo& descInfo) noexcept;
	};
}