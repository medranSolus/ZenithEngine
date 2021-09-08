#pragma once
#include "Allocator.h"

namespace ZE::GFX::API::DX12
{
	// Allocator for managing buffers and textures on GPU according to Tier 1 heaps
	class AllocatorTier1 : public Allocator
	{
		static constexpr D3D12_HEAP_FLAGS BUF_FLAG = D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
		static constexpr D3D12_HEAP_FLAGS TEX_FLAG = D3D12_HEAP_FLAG_DENY_BUFFERS;

		BufferInfo buffers;
		BufferInfo textures;

	public:
		AllocatorTier1(Device& dev, U32 freeListInitSize);
		AllocatorTier1(AllocatorTier1&&) = delete;
		AllocatorTier1(const AllocatorTier1&) = delete;
		AllocatorTier1& operator=(AllocatorTier1&&) = delete;
		AllocatorTier1& operator=(const AllocatorTier1&) = delete;
		virtual ~AllocatorTier1() = default;

		ResourceInfo AllocBuffer(Device& dev, U32 bytes, D3D12_RESOURCE_DESC& desc) { return AllocAlignMinimalChunks<NORMAL_CHUNK>(dev, buffers, bytes, desc, BUF_FLAG); }
		// Only small textures (smaller than 64KB)
		ResourceInfo AllocTexture_4KB(Device& dev, U32 bytes, D3D12_RESOURCE_DESC& desc) { return AllocAlignMinimalChunks<SMALL_CHUNK>(dev, textures, bytes, desc, TEX_FLAG); }
		// only normal textures and small multisampled textures (smaller than 4MB)
		ResourceInfo AllocTexture_64KB(Device& dev, U32 bytes, D3D12_RESOURCE_DESC& desc) { return AllocAlignBigChunks<NORMAL_CHUNK>(dev, textures, bytes, desc, TEX_FLAG); }
		// Only multisampled textures
		ResourceInfo AllocTexture_4MB(Device& dev, U32 bytes, D3D12_RESOURCE_DESC& desc) { return AllocAlignBigChunks<HUGE_CHUNK>(dev, textures, bytes, desc, TEX_FLAG); }

		void RemoveBuffer(U32 id, U32 size) { Remove<NORMAL_CHUNK>(buffers, id, size); }
		void RemoveTexture(U32 id, U32 size) { Remove<SMALL_CHUNK>(textures, id, size); }
	};
}