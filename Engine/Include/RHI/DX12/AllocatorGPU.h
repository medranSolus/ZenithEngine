#pragma once
#include "Allocator/ChunkedTLSF.h"
#include "ResourceInfo.h"

namespace ZE::RHI::DX12
{
	class Device;

	// Allocator for managing buffers and textures on GPU based on TLSF algorithm for Tier 1&2 heaps
	class AllocatorGPU final
	{
	public:
		enum class AllocTier : bool { Tier1, Tier2 };

	private:
		typedef Allocator::TLSFMemoryChunkFlags HeapFlags;
		enum HeapFlag : HeapFlags { None = 0, Dynamic = 1, AllowBuffers = 2, AllowTextures = 4, AllowTexturesRTDS = 8, NoMSAA = 16, CommittedAlloc = 32, GpuUploadHeap = 64, Readback = 128 };

		struct Memory
		{
			DX::ComPtr<IHeap> Heap = nullptr;

			static Status Init(Memory& chunk, HeapFlags flags, U64 size, void* userData) noexcept;
			static void Destroy(Memory& chunk, void* userData) noexcept { chunk.Heap = nullptr; }
		};
		typedef Allocator::ChunkedTLSF<Memory, 4, 2> HeapAllocator;

		static constexpr U32 TIGHT_CHUNK = D3D12_TIGHT_ALIGNMENT_MIN_PLACED_RESOURCE_ALIGNMENT; // 8 B
		static constexpr U32 SMALL_CHUNK = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT; // 4 KB
		static constexpr U32 NORMAL_CHUNK = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; // 64 KB
		static constexpr U32 HUGE_CHUNK = D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT; // 4 MB
		static constexpr U64 BLOCK_ALLOC_CAPACITY = 200;
		static constexpr U64 CHUNK_ALLOC_CAPACITY = 30;

		static constexpr HeapFlags MAIN_HEAP_FLAGS = HeapFlag::AllowBuffers | HeapFlag::NoMSAA;
		static constexpr HeapFlags SECONDARY_HEAP_FLAGS = HeapFlag::AllowTextures | HeapFlag::NoMSAA;
		static constexpr HeapFlags DYNAMIC_BUFF_HEAP_FLAGS = HeapFlag::AllowBuffers | HeapFlag::Dynamic | HeapFlag::NoMSAA;
		static constexpr HeapFlags READBACK_BUFF_HEAP_FLAGS = HeapFlag::AllowBuffers | HeapFlag::Readback | HeapFlag::NoMSAA;

		AllocTier allocTier = AllocTier::Tier1;
		bool tightAlignment = false;
		std::shared_ptr<HeapAllocator::BlockAllocator> blockAllocator;
		std::shared_ptr<HeapAllocator::ChunkAllocator> chunkAllocator;

		// Tier1: buffers | Tier2: buffers + textures
		HeapAllocator mainAllocator;
		// Tier1: textures | Tier2: unused
		HeapAllocator secondaryAllocator;
		// Tier1 + Tier2: dynamic buffers
		HeapAllocator dynamicBuffersAllocator;
		// Tier1 + Tier2: readback buffers
		HeapAllocator readbackBuffersAllocator;

		static constexpr U64 GetHeapAlignment(HeapFlags flags) noexcept;
		static constexpr D3D12_HEAP_TYPE GetHeapType(HeapFlags flags) noexcept;
		static constexpr D3D12_HEAP_FLAGS GetHeapFlags(HeapFlags flags) noexcept;

		static Expected<DX::ComPtr<IResource>> CreateCommittedResource(Device& dev,
			const D3D12_RESOURCE_DESC1& desc, D3D12_BARRIER_LAYOUT layout, HeapFlags flags) noexcept;
		static Expected<DX::ComPtr<IResource>> CreateResource(Device& dev, const D3D12_RESOURCE_DESC1& desc,
			D3D12_BARRIER_LAYOUT layout, U64 offset, IHeap* heap, HeapFlags flags) noexcept;

		static Expected<ResourceInfo> Alloc(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc,
			D3D12_BARRIER_LAYOUT layout, U64 alignment, HeapAllocator& allocator) noexcept;
		// Find and allocate smallest memory region in heap at given boundary (assuming boundary bigger than smallest chunk)
		static Expected<ResourceInfo> AllocBigChunks(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc,
			D3D12_BARRIER_LAYOUT layout, U64 alignment, HeapAllocator& allocator) noexcept;
		// Find and allocate smallest memory region in heap at minimal boundary (given chunk size is smallest possible)
		static Expected<ResourceInfo> AllocMinimalChunks(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc,
			D3D12_BARRIER_LAYOUT layout, HeapAllocator& allocator) noexcept;

		// Remove allocated memory and return it to free pool merging whenever possible with nerby free regions
		static void Remove(ResourceInfo& resInfo, HeapAllocator& allocator) noexcept;

	public:
		AllocatorGPU() noexcept;
		ZE_CLASS_MOVE(AllocatorGPU);
		~AllocatorGPU();

		static Expected<AllocatorGPU> Create(Device& dev, D3D12_RESOURCE_HEAP_TIER heapTier, bool gpuUploadHeapSupported, D3D12_TIGHT_ALIGNMENT_TIER alignmentTier) noexcept;

		constexpr AllocTier GetCurrentTier() const noexcept { return allocTier; }
		constexpr bool IsGpuUploadHeap() const noexcept { return mainAllocator.GetChunkCreationFlags() & HeapFlag::GpuUploadHeap; }
		constexpr bool IsTightAlignmentEnabled() const noexcept { return tightAlignment; }

		void RemoveBuffer(ResourceInfo& resInfo) noexcept { Remove(resInfo, mainAllocator); }
		void RemoveDynamicBuffer(ResourceInfo& resInfo) noexcept { Remove(resInfo, dynamicBuffersAllocator); }
		void RemoveReadbackBuffer(ResourceInfo& resInfo) noexcept { Remove(resInfo, readbackBuffersAllocator); }
		void RemoveTexture(ResourceInfo& resInfo) noexcept { Remove(resInfo, allocTier == AllocTier::Tier2 ? mainAllocator : secondaryAllocator); }

		Expected<ResourceInfo> AllocBuffer(Device& dev, const D3D12_RESOURCE_DESC1& desc) noexcept;
		// Buffers that can be written fast into from CPU
		Expected<ResourceInfo> AllocDynamicBuffer(Device& dev, const D3D12_RESOURCE_DESC1& desc) noexcept;
		// Buffers that can be written by GPU and read fast from CPU
		Expected<ResourceInfo> AllocReadbackBuffer(Device& dev, const D3D12_RESOURCE_DESC1& desc) noexcept;
		Expected<ResourceInfo> AllocTexture(Device& dev, const D3D12_RESOURCE_DESC1& desc) noexcept;
	};
}