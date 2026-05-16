#include "RHI/DX12/Device.h"

namespace ZE::RHI::DX12
{
	Status AllocatorGPU::Memory::Init(Memory& chunk, HeapFlags flags, U64 size, void* userData) noexcept
	{
		ZE_ASSERT(chunk.Heap.Get() == nullptr, "Incorrect memory block!");
		ZE_ASSERT(userData, "Cannot access GFX::API::DX12::Device for creating heap!");

		Device& dev = *reinterpret_cast<Device*>(userData);

		D3D12_HEAP_DESC desc = {};
		desc.SizeInBytes = size;
		desc.Properties.Type = GetHeapType(flags);
		desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		desc.Properties.CreationNodeMask = 0;
		desc.Properties.VisibleNodeMask = 0;
		desc.Alignment = GetHeapAlignment(flags);
		desc.Flags = GetHeapFlags(flags);

		ZE_DX_RET_FAILED(dev.GetDevice()->CreateHeap1(&desc, nullptr, IID_PPV_ARGS(&chunk.Heap)));
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
		return {};
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

	Expected<DX::ComPtr<IResource>> AllocatorGPU::CreateCommittedResource(Device& dev,
		const D3D12_RESOURCE_DESC1& desc, D3D12_BARRIER_LAYOUT layout, HeapFlags flags) noexcept
	{
		D3D12_HEAP_PROPERTIES heapProp = {};
		heapProp.Type = GetHeapType(flags);
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp.CreationNodeMask = 0;
		heapProp.VisibleNodeMask = 0;

		DX::ComPtr<IResource> res = nullptr;
		ZE_DX_RET_FAILED_EXPECT(dev.GetDevice()->CreateCommittedResource3(&heapProp, GetHeapFlags(flags | HeapFlag::CommittedAlloc),
			&desc, layout, nullptr, nullptr, 0, nullptr, IID_PPV_ARGS(&res)));
		return res;
	}

	Expected<DX::ComPtr<IResource>> AllocatorGPU::CreateResource(Device& dev, const D3D12_RESOURCE_DESC1& desc,
		D3D12_BARRIER_LAYOUT layout, U64 offset, IHeap* heap, HeapFlags flags) noexcept
	{
		DX::ComPtr<IResource> res = nullptr;
		ZE_DX_RET_FAILED_EXPECT(dev.GetDevice()->CreatePlacedResource2(heap, offset,
			&desc, layout, nullptr, 0, nullptr, IID_PPV_ARGS(&res)));
		return res;
	}

	Expected<ResourceInfo> AllocatorGPU::Alloc(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc,
		D3D12_BARRIER_LAYOUT layout, U64 alignment, HeapAllocator& allocator) noexcept
	{
		ResourceInfo info = {};
		if (bytes >= allocator.GetChunkSize())
		{
			ZE_EXPECT_RET_FAILED(info.Resource, CreateCommittedResource(dev, desc, layout, allocator.GetChunkCreationFlags()));
			return info;
		}

		info.Handle = allocator.Alloc(bytes, alignment, &dev);
		if (!info.Handle)
		{
			ZE_FAIL("Failed to allocate GPU memory!");
			return std::unexpected(DX::Error::Make(DX::Error::ALLOC_ERROR));
		}

		auto exp = CreateResource(dev, desc, layout, allocator.GetOffset(info.Handle),
			allocator.GetMemory(info.Handle).Heap.Get(), allocator.GetChunkCreationFlags());
		if (!exp)
		{
			allocator.Free(info.Handle, &dev);
			return std::unexpected(exp.error());
		}
		info.Resource = std::move(*exp);
		return info;
	}

	Expected<ResourceInfo> AllocatorGPU::AllocBigChunks(Device& dev, U64 bytes, const D3D12_RESOURCE_DESC1& desc,
		D3D12_BARRIER_LAYOUT layout, U64 alignment, HeapAllocator& allocator) noexcept
	{
		return Alloc(dev, bytes, desc, layout, alignment / allocator.GetChunkSizeGranularity(), allocator);
	}

	Expected<ResourceInfo> AllocatorGPU::AllocMinimalChunks(Device& dev, U64 bytes,
		const D3D12_RESOURCE_DESC1& desc, D3D12_BARRIER_LAYOUT layout, HeapAllocator& allocator) noexcept
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

	AllocatorGPU::AllocatorGPU() noexcept
		: blockAllocator(std::make_shared<HeapAllocator::BlockAllocator>(BLOCK_ALLOC_CAPACITY)),
		chunkAllocator(std::make_shared<HeapAllocator::ChunkAllocator>(CHUNK_ALLOC_CAPACITY)),
		mainAllocator(blockAllocator, chunkAllocator),
		secondaryAllocator(blockAllocator, chunkAllocator),
		dynamicBuffersAllocator(blockAllocator, chunkAllocator),
		readbackBuffersAllocator(blockAllocator, chunkAllocator)
	{}

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

	Expected<AllocatorGPU> AllocatorGPU::Create(Device& dev, D3D12_RESOURCE_HEAP_TIER heapTier, bool gpuUploadHeapSupported, D3D12_TIGHT_ALIGNMENT_TIER alignmentTier) noexcept
	{
		AllocatorGPU allocator = {};
		allocator.allocTier = heapTier == D3D12_RESOURCE_HEAP_TIER_2 ? AllocTier::Tier2 : AllocTier::Tier1;
		allocator.tightAlignment = alignmentTier != D3D12_TIGHT_ALIGNMENT_TIER_NOT_SUPPORTED;

		const HeapFlags flags = gpuUploadHeapSupported ? HeapFlag::GpuUploadHeap : HeapFlag::None;
		const U32 normalChunk = allocator.tightAlignment ? TIGHT_CHUNK : NORMAL_CHUNK;
		const U32 smallChunk = allocator.tightAlignment ? TIGHT_CHUNK : SMALL_CHUNK;
		switch (allocator.allocTier)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case AllocTier::Tier1:
		{
			ZE_CODE_RET_FAILED_EXPECT(allocator.mainAllocator.Init(MAIN_HEAP_FLAGS | flags, Settings::GetHeapSizes().BuffersHeapSize, normalChunk, 3));
			ZE_CODE_RET_FAILED_EXPECT(allocator.secondaryAllocator.Init(SECONDARY_HEAP_FLAGS | flags, Settings::GetHeapSizes().TexturesHeapSize, smallChunk, 3));
			break;
		}
		case AllocTier::Tier2:
		{
			ZE_CODE_RET_FAILED_EXPECT(allocator.mainAllocator.Init(MAIN_HEAP_FLAGS | SECONDARY_HEAP_FLAGS | HeapFlag::AllowTexturesRTDS | flags, Settings::GetHeapSizes().BuffersHeapSize + Settings::GetHeapSizes().TexturesHeapSize, smallChunk, 3));
			break;
		}
		}
		ZE_CODE_RET_FAILED_EXPECT(allocator.dynamicBuffersAllocator.Init(DYNAMIC_BUFF_HEAP_FLAGS | flags, Settings::GetHeapSizes().UploadHeapSize, normalChunk, 3));
		ZE_CODE_RET_FAILED_EXPECT(allocator.readbackBuffersAllocator.Init(READBACK_BUFF_HEAP_FLAGS | flags, Settings::GetHeapSizes().HostHeapSize, normalChunk, 3));
		return allocator;
	}

	Expected<ResourceInfo> AllocatorGPU::AllocBuffer(Device& dev, const D3D12_RESOURCE_DESC1& desc) noexcept
	{
		if (tightAlignment)
		{
			D3D12_RESOURCE_ALLOCATION_INFO1 info = {};
			dev.GetDevice()->GetResourceAllocationInfo3(0, 1, &desc, nullptr, nullptr, &info);

			if (info.Alignment > TIGHT_CHUNK)
				return AllocBigChunks(dev, desc.Width, desc, D3D12_BARRIER_LAYOUT_UNDEFINED, info.Alignment, mainAllocator);
			return AllocMinimalChunks(dev, desc.Width, desc, D3D12_BARRIER_LAYOUT_UNDEFINED, mainAllocator);
		}

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

	Expected<ResourceInfo> AllocatorGPU::AllocDynamicBuffer(Device& dev, const D3D12_RESOURCE_DESC1& desc) noexcept
	{
		if (tightAlignment)
		{
			D3D12_RESOURCE_ALLOCATION_INFO1 info = {};
			dev.GetDevice()->GetResourceAllocationInfo3(0, 1, &desc, nullptr, nullptr, &info);

			if (info.Alignment > TIGHT_CHUNK)
				return AllocBigChunks(dev, desc.Width, desc, D3D12_BARRIER_LAYOUT_UNDEFINED, info.Alignment, dynamicBuffersAllocator);
		}
		return AllocMinimalChunks(dev, desc.Width, desc, D3D12_BARRIER_LAYOUT_UNDEFINED, dynamicBuffersAllocator);
	}

	Expected<ResourceInfo> AllocatorGPU::AllocReadbackBuffer(Device& dev, const D3D12_RESOURCE_DESC1& desc) noexcept
	{
		if (tightAlignment)
		{
			D3D12_RESOURCE_ALLOCATION_INFO1 info = {};
			dev.GetDevice()->GetResourceAllocationInfo3(0, 1, &desc, nullptr, nullptr, &info);

			if (info.Alignment > TIGHT_CHUNK)
				return AllocBigChunks(dev, desc.Width, desc, D3D12_BARRIER_LAYOUT_UNDEFINED, info.Alignment, readbackBuffersAllocator);
		}
		return AllocMinimalChunks(dev, desc.Width, desc, D3D12_BARRIER_LAYOUT_UNDEFINED, readbackBuffersAllocator);
	}

	Expected<ResourceInfo> AllocatorGPU::AllocTexture(Device& dev, const D3D12_RESOURCE_DESC1& desc) noexcept
	{
		// SMALL_CHUNK -> Only small textures (smaller than 64KB)
		// NORMAL_CHUNK -> Only normal textures and small multisampled textures (smaller than 4MB)
		// HUGE_CHUNK -> Only multisampled textures
		auto& allocator = allocTier == AllocTier::Tier2 ? mainAllocator : secondaryAllocator;

		D3D12_RESOURCE_ALLOCATION_INFO1 info = {};
		dev.GetDevice()->GetResourceAllocationInfo3(0, 1, &desc, nullptr, nullptr, &info);
		if (tightAlignment)
		{
			if (info.Alignment > TIGHT_CHUNK)
				return AllocBigChunks(dev, info.SizeInBytes, desc, D3D12_BARRIER_LAYOUT_COMMON, info.Alignment, allocator);
			return AllocMinimalChunks(dev, info.SizeInBytes, desc, D3D12_BARRIER_LAYOUT_COMMON, allocator);
		}

		if (desc.Alignment == SMALL_CHUNK)
			return AllocMinimalChunks(dev, info.SizeInBytes, desc, D3D12_BARRIER_LAYOUT_COMMON, allocator);
		return AllocBigChunks(dev, info.SizeInBytes, desc, D3D12_BARRIER_LAYOUT_COMMON, desc.Alignment, allocator);
	}
}