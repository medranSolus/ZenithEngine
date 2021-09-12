#pragma once
#include "Allocator.h"

namespace ZE::GFX::API::DX12
{
	// Allocator for managing buffers and textures on GPU according to Tier 2 heaps
	class AllocatorTier2 : public Allocator
	{
		static constexpr HeapFlags HEAP_FLAG = static_cast<HeapFlags>(HeapFlags::AllowBuffers | HeapFlags::AllowTextures);
		static constexpr HeapFlags BUF_HEAP_FLAG = static_cast<HeapFlags>(HeapFlags::AllowBuffers | HeapFlags::Dynamic);
		static constexpr U64 HEAP_SIZE = 768 << 20; // 768 MB
		static constexpr U64 BUF_HEAP_SIZE = 64 << 20; // 64 MB

		BufferInfo memory;
		BufferInfo buffers;

	public:
		AllocatorTier2(Device& dev);
		AllocatorTier2(AllocatorTier2&&) = delete;
		AllocatorTier2(const AllocatorTier2&) = delete;
		AllocatorTier2& operator=(AllocatorTier2&&) = delete;
		AllocatorTier2& operator=(const AllocatorTier2&) = delete;
		virtual ~AllocatorTier2() = default;

		// Only small textures (smaller than 64KB)
		ResourceInfo Alloc_4KB(Device& dev, U32 bytes, D3D12_RESOURCE_DESC& desc) { return AllocAlignMinimalChunks<SMALL_CHUNK, HEAP_SIZE>(dev, memory, bytes, desc, HEAP_FLAG); }
		// Buffers, normal textures and small multisampled textures (smaller than 4MB)
		ResourceInfo Alloc_64KB(Device& dev, U32 bytes, D3D12_RESOURCE_DESC& desc) { return AllocAlignBigChunks<NORMAL_CHUNK, HEAP_SIZE>(dev, memory, bytes, desc, HEAP_FLAG); }
		// Only multisampled textures
		ResourceInfo Alloc_4MB(Device& dev, U32 bytes, D3D12_RESOURCE_DESC& desc) { return AllocAlignBigChunks<HUGE_CHUNK, HEAP_SIZE>(dev, memory, bytes, desc, HEAP_FLAG); }
		// Buffers that can be written fast into from CPU
		ResourceInfo AllocDynamicBuffer(Device& dev, U32 bytes, D3D12_RESOURCE_DESC& desc) { return AllocAlignMinimalChunks<NORMAL_CHUNK, BUF_HEAP_SIZE>(dev, buffers, bytes, desc, BUF_HEAP_FLAG); }

		void Remove(U32 id, U32 size) { Allocator::Remove<SMALL_CHUNK, HEAP_SIZE>(memory, id, size); }
		void RemoveDynamicBuffer(U32 id, U32 size) { Allocator::Remove<NORMAL_CHUNK, BUF_HEAP_SIZE>(buffers, id, size); }
	};
}