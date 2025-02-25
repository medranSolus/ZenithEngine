#include "RHI/DX12/Device.h"

namespace ZE::RHI::DX12
{
	void AllocatorGPU::Memory::Init(Memory& chunk, HeapFlags flags, U64 size, void* userData)
	{
		ZE_ASSERT(chunk.Heap.Get() == nullptr, "Incorrect memory block!");
		ZE_ASSERT(userData, "Cannot access GFX::API::DX12::Device for creating heap!");

		Device& dev = *reinterpret_cast<Device*>(userData);
		ZE_DX_ENABLE_ID(dev);

		D3D12_HEAP_DESC desc = {};
		desc.SizeInBytes = size;
		desc.Properties.Type = GetHeapType(flags);
		desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		desc.Properties.CreationNodeMask = 0;
		desc.Properties.VisibleNodeMask = 0;
		desc.Alignment = GetHeapAlignment(flags);
		desc.Flags = GetHeapFlags(flags);

		ZE_DX_THROW_FAILED(dev.GetDevice()->CreateHeap1(&desc, nullptr, IID_PPV_ARGS(&chunk.Heap)));
#if _ZE_DEBUG_GFX_NAMES
		switch (desc.Properties.Type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case D3D12_HEAP_TYPE_DEFAULT:
		{
			ZE_DX_SET_ID(chunk.Heap, "Default heap");
			break;
		}
		case D3D12_HEAP_TYPE_UPLOAD:
		{
			ZE_DX_SET_ID(chunk.Heap, "Upload heap");
			break;
		}
		case D3D12_HEAP_TYPE_READBACK:
		{
			ZE_DX_SET_ID(chunk.Heap, "Readback heap");
			break;
		}
		case D3D12_HEAP_TYPE_CUSTOM:
		{
			ZE_DX_SET_ID(chunk.Heap, "Custom heap");
			break;
		}
		case D3D12_HEAP_TYPE_GPU_UPLOAD:
		{
			ZE_DX_SET_ID(chunk.Heap, "GPU Upload heap");
			break;
		}
		}
#endif
	}

	constexpr U64 AllocatorGPU::GetHeapAlignment(HeapFlags flags) noexcept
	{
		return flags & HeapFlag::NoMSAA ? D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT : D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
	}

	constexpr D3D12_HEAP_TYPE AllocatorGPU::GetHeapType(HeapFlags flags) noexcept
	{
		return flags & HeapFlag::Dynamic ? D3D12_HEAP_TYPE_UPLOAD : (flags & HeapFlag::GpuUploadHeap ? D3D12_HEAP_TYPE_GPU_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
	}

	constexpr D3D12_HEAP_FLAGS AllocatorGPU::GetHeapFlags(HeapFlags flags) noexcept
	{
		return static_cast<D3D12_HEAP_FLAGS>(D3D12_HEAP_FLAG_CREATE_NOT_ZEROED
			| (flags & HeapFlag::CommittedAlloc ? 0 :
				(flags & HeapFlag::AllowBuffers ? 0 : D3D12_HEAP_FLAG_DENY_BUFFERS)
				| (flags & HeapFlag::AllowTextures ? 0 : D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES))
			| (flags & HeapFlag::AllowTexturesRTDS ? 0 : D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES));
	}

	DX::ComPtr<IResource> AllocatorGPU::CreateCommittedResource(Device& dev,
		const D3D12_RESOURCE_DESC1& desc, D3D12_BARRIER_LAYOUT layout, HeapFlags flags)
	{
		ZE_DX_ENABLE(dev);

		D3D12_HEAP_PROPERTIES heapProp = {};
		heapProp.Type = GetHeapType(flags);
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp.CreationNodeMask = 0;
		heapProp.VisibleNodeMask = 0;

		DX::ComPtr<IResource> res = nullptr;
		ZE_DX_THROW_FAILED(dev.GetDevice()->CreateCommittedResource3(&heapProp, GetHeapFlags(flags | HeapFlag::CommittedAlloc),
			&desc, layout, nullptr, nullptr, 0, nullptr, IID_PPV_ARGS(&res)));
		return res;
	}

	DX::ComPtr<IResource> AllocatorGPU::CreateResource(Device& dev, const D3D12_RESOURCE_DESC1& desc,
		D3D12_BARRIER_LAYOUT layout, U64 offset, IHeap* heap, HeapFlags flags)
	{
		ZE_DX_ENABLE(dev);

		DX::ComPtr<IResource> res = nullptr;
		ZE_DX_THROW_FAILED(dev.GetDevice()->CreatePlacedResource2(heap, offset,
			&desc, layout, nullptr, 0, nullptr, IID_PPV_ARGS(&res)));
		return res;
	}

	ResourceInfo AllocatorGPU::Alloc(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc,
		D3D12_BARRIER_LAYOUT layout, U64 alignment, HeapAllocator& allocator)
	{
		if (bytes >= allocator.GetChunkSize())
			return { CreateCommittedResource(dev, desc, layout, allocator.GetChunkCreationFlags()), 0 };

		AllocHandle alloc = allocator.Alloc(bytes, alignment, &dev);
		ZE_ASSERT(alloc != nullptr, "Should always find some blocks or at least create new heap!");

		return { CreateResource(dev, desc, layout, allocator.GetOffset(alloc),
			allocator.GetMemory(alloc).Heap.Get(), allocator.GetChunkCreationFlags()), alloc };
	}

	ResourceInfo AllocatorGPU::AllocBigChunks(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc,
		D3D12_BARRIER_LAYOUT layout, U64 alignment, HeapAllocator& allocator)
	{
		return Alloc(dev, bytes, desc, layout, alignment / allocator.GetChunkSizeGranularity(), allocator);
	}

	ResourceInfo AllocatorGPU::AllocMinimalChunks(Device& dev, U64 bytes,
		const D3D12_RESOURCE_DESC1& desc, D3D12_BARRIER_LAYOUT layout, HeapAllocator& allocator)
	{
		return Alloc(dev, bytes, desc, layout, 1, allocator);
	}

	void AllocatorGPU::Remove(ResourceInfo& resInfo, HeapAllocator& allocator) noexcept
	{
		ZE_ASSERT(resInfo.Resource, "Freeing already destroyed resource!");
		resInfo.Resource = nullptr;

		// Committed resource, nothing inside algorithm
		if (resInfo.Handle == 0)
			return;
		allocator.Free(resInfo.Handle, nullptr);
		resInfo.Handle = 0;
	}

	AllocatorGPU::~AllocatorGPU()
	{
		switch (allocTier)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case AllocTier::Tier1:
		{
			secondaryAllocator.DestroyFreeChunks(nullptr);
			[[fallthrough]];
		}
		case AllocTier::Tier2:
		{
			mainAllocator.DestroyFreeChunks(nullptr);
			break;
		}
		}
		dynamicBuffersAllocator.DestroyFreeChunks(nullptr);
	}

	void AllocatorGPU::Init(Device& dev, D3D12_RESOURCE_HEAP_TIER heapTier, bool gpuUploadHeapSupported)
	{
		allocTier = heapTier == D3D12_RESOURCE_HEAP_TIER_2 ? AllocTier::Tier2 : AllocTier::Tier1;
		const HeapFlags flags = gpuUploadHeapSupported ? HeapFlag::GpuUploadHeap : HeapFlag::None;
		switch (allocTier)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case AllocTier::Tier1:
		{
			mainAllocator.Init(MAIN_HEAP_FLAGS | flags, Settings::BUFFERS_HEAP_SIZE, NORMAL_CHUNK, 3);
			secondaryAllocator.Init(SECONDARY_HEAP_FLAGS | flags, Settings::TEXTURES_HEAP_SIZE, SMALL_CHUNK, 3);
			break;
		}
		case AllocTier::Tier2:
		{
			mainAllocator.Init(MAIN_HEAP_FLAGS | SECONDARY_HEAP_FLAGS | HeapFlag::AllowTexturesRTDS | flags, Settings::BUFFERS_HEAP_SIZE + Settings::TEXTURES_HEAP_SIZE, SMALL_CHUNK, 3);
			break;
		}
		}
		dynamicBuffersAllocator.Init(DYNAMIC_BUFF_HEAP_FLAGS | flags, Settings::UPLOAD_HEAP_SIZE, NORMAL_CHUNK, 3);
		readbackBuffersAllocator.Init(READBACK_BUFF_HEAP_FLAGS | flags, Settings::HOST_HEAP_SIZE, NORMAL_CHUNK, 3);
	}

	ResourceInfo AllocatorGPU::AllocBuffer(Device& dev, const D3D12_RESOURCE_DESC1& desc)
	{
		switch (allocTier)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case AllocTier::Tier1:
			return AllocMinimalChunks(dev, desc.Width, desc, D3D12_BARRIER_LAYOUT_UNDEFINED, mainAllocator);
		case AllocTier::Tier2:
			return AllocBigChunks(dev, desc.Width, desc, D3D12_BARRIER_LAYOUT_UNDEFINED, NORMAL_CHUNK, mainAllocator);
		}
	}

	ResourceInfo AllocatorGPU::AllocDynamicBuffer(Device& dev, const D3D12_RESOURCE_DESC1& desc)
	{
		return AllocMinimalChunks(dev, desc.Width, desc, D3D12_BARRIER_LAYOUT_UNDEFINED, dynamicBuffersAllocator);
	}

	ResourceInfo AllocatorGPU::AllocReadbackBuffer(Device& dev, const D3D12_RESOURCE_DESC1& desc)
	{
		return AllocMinimalChunks(dev, desc.Width, desc, D3D12_BARRIER_LAYOUT_UNDEFINED, readbackBuffersAllocator);
	}

	ResourceInfo AllocatorGPU::AllocTexture_4KB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc)
	{
		return AllocMinimalChunks(dev, bytes, desc, D3D12_BARRIER_LAYOUT_COMMON,
			allocTier == AllocTier::Tier2 ? mainAllocator : secondaryAllocator);
	}

	ResourceInfo AllocatorGPU::AllocTexture_64KB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc)
	{
		return AllocBigChunks(dev, bytes, desc, D3D12_BARRIER_LAYOUT_COMMON,
			NORMAL_CHUNK, allocTier == AllocTier::Tier2 ? mainAllocator : secondaryAllocator);
	}

	ResourceInfo AllocatorGPU::AllocTexture_4MB(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc)
	{
		return AllocBigChunks(dev, bytes, desc, D3D12_BARRIER_LAYOUT_COMMON,
			HUGE_CHUNK, allocTier == AllocTier::Tier2 ? mainAllocator : secondaryAllocator);
	}
}