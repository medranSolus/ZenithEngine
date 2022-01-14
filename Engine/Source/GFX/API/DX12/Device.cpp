#include "GFX/API/DX12/Device.h"
#include "GFX/API/DX/GraphicsException.h"
#include "GFX/CommandList.h"

namespace ZE::GFX::API::DX12
{
	void Device::WaitCPU(ID3D12Fence1* fence, U64 val)
	{
		ZE_WIN_ENABLE_EXCEPT();

		if (fence->GetCompletedValue() < val)
		{
			HANDLE fenceEvent;
			fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			assert(fenceEvent && "Cannot create fence event!");

			ZE_GFX_THROW_FAILED(fence->SetEventOnCompletion(val, fenceEvent));
			if (WaitForSingleObject(fenceEvent, INFINITE) != WAIT_OBJECT_0)
				throw ZE_WIN_EXCEPT_LAST();
		}
	}

	void Device::WaitGPU(ID3D12Fence1* fence, ID3D12CommandQueue* queue, U64 val)
	{
		ZE_WIN_ENABLE_EXCEPT();
		ZE_GFX_THROW_FAILED(queue->Wait(fence, val));
	}

	U64 Device::SetFenceCPU(ID3D12Fence1* fence, UA64& fenceVal)
	{
		ZE_WIN_ENABLE_EXCEPT();
		U64 val = ++fenceVal;
		ZE_GFX_THROW_FAILED(fence->Signal(val));
		return val;
	}

	U64 Device::SetFenceGPU(ID3D12Fence1* fence, ID3D12CommandQueue* queue, UA64& fenceVal)
	{
		ZE_WIN_ENABLE_EXCEPT();
		U64 val = ++fenceVal;
		ZE_GFX_THROW_FAILED(queue->Signal(fence, val));
		return val;
	}

	void Device::Execute(ID3D12CommandQueue* queue, CommandList& cl) noexcept(ZE_NO_DEBUG)
	{
		ZE_ASSERT(cl.GetList() != nullptr, "Empty list!");
		ID3D12CommandList* lists[] = { cl.GetList() };
		ZE_GFX_THROW_FAILED_INFO(queue->ExecuteCommandLists(1, lists));
	}

	Device::Device(U32 descriptorCount, U32 scratchDescriptorCount)
		: descriptorCount(descriptorCount), scratchDescStart(descriptorCount - scratchDescriptorCount)
	{
		ZE_WIN_ENABLE_EXCEPT();
		std::string ZE_GFX_DEBUG_ID;
		assert(descriptorCount > scratchDescriptorCount && "Descriptor count has to be greater than scratch descriptor count!");

#ifdef _ZE_MODE_DEBUG
		// Enable Debug Layer before calling any DirectX commands
		DX::ComPtr<ID3D12Debug> debugInterface;
		ZE_GFX_THROW_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
		debugInterface->EnableDebugLayer();
#endif

		DX::ComPtr<IDXGIAdapter4> adapter = DX::CreateAdapter(
#ifdef _ZE_MODE_DEBUG
			debugManager
#endif
		);
		ZE_GFX_THROW_FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device)));

#ifdef _ZE_MODE_DEBUG
		DX::ComPtr<ID3D12InfoQueue> infoQueue;
		ZE_GFX_THROW_FAILED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)));

		// Set breaks on dangerous messages
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

		// Suppress non important messages
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_MESSAGE_ID denyIds[3] =
		{
			// D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
			// Bug in Visual Studio Graphics Debugger while capturing frame
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
			// When asking for smaller alignment error is generated, silence it
			D3D12_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT
		};

		D3D12_INFO_QUEUE_FILTER filter = { 0 };
		filter.DenyList.NumSeverities = 1;
		filter.DenyList.pSeverityList = severities;
		filter.DenyList.NumIDs = 3;
		filter.DenyList.pIDList = denyIds;

		ZE_GFX_THROW_FAILED(infoQueue->PushStorageFilter(&filter));
#endif
		D3D12_COMMAND_QUEUE_DESC desc;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ZE_GFX_THROW_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&mainQueue)));
		ZE_GFX_SET_ID(mainQueue, "direct_queue");
		ZE_GFX_THROW_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mainFence)));
		ZE_GFX_SET_ID(mainFence, "direct_fence");

		desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		ZE_GFX_THROW_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&computeQueue)));
		ZE_GFX_SET_ID(computeQueue, "compute_queue");
		ZE_GFX_THROW_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&computeFence)));
		ZE_GFX_SET_ID(computeFence, "computev");

		desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		ZE_GFX_THROW_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&copyQueue)));
		ZE_GFX_SET_ID(copyQueue, "copy_queue");
		ZE_GFX_THROW_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&copyFence)));
		ZE_GFX_SET_ID(copyFence, "copy_fence");

		D3D12_FEATURE_DATA_D3D12_OPTIONS options = { 0 };
		ZE_WIN_THROW_FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)));
		if (options.ResourceHeapTier == D3D12_RESOURCE_HEAP_TIER_1)
		{
			new(&allocTier1) AllocatorTier1(*this);
			allocTier = AllocTier::Tier1;
		}
		else
		{
			new(&allocTier2) AllocatorTier2(*this);
			allocTier = AllocTier::Tier2;
		}

		copyList.Init(*this, CommandType::All);
		copyResInfo.Size = 0;
		copyResInfo.Allocated = COPY_LIST_GROW_SIZE;

		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
		descHeapDesc.NodeMask = 0;
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descHeapDesc.NumDescriptors = descriptorCount;
		ZE_GFX_THROW_FAILED(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap)));
		descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	Device::~Device()
	{
		if (commandLists)
			delete[] commandLists;
		if (allocTier == AllocTier::Tier1)
			allocTier1.~AllocatorTier1();
		else
			allocTier2.~AllocatorTier2();
		if (copyResList != nullptr)
			Table::Clear(copyResInfo.Size, copyResList);

#ifdef _ZE_MODE_DEBUG
		DX::ComPtr<ID3D12DebugDevice> debug;
		device->QueryInterface(IID_PPV_ARGS(&debug));
		if (debug != nullptr)
			debug->ReportLiveDeviceObjects(D3D12_RLDO_IGNORE_INTERNAL);
#endif
	}

	void Device::StartUpload()
	{
		ZE_ASSERT(copyResList == nullptr, "Finish previous upload first!");
		copyList.Open(*this);
		copyResList = Table::Create<UploadInfo>(COPY_LIST_GROW_SIZE);
	}

	void Device::FinishUpload()
	{
		ZE_ASSERT(copyResList != nullptr, "Empty upload list!");
		ZE_WIN_ENABLE_EXCEPT();

		D3D12_RESOURCE_BARRIER* barriers = nullptr;
		if (copyResInfo.Size)
		{
			barriers = new D3D12_RESOURCE_BARRIER[copyResInfo.Size];
			for (U16 i = 0; i < copyResInfo.Size; ++i)
			{
				auto& barrier = barriers[i];
				auto& copyInfo = copyResList[i];
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				barrier.Transition.StateAfter = copyInfo.FinalState;
				barrier.Transition.pResource = copyInfo.Destination;
			}
			copyList.GetList()->ResourceBarrier(copyResInfo.Size, barriers);
		}
		copyList.Close(*this);
		Execute(mainQueue.Get(), copyList);

		U64 fenceVal = ++mainFenceVal;
		ZE_GFX_THROW_FAILED(mainQueue->Signal(mainFence.Get(), fenceVal));
		WaitMain(fenceVal);
		copyList.Reset(*this);
		if (barriers)
			delete[] barriers;

		Table::Clear(copyResInfo.Size, copyResList);
		copyResInfo.Size = 0;
		copyResInfo.Allocated = COPY_LIST_GROW_SIZE;
	}

	void Device::Execute(GFX::CommandList* cls, U32 count) noexcept(ZE_NO_DEBUG)
	{
		if (count == 1)
		{
			switch (cls->Get().dx12.GetType())
			{
			case D3D12_COMMAND_LIST_TYPE_DIRECT:
				return ExecuteMain(*cls);
			case D3D12_COMMAND_LIST_TYPE_COMPUTE:
				return ExecuteCompute(*cls);
			case D3D12_COMMAND_LIST_TYPE_COPY:
				return ExecuteCopy(*cls);
			default:
				assert(false && "Incorrect type of command list!!!");
			}
		}
		U32 mainCount = 0, computeCount = 0, copyCount = 0;
		for (U32 i = 0; i < count; ++i)
		{
			switch (cls[i].Get().dx12.GetType())
			{
			case D3D12_COMMAND_LIST_TYPE_DIRECT:
			{
				assert(cls[i].Get().dx12.GetList() != nullptr);
				commandLists[mainCount++] = cls[i].Get().dx12.GetList();
				break;
			}
			case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			{
				assert(cls[i].Get().dx12.GetList() != nullptr);
				commandLists[count + computeCount++] = cls[i].Get().dx12.GetList();
				break;
			}
			case D3D12_COMMAND_LIST_TYPE_COPY:
			{
				assert(cls[i].Get().dx12.GetList() != nullptr);
				commandLists[2 * count + copyCount++] = cls[i].Get().dx12.GetList();
				break;
			}
			default:
				assert(false && "Incorrect type of command list!!!");
			}
		}
		if (mainCount)
		{
			ZE_GFX_THROW_FAILED_INFO(mainQueue->ExecuteCommandLists(mainCount, commandLists));
		}
		if (computeCount)
		{
			ZE_GFX_THROW_FAILED_INFO(computeQueue->ExecuteCommandLists(computeCount, commandLists + count));
		}
		if (copyCount)
		{
			ZE_GFX_THROW_FAILED_INFO(copyQueue->ExecuteCommandLists(copyCount, commandLists + 2 * static_cast<U64>(count)));
		}
	}

	void Device::ExecuteMain(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG)
	{
		Execute(mainQueue.Get(), cl.Get().dx12);
	}

	void Device::ExecuteCompute(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG)
	{
		Execute(computeQueue.Get(), cl.Get().dx12);
	}

	void Device::ExecuteCopy(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG)
	{
		Execute(copyQueue.Get(), cl.Get().dx12);
	}

	D3D12_RESOURCE_DESC Device::GetBufferDesc(U64 size) const noexcept
	{
		ZE_ASSERT(size, "Cannot create empty buffer!");

		D3D12_RESOURCE_DESC desc;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Width = size;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		return desc;
	}

	std::pair<D3D12_RESOURCE_DESC, U32> Device::GetTextureDesc(U32 width, U32 height,
		U16 count, DXGI_FORMAT format, bool is3D) const noexcept
	{
		ZE_ASSERT(width < D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION
			&& height < D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION,
			"Texture too big!");

		D3D12_RESOURCE_DESC desc;
		desc.Dimension = is3D ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Width = width;
		desc.Height = height;
		desc.DepthOrArraySize = count;
		desc.MipLevels = 1; // TODO: Create mip generation module
		desc.Format = format; // Maybe not all formats supported on given hardware, if strange formats to be used check D3D12_FORMAT_SUPPORT1
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_RESOURCE_ALLOCATION_INFO info = device->GetResourceAllocationInfo(0, 1, &desc);
		if (info.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
		{
			desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			info = device->GetResourceAllocationInfo(0, 1, &desc);
		}

		return { desc, static_cast<U32>(info.SizeInBytes) };
	}

	ResourceInfo Device::CreateBuffer(const D3D12_RESOURCE_DESC& desc)
	{
		if (allocTier == AllocTier::Tier1)
			return allocTier1.AllocBuffer(*this, static_cast<U32>(desc.Width), desc);
		else
			return allocTier2.Alloc_64KB(*this, static_cast<U32>(desc.Width), desc);
	}

	ResourceInfo Device::CreateTexture(const std::pair<D3D12_RESOURCE_DESC, U32>& desc)
	{
		if (desc.first.Alignment == D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
		{
			if (allocTier == AllocTier::Tier1)
				return allocTier1.AllocTexture_64KB(*this, desc.second, desc.first);
			else
				return allocTier2.Alloc_64KB(*this, desc.second, desc.first);
		}
		else if (allocTier == AllocTier::Tier1)
			return allocTier1.AllocTexture_4KB(*this, desc.second, desc.first);
		else
			return allocTier2.Alloc_4KB(*this, desc.second, desc.first);
	}

	DX::ComPtr<ID3D12Resource> Device::CreateTextureUploadBuffer(U64 size)
	{
		ZE_WIN_ENABLE_EXCEPT();

		auto desc = GetBufferDesc(size);
		D3D12_HEAP_PROPERTIES tempHeap;
		tempHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
		tempHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		tempHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		tempHeap.CreationNodeMask = 0;
		tempHeap.VisibleNodeMask = 0;

		DX::ComPtr<ID3D12Resource> uploadRes;
		ZE_GFX_THROW_FAILED(device->CreateCommittedResource(&tempHeap,
			D3D12_HEAP_FLAG_CREATE_NOT_ZEROED, &desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadRes)));
		return uploadRes;
	}

	void Device::UploadBuffer(ID3D12Resource* dest, const D3D12_RESOURCE_DESC& desc,
		const void* data, U64 size, D3D12_RESOURCE_STATES finalState)
	{
		ZE_ASSERT(copyResList != nullptr, "Empty upload list!");
		ZE_ASSERT(desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER, "Wrong resource type!");
		ZE_WIN_ENABLE_EXCEPT();

		// Create upload heap and temporary resource
		D3D12_HEAP_PROPERTIES tempHeap;
		tempHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
		tempHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		tempHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		tempHeap.CreationNodeMask = 0;
		tempHeap.VisibleNodeMask = 0;

		DX::ComPtr<ID3D12Resource> uploadRes;
		ZE_GFX_THROW_FAILED(device->CreateCommittedResource(&tempHeap,
			D3D12_HEAP_FLAG_CREATE_NOT_ZEROED, &desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadRes)));

		// Map and copy data into upload buffer
		D3D12_RANGE range = { 0 };
		void* uploadBuffer = nullptr;
		ZE_GFX_THROW_FAILED(uploadRes->Map(0, &range, &uploadBuffer));
		memcpy(uploadBuffer, data, size);
		uploadRes->Unmap(0, nullptr);

		// Perform copy and save upload resource till FinishUpload is called
		copyList.GetList()->CopyResource(dest, uploadRes.Get());
		Table::Append<COPY_LIST_GROW_SIZE>(copyResInfo, copyResList, UploadInfo(finalState, dest, std::move(uploadRes)));
	}

	void Device::UploadTexture(const D3D12_TEXTURE_COPY_LOCATION& dest,
		const D3D12_TEXTURE_COPY_LOCATION& source, D3D12_RESOURCE_STATES finalState)
	{
		ZE_ASSERT(copyResList != nullptr, "Empty upload list!");

		copyList.GetList()->CopyTextureRegion(&dest, 0, 0, 0, &source, nullptr);
		if (dest.SubresourceIndex == 0)
			Table::Append<COPY_LIST_GROW_SIZE>(copyResInfo, copyResList, UploadInfo(finalState, dest.pResource, source.pResource));
	}

	void Device::FreeBuffer(ResourceInfo& info) noexcept
	{
		D3D12_RESOURCE_DESC desc = info.Resource->GetDesc();
		FreeBuffer(info, static_cast<U32>(device->GetResourceAllocationInfo(0, 1, &desc).SizeInBytes));
	}

	void Device::FreeBuffer(ResourceInfo& info, U32 size) noexcept
	{
		info.Resource = nullptr;
		if (allocTier == AllocTier::Tier1)
			allocTier1.RemoveBuffer(info.ID, size);
		else
			allocTier2.Remove(info.ID, size);
	}

	void Device::FreeTexture(ResourceInfo& info) noexcept
	{
		D3D12_RESOURCE_DESC desc = info.Resource->GetDesc();
		U32 size = static_cast<U32>(device->GetResourceAllocationInfo(0, 1, &desc).SizeInBytes);
		info.Resource = nullptr;
		if (allocTier == AllocTier::Tier1)
			allocTier1.RemoveTexture(info.ID, size);
		else
			allocTier2.Remove(info.ID, size);
	}

	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> Device::AddStaticDescs(U32 count) noexcept
	{
		ZE_ASSERT(dynamicDescStart + count < scratchDescStart,
			"Prepared too small range for static descriptors, increase pool size!");

		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> rangeStart;
		U64 offest = static_cast<U64>(dynamicDescStart) * descriptorSize;
		rangeStart.first.ptr = descHeap->GetCPUDescriptorHandleForHeapStart().ptr + offest;
		rangeStart.second.ptr = descHeap->GetGPUDescriptorHandleForHeapStart().ptr + offest;
		dynamicDescStart += count;
		return rangeStart;
	}

	DescriptorInfo Device::AllocDescs(U32 count) noexcept
	{
		ZE_ASSERT(dynamicDescStart + dynamicDescCount + count < scratchDescStart,
			"Prepared too small range for dynamic descriptors, increase pool size!");

		DescriptorInfo rangeStart;
		rangeStart.ID = dynamicDescStart + dynamicDescCount;
		U64 offest = static_cast<U64>(rangeStart.ID) * descriptorSize;
		rangeStart.GPU.ptr = descHeap->GetGPUDescriptorHandleForHeapStart().ptr + offest;
		rangeStart.CPU.ptr = descHeap->GetCPUDescriptorHandleForHeapStart().ptr + offest;
		dynamicDescCount += count;
		return rangeStart;
	}
}