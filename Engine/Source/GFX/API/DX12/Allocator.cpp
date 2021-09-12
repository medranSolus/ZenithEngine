#include "GFX/API/DX12/Allocator.h"
#include "GFX/API/DX/GraphicsException.h"
#include "GFX/API/DX12/Device.h"

namespace ZE::GFX::API::DX12
{
	Allocator::BufferInfo::BufferInfo(U32 heapSize) noexcept
	{
		FreeInfo.Size = 1;
		FreeInfo.Allocated = FREE_LIST_CHUNKS_GROW;
		FreeList = Table::Create<MemInfo>(FREE_LIST_CHUNKS_GROW);
		FreeList[0].Index = 0;
		FreeList[0].Size = heapSize;
		HeapsInfo.Size = 1;
		HeapsInfo.Allocated = HEAP_CHUNKS_GROW;
		Heaps = Table::Create<DX::ComPtr<ID3D12Heap>>(HEAP_CHUNKS_GROW);
	}

	Allocator::BufferInfo::~BufferInfo()
	{
		Table::Clear(FreeInfo.Size, FreeList);
		Table::Clear(HeapsInfo.Size, Heaps);
	}

	void Allocator::Sort(BufferInfo& bufferInfo, U32 element)
	{
		assert(element < bufferInfo.FreeInfo.Size);
		if (bufferInfo.FreeInfo.Size == 1)
			return;

		U32 next = element + 1;
		while (next < bufferInfo.FreeInfo.Size && bufferInfo.FreeList[element].Size > bufferInfo.FreeList[next].Size)
		{
			std::swap(bufferInfo.FreeList[next], bufferInfo.FreeList[element]);
			++element;
			++next;
		}
		next -= 2;
		while (element > 0 && bufferInfo.FreeList[element].Size < bufferInfo.FreeList[next].Size)
		{
			std::swap(bufferInfo.FreeList[element], bufferInfo.FreeList[next]);
			--element;
			--next;
		}
	}

	DX::ComPtr<ID3D12Resource> Allocator::Alloc(Device& dev, const D3D12_RESOURCE_DESC& desc,
		ID3D12Heap* heap, U64 offset, HeapFlags flags)
	{
		ZE_GFX_ENABLE(dev);

		DX::ComPtr<ID3D12Resource> res;
		ZE_GFX_THROW_FAILED(dev.GetDevice()->CreatePlacedResource(heap, offset, &desc,
			flags & HeapFlags::Dynamic ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&res)));
		return res;
	}

	void Allocator::CreateHeap(Device& dev, ID3D12Heap** heap, HeapFlags flags, U64 heapSize)
	{
		ZE_GFX_ENABLE(dev);

		D3D12_HEAP_DESC desc;
		desc.SizeInBytes = heapSize;
		desc.Properties.Type = flags & HeapFlags::Dynamic ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
		desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		desc.Properties.CreationNodeMask = 0;
		desc.Properties.VisibleNodeMask = 0;
		desc.Alignment = D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Flags = static_cast<D3D12_HEAP_FLAGS>(D3D12_HEAP_FLAG_CREATE_NOT_ZEROED | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES
			| (flags & HeapFlags::AllowBuffers ? 0 : D3D12_HEAP_FLAG_DENY_BUFFERS)
			| (flags & HeapFlags::AllowTextures ? 0 : D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES));

		ZE_GFX_THROW_FAILED(dev.GetDevice()->CreateHeap(&desc, IID_PPV_ARGS(heap)));
	}
}