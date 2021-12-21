#pragma once
#include "Allocator.h"

namespace ZE::GFX::API::DX12
{
	// Allocator for managing buffers and textures on GPU according to Tier 1 heaps
	class AllocatorTier1 : public Allocator
	{
		static constexpr HeapFlags BUF_FLAG = HeapFlags::AllowBuffers;
		static constexpr HeapFlags DBUF_FLAG = static_cast<HeapFlags>(HeapFlags::AllowBuffers | HeapFlags::Dynamic);
		static constexpr HeapFlags TEX_FLAG = HeapFlags::AllowTextures;
		static constexpr U64 BUF_HEAP_SIZE = 256 << 20; // 256 MB
		static constexpr U64 DBUF_HEAP_SIZE = 64 << 20; // 64 MB
		static constexpr U64 TEX_HEAP_SIZE = 512 << 20; // 512 MB

		BufferInfo buffers;
		BufferInfo dynamicBuffers;
		BufferInfo textures;

	public:
		AllocatorTier1(Device& dev);
		ZE_CLASS_DELETE(AllocatorTier1);
		virtual ~AllocatorTier1() = default;

		ResourceInfo AllocBuffer(Device& dev, U32 bytes, const D3D12_RESOURCE_DESC& desc) { return AllocAlignMinimalChunks<NORMAL_CHUNK, BUF_HEAP_SIZE>(dev, buffers, bytes, desc, BUF_FLAG); }
		// Buffers that can be written fast into from CPU
		ResourceInfo AllocDynamicBuffer(Device& dev, U32 bytes, const D3D12_RESOURCE_DESC& desc) { return AllocAlignMinimalChunks<NORMAL_CHUNK, DBUF_HEAP_SIZE>(dev, dynamicBuffers, bytes, desc, DBUF_FLAG); }
		// Only small textures (smaller than 64KB)
		ResourceInfo AllocTexture_4KB(Device& dev, U32 bytes, const D3D12_RESOURCE_DESC& desc) { return AllocAlignMinimalChunks<SMALL_CHUNK, TEX_HEAP_SIZE>(dev, textures, bytes, desc, TEX_FLAG); }
		// only normal textures and small multisampled textures (smaller than 4MB)
		ResourceInfo AllocTexture_64KB(Device& dev, U32 bytes, const D3D12_RESOURCE_DESC& desc) { return AllocAlignBigChunks<NORMAL_CHUNK, TEX_HEAP_SIZE>(dev, textures, bytes, desc, TEX_FLAG); }
		// Only multisampled textures
		ResourceInfo AllocTexture_4MB(Device& dev, U32 bytes, const D3D12_RESOURCE_DESC& desc) { return AllocAlignBigChunks<HUGE_CHUNK, TEX_HEAP_SIZE>(dev, textures, bytes, desc, TEX_FLAG); }

		void RemoveBuffer(U32 id, U32 size) { Remove<NORMAL_CHUNK, BUF_HEAP_SIZE>(buffers, id, size); }
		void RemoveDynamicBuffer(U32 id, U32 size) { Remove<NORMAL_CHUNK, DBUF_HEAP_SIZE>(dynamicBuffers, id, size); }
		void RemoveTexture(U32 id, U32 size) { Remove<SMALL_CHUNK, TEX_HEAP_SIZE>(textures, id, size); }
	};
}