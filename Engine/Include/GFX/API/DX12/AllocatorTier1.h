#pragma once
#include "Allocator.h"

namespace ZE::GFX::API::DX12
{
	// Allocator for managing buffers and textures on GPU according to Tier 1 heaps
	class AllocatorTier1 : public Allocator
	{
		static constexpr HeapFlags BUF_FLAG = HeapFlag::AllowBuffers;
		static constexpr HeapFlags DBUF_FLAG = static_cast<HeapFlags>(HeapFlag::AllowBuffers | HeapFlag::Dynamic);
		static constexpr HeapFlags TEX_FLAG = HeapFlag::AllowTextures;
		static constexpr U64 BUF_HEAP_SIZE = 256 << 20; // 256 MB
		static constexpr U64 DBUF_HEAP_SIZE = 64 << 20; // 64 MB
		static constexpr U64 TEX_HEAP_SIZE = 512 << 20; // 512 MB

		BufferInfo buffers;
		BufferInfo dynamicBuffers;
		BufferInfo textures;

	public:
		AllocatorTier1() = default;
		AllocatorTier1(Device& dev)
			: buffers(dev, heaps, BUF_FLAG, BUF_HEAP_SIZE, NORMAL_CHUNK),
			dynamicBuffers(dev, heaps, DBUF_FLAG, DBUF_HEAP_SIZE, NORMAL_CHUNK),
			textures(dev, heaps, TEX_FLAG, TEX_HEAP_SIZE, SMALL_CHUNK) {}
		ZE_CLASS_DELETE(AllocatorTier1);
		virtual ~AllocatorTier1() = default;

		ResourceInfo AllocBuffer(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC& desc) { return AllocMinimalChunks(dev, buffers, bytes, desc, BUF_FLAG, BUF_HEAP_SIZE, NORMAL_CHUNK); }
		// Buffers that can be written fast into from CPU
		ResourceInfo AllocDynamicBuffer(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC& desc) { return AllocMinimalChunks(dev, dynamicBuffers, bytes, desc, DBUF_FLAG, DBUF_HEAP_SIZE, NORMAL_CHUNK); }
		// Only small textures (smaller than 64KB)
		ResourceInfo AllocTexture_4KB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC& desc) { return AllocMinimalChunks(dev, textures, bytes, desc, TEX_FLAG, TEX_HEAP_SIZE, SMALL_CHUNK); }
		// Only normal textures and small multisampled textures (smaller than 4MB)
		ResourceInfo AllocTexture_64KB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC& desc) { return AllocBigChunks(dev, textures, bytes, desc, TEX_FLAG, TEX_HEAP_SIZE, NORMAL_CHUNK, SMALL_CHUNK); }
		// Only multisampled textures
		ResourceInfo AllocTexture_4MB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC& desc) { return AllocBigChunks(dev, textures, bytes, desc, TEX_FLAG, TEX_HEAP_SIZE, HUGE_CHUNK, SMALL_CHUNK); }

		void RemoveBuffer(ResourceInfo& resInfo) { Remove(buffers, resInfo, BUF_HEAP_SIZE, NORMAL_CHUNK); }
		void RemoveDynamicBuffer(ResourceInfo& resInfo) { Remove(dynamicBuffers, resInfo, DBUF_HEAP_SIZE, NORMAL_CHUNK); }
		void RemoveTexture(ResourceInfo& resInfo) { Remove(textures, resInfo, TEX_HEAP_SIZE, SMALL_CHUNK); }
	};
}