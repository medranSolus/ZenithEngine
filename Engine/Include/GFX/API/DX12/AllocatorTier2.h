#pragma once
#include "Allocator.h"

namespace ZE::GFX::API::DX12
{
	// Allocator for managing buffers and textures on GPU according to Tier 2 heaps
	class AllocatorTier2 : public Allocator
	{
		static constexpr D3D12_HEAP_FLAGS HEAP_FLAG = D3D12_HEAP_FLAG_NONE;

		BufferInfo memory;

	public:
		AllocatorTier2(Device& dev, U32 freeListInitSize) : memory(freeListInitSize) { CreateHeap(dev, &*memory.Heaps, HEAP_FLAG); }
		AllocatorTier2(AllocatorTier2&&) = delete;
		AllocatorTier2(const AllocatorTier2&) = delete;
		AllocatorTier2& operator=(AllocatorTier2&&) = delete;
		AllocatorTier2& operator=(const AllocatorTier2&) = delete;
		virtual ~AllocatorTier2() = default;

		// Only small textures (smaller than 64KB)
		ResourceInfo Alloc_4KB(Device& dev, U32 bytes, D3D12_RESOURCE_DESC& desc) { return AllocAlignMinimalChunks<SMALL_CHUNK>(dev, memory, bytes, desc, HEAP_FLAG); }
		// Buffers, normal textures and small multisampled textures (smaller than 4MB)
		ResourceInfo Alloc_64KB(Device& dev, U32 bytes, D3D12_RESOURCE_DESC& desc) { return AllocAlignBigChunks<NORMAL_CHUNK>(dev, memory, bytes, desc, HEAP_FLAG); }
		// Only multisampled textures
		ResourceInfo Alloc_4MB(Device& dev, U32 bytes, D3D12_RESOURCE_DESC& desc) { return AllocAlignBigChunks<HUGE_CHUNK>(dev, memory, bytes, desc, HEAP_FLAG); }
		void Remove(U32 id, U32 size) { Allocator::Remove<SMALL_CHUNK>(memory, id, size); }
	};
}