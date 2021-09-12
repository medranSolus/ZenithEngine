#include "GFX/API/DX12/AllocatorTier2.h"

namespace ZE::GFX::API::DX12
{
	AllocatorTier2::AllocatorTier2(Device& dev)
		: memory(HEAP_SIZE), buffers(BUF_HEAP_SIZE)
	{
		CreateHeap(dev, &*memory.Heaps, HEAP_FLAG, HEAP_SIZE);
		CreateHeap(dev, &*buffers.Heaps, BUF_HEAP_FLAG, BUF_HEAP_SIZE);
	}
}