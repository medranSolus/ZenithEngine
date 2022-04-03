#pragma once
#include "Allocator.h"

namespace ZE::GFX::API::DX12
{
	// Allocator for managing buffers and textures on GPU according to Tier 2 heaps
	class AllocatorTier2 : public Allocator
	{
		static constexpr HeapFlags HEAP_FLAG = static_cast<HeapFlags>(HeapFlag::AllowBuffers | HeapFlag::AllowTextures);
		static constexpr HeapFlags BUF_HEAP_FLAG = static_cast<HeapFlags>(HeapFlag::AllowBuffers | HeapFlag::Dynamic);
		static constexpr U64 HEAP_SIZE = 768 << 20; // 768 MB
		static constexpr U64 BUF_HEAP_SIZE = 64 << 20; // 64 MB

		BufferInfo memory;
		BufferInfo buffers;

	public:
		AllocatorTier2() = default;
		AllocatorTier2(Device& dev)
			: memory(dev, heaps, HEAP_FLAG, HEAP_SIZE, SMALL_CHUNK),
			buffers(dev, heaps, BUF_HEAP_FLAG, BUF_HEAP_SIZE, NORMAL_CHUNK) {}
		ZE_CLASS_DELETE(AllocatorTier2);
		virtual ~AllocatorTier2() = default;

		// Only small textures (smaller than 64KB)
		ResourceInfo Alloc_4KB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC& desc) { return AllocMinimalChunks(dev, memory, bytes, desc, HEAP_FLAG, HEAP_SIZE, SMALL_CHUNK); }
		// Buffers, normal textures and small multisampled textures (smaller than 4MB)
		ResourceInfo Alloc_64KB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC& desc) { return AllocBigChunks(dev, memory, bytes, desc, HEAP_FLAG, HEAP_SIZE, NORMAL_CHUNK, SMALL_CHUNK); }
		// Only multisampled textures
		ResourceInfo Alloc_4MB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC& desc) { return AllocBigChunks(dev, memory, bytes, desc, HEAP_FLAG, HEAP_SIZE, HUGE_CHUNK, SMALL_CHUNK); }
		// Buffers that can be written fast into from CPU
		ResourceInfo AllocDynamicBuffer(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC& desc) { return AllocMinimalChunks(dev, buffers, bytes, desc, BUF_HEAP_FLAG, BUF_HEAP_SIZE, NORMAL_CHUNK); }

		void Remove(ResourceInfo& resInfo) { Allocator::Remove(memory, resInfo, HEAP_SIZE, SMALL_CHUNK); }
		void RemoveDynamicBuffer(ResourceInfo& resInfo) { Allocator::Remove(buffers, resInfo, BUF_HEAP_SIZE, NORMAL_CHUNK); }
	};
}