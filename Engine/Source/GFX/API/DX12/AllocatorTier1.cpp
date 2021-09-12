#include "GFX/API/DX12/AllocatorTier1.h"

namespace ZE::GFX::API::DX12
{
	AllocatorTier1::AllocatorTier1(Device& dev)
		: buffers(BUF_HEAP_SIZE), dynamicBuffers(DBUF_HEAP_SIZE), textures(TEX_HEAP_SIZE)
	{
		CreateHeap(dev, &*buffers.Heaps, BUF_FLAG, BUF_HEAP_SIZE);
		CreateHeap(dev, &*dynamicBuffers.Heaps, DBUF_FLAG, DBUF_HEAP_SIZE);
		CreateHeap(dev, &*textures.Heaps, TEX_FLAG, TEX_HEAP_SIZE);
	}
}