#pragma once
#include "Allocator/ChunkedTLSF.h"
#include "D3D12.h"
#include "ResourceInfo.h"

namespace ZE::GFX::API::DX12
{
	class Device;

	// Allocator for managing buffers and textures on GPU based on TLSF algorithm for Tier 1&2 heaps
	class AllocatorGPU final
	{
	public:
		enum class AllocTier : bool { Tier1, Tier2 };

	private:
		typedef Allocator::TLSFMemoryChunkFlags HeapFlags;
		enum HeapFlag : HeapFlags { Dynamic = 1, AllowBuffers = 2, AllowTextures = 4, NoMSAA = 8, CommittedAlloc = 16 };

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
		static constexpr U64 MAIN_HEAP_SIZE = 256 * Math::MEGABYTE;
		static constexpr U64 SECONDARY_HEAP_SIZE = 512 * Math::MEGABYTE;
		static constexpr U64 DYNAMIC_BUFF_HEAP_SIZE = 64 * Math::MEGABYTE;

		AllocTier allocTier = AllocTier::Tier1;
		HeapAllocator::BlockAllocator blockAllocator;
		HeapAllocator::ChunkAllocator chunkAllocator;

		// Tier1: buffers | Tier2: buffers + textures
		HeapAllocator mainAllocator;
		// Tier1: textures | Tier2: unused
		HeapAllocator secondaryAllocator;
		// Tier1 + Tier2: dynamic buffers
		HeapAllocator dynamicBuffersAllocator;

		static constexpr U64 GetHeapAlignment(HeapFlags flags) noexcept;
		static constexpr D3D12_HEAP_TYPE GetHeapType(HeapFlags flags) noexcept;
		static constexpr D3D12_HEAP_FLAGS GetHeapFlags(HeapFlags flags) noexcept;
		static constexpr D3D12_RESOURCE_STATES GetResourceState(HeapFlags flags) noexcept;

		static DX::ComPtr<IResource> CreateCommittedResource(Device& dev,
			const D3D12_RESOURCE_DESC1& desc, HeapFlags flags);
		static DX::ComPtr<IResource> CreateResource(Device& dev,
			const D3D12_RESOURCE_DESC1& desc, U64 offset, IHeap* heap, HeapFlags flags);

		static ResourceInfo Alloc(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc, U64 alignment, HeapAllocator& allocator);
		// Find and allocate smallest memory region in heap at given boundary (assuming boundary bigger than smallest chunk)
		static ResourceInfo AllocBigChunks(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc, U64 alignment, HeapAllocator& allocator);
		// Find and allocate smallest memory region in heap at minimal boundary (given chunk size is smallest possible)
		static ResourceInfo AllocMinimalChunks(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc, HeapAllocator& allocator);

		// Remove allocated memory and return it to free pool merging whenever possible with nerby free regions
		static void Remove(ResourceInfo& resInfo, HeapAllocator& allocator);

	public:
		AllocatorGPU() : blockAllocator(BLOCK_ALLOC_CAPACITY), chunkAllocator(CHUNK_ALLOC_CAPACITY), mainAllocator(blockAllocator, chunkAllocator),
			secondaryAllocator(blockAllocator, chunkAllocator), dynamicBuffersAllocator(blockAllocator, chunkAllocator) {}
		ZE_CLASS_MOVE(AllocatorGPU);
		~AllocatorGPU() = default;

		constexpr AllocTier GetCurrentTier() const noexcept { return allocTier; }

		void RemoveBuffer(ResourceInfo& resInfo) { Remove(resInfo, mainAllocator); }
		void RemoveDynamicBuffer(ResourceInfo& resInfo) { Remove(resInfo, dynamicBuffersAllocator); }
		void RemoveTexture(ResourceInfo& resInfo) { Remove(resInfo, allocTier == AllocTier::Tier2 ? mainAllocator : secondaryAllocator); }

		void Init(Device& dev, D3D12_RESOURCE_HEAP_TIER heapTier);

		ResourceInfo AllocBuffer(Device& dev, const D3D12_RESOURCE_DESC1& desc);
		// Buffers that can be written fast into from CPU
		ResourceInfo AllocDynamicBuffer(Device& dev, const D3D12_RESOURCE_DESC1& desc);
		// Only small textures (smaller than 64KB)
		ResourceInfo AllocTexture_4KB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc);
		// Only normal textures and small multisampled textures (smaller than 4MB)
		ResourceInfo AllocTexture_64KB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc);
		// Only multisampled textures
		ResourceInfo AllocTexture_4MB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc);
	};
}