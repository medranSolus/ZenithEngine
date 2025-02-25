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

			static void Init(Memory& chunk, HeapFlags flags, U64 size, void* userData);
			static void Destroy(Memory& chunk, void* userData) noexcept { chunk.Heap = nullptr; }
		};
		typedef Allocator::ChunkedTLSF<Memory, 4, 2> HeapAllocator;

		static constexpr U64 SMALL_CHUNK = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT; // 4 KB
		static constexpr U64 NORMAL_CHUNK = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; // 64 KB
		static constexpr U64 HUGE_CHUNK = D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT; // 4 MB
		static constexpr U64 BLOCK_ALLOC_CAPACITY = 200;
		static constexpr U64 CHUNK_ALLOC_CAPACITY = 30;

		static constexpr HeapFlags MAIN_HEAP_FLAGS = HeapFlag::AllowBuffers | HeapFlag::NoMSAA;
		static constexpr HeapFlags SECONDARY_HEAP_FLAGS = HeapFlag::AllowTextures | HeapFlag::NoMSAA;
		static constexpr HeapFlags DYNAMIC_BUFF_HEAP_FLAGS = HeapFlag::AllowBuffers | HeapFlag::Dynamic | HeapFlag::NoMSAA;
		static constexpr HeapFlags READBACK_BUFF_HEAP_FLAGS = HeapFlag::AllowBuffers | HeapFlag::Readback | HeapFlag::NoMSAA;

		AllocTier allocTier = AllocTier::Tier1;
		HeapAllocator::BlockAllocator blockAllocator;
		HeapAllocator::ChunkAllocator chunkAllocator;

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

		static DX::ComPtr<IResource> CreateCommittedResource(Device& dev,
			const D3D12_RESOURCE_DESC1& desc, D3D12_BARRIER_LAYOUT layout, HeapFlags flags);
		static DX::ComPtr<IResource> CreateResource(Device& dev, const D3D12_RESOURCE_DESC1& desc,
			D3D12_BARRIER_LAYOUT layout, U64 offset, IHeap* heap, HeapFlags flags);

		static ResourceInfo Alloc(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc,
			D3D12_BARRIER_LAYOUT layout, U64 alignment, HeapAllocator& allocator);
		// Find and allocate smallest memory region in heap at given boundary (assuming boundary bigger than smallest chunk)
		static ResourceInfo AllocBigChunks(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc,
			D3D12_BARRIER_LAYOUT layout, U64 alignment, HeapAllocator& allocator);
		// Find and allocate smallest memory region in heap at minimal boundary (given chunk size is smallest possible)
		static ResourceInfo AllocMinimalChunks(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc,
			D3D12_BARRIER_LAYOUT layout, HeapAllocator& allocator);

		// Remove allocated memory and return it to free pool merging whenever possible with nerby free regions
		static void Remove(ResourceInfo& resInfo, HeapAllocator& allocator) noexcept;

	public:
		AllocatorGPU() : blockAllocator(BLOCK_ALLOC_CAPACITY), chunkAllocator(CHUNK_ALLOC_CAPACITY), mainAllocator(blockAllocator, chunkAllocator),
			secondaryAllocator(blockAllocator, chunkAllocator), dynamicBuffersAllocator(blockAllocator, chunkAllocator), readbackBuffersAllocator(blockAllocator, chunkAllocator) {}
		ZE_CLASS_MOVE(AllocatorGPU);
		~AllocatorGPU();

		constexpr AllocTier GetCurrentTier() const noexcept { return allocTier; }
		constexpr bool IsGpuUploadHeap() const noexcept { return mainAllocator.GetChunkCreationFlags() & HeapFlag::GpuUploadHeap; }

		void RemoveBuffer(ResourceInfo& resInfo) noexcept { Remove(resInfo, mainAllocator); }
		void RemoveDynamicBuffer(ResourceInfo& resInfo) noexcept { Remove(resInfo, dynamicBuffersAllocator); }
		void RemoveReadackBuffer(ResourceInfo& resInfo) noexcept { Remove(resInfo, readbackBuffersAllocator); }
		void RemoveTexture(ResourceInfo& resInfo) noexcept { Remove(resInfo, allocTier == AllocTier::Tier2 ? mainAllocator : secondaryAllocator); }

		void Init(Device& dev, D3D12_RESOURCE_HEAP_TIER heapTier, bool gpuUploadHeapSupported);

		ResourceInfo AllocBuffer(Device& dev, const D3D12_RESOURCE_DESC1& desc);
		// Buffers that can be written fast into from CPU
		ResourceInfo AllocDynamicBuffer(Device& dev, const D3D12_RESOURCE_DESC1& desc);
		// Buffers that can be written by GPU and read fast from CPU
		ResourceInfo AllocReadbackBuffer(Device& dev, const D3D12_RESOURCE_DESC1& desc);
		// Only small textures (smaller than 64KB)
		ResourceInfo AllocTexture_4KB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc);
		// Only normal textures and small multisampled textures (smaller than 4MB)
		ResourceInfo AllocTexture_64KB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc);
		// Only multisampled textures
		ResourceInfo AllocTexture_4MB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc);
	};
}