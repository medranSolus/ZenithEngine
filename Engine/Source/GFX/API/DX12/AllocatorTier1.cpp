#include "GFX/API/DX12/AllocatorTier1.h"

namespace ZE::GFX::API::DX12
{
	AllocatorTier1::AllocatorTier1(Device& dev, U32 freeListInitSize)
		: buffers(freeListInitSize), textures(freeListInitSize)
	{
		CreateHeap(dev, &*buffers.Heaps, BUF_FLAG);
		CreateHeap(dev, &*textures.Heaps, TEX_FLAG);
	}
}