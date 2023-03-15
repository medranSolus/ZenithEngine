#include "GFX/API/DX12/Device.h"
#include "GFX/API/DX12/DREDRecovery.h"
#include "GFX/CommandList.h"

namespace ZE::GFX::API::DX12
{
	void Device::WaitCPU(IFence* fence, U64 val)
	{
		ZE_WIN_ENABLE_EXCEPT();

		if (fence->GetCompletedValue() < val)
		{
			HANDLE fenceEvent;
			fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			assert(fenceEvent && "Cannot create fence event!");

			ZE_DX_THROW_FAILED(fence->SetEventOnCompletion(val, fenceEvent));
			if (WaitForSingleObject(fenceEvent, INFINITE) != WAIT_OBJECT_0)
				throw ZE_WIN_EXCEPT_LAST();
		}
	}

	void Device::WaitGPU(IFence* fence, ICommandQueue* queue, U64 val)
	{
		ZE_WIN_ENABLE_EXCEPT();
		ZE_DX_THROW_FAILED(queue->Wait(fence, val));
	}

	U64 Device::SetFenceCPU(IFence* fence, UA64& fenceVal)
	{
		ZE_WIN_ENABLE_EXCEPT();
		U64 val = ++fenceVal;
		ZE_DX_THROW_FAILED(fence->Signal(val));
		return val;
	}

	U64 Device::SetFenceGPU(IFence* fence, ICommandQueue* queue, UA64& fenceVal)
	{
		ZE_WIN_ENABLE_EXCEPT();
		U64 val = ++fenceVal;
		ZE_DX_THROW_FAILED(queue->Signal(fence, val));
		return val;
	}

	void Device::Execute(ICommandQueue* queue, CommandList& cl)
	{
		ZE_ASSERT(cl.GetList() != nullptr, "Empty list!");
		ICommandList* lists[] = { cl.GetList() };
		ZE_DX_THROW_FAILED_INFO(queue->ExecuteCommandLists(1, lists));
	}

	Device::Device(const Window::MainWindow& window, U32 descriptorCount, U32 scratchDescriptorCount)
		: scratchDescStart(descriptorCount - scratchDescriptorCount), descriptorCount(descriptorCount)
	{
		ZE_ASSERT(descriptorCount > scratchDescriptorCount, "Descriptor count has to be greater than scratch descriptor count!");

		ZE_WIN_ENABLE_EXCEPT();
#if _ZE_DEBUG_GFX_NAMES
		std::string ZE_DX_DEBUG_ID;
#endif
		// No support for 8 bit indices on DirectX
		Settings::SetU8IndexSets(false);

#if _ZE_DEBUG_GFX_API
		// Enable Debug Layer before calling any DirectX commands
		DX::ComPtr<IDebug> debugInterface;
		ZE_DX_THROW_FAILED_NOINFO(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
		debugInterface->EnableDebugLayer();

		// Enable device removed recovery
		DREDRecovery::Enable(debugManager);

#if _ZE_DEBUG_GPU_VALIDATION
		debugInterface->SetEnableGPUBasedValidation(TRUE);
#endif
#endif

		DX::ComPtr<DX::IAdapter> adapter = DX::CreateAdapter(
#if _ZE_DEBUG_GFX_API
			debugManager
#endif
		);

		// Initialize via hardware specific functions
		switch (Settings::GetGpuVendor())
		{
		case VendorGPU::AMD:
		{
			AGSConfiguration agsConfig = {};
			if (agsInitialize(AGS_MAKE_VERSION(AMD_AGS_VERSION_MAJOR, AMD_AGS_VERSION_MINOR, AMD_AGS_VERSION_PATCH),
				&agsConfig, &gpuCtxAMD, nullptr) == AGS_SUCCESS)
			{
				AGSDX12DeviceCreationParams deviceParams;
				deviceParams.pAdapter = adapter.Get();
				deviceParams.iid = __uuidof(device);
				deviceParams.FeatureLevel = MINIMAL_D3D_LEVEL;

				AGSDX12ExtensionParams extensionParams = {};
				AGSDX12ReturnedParams returnParams;
				if (agsDriverExtensionsDX12_CreateDevice(gpuCtxAMD, &deviceParams, &extensionParams, &returnParams) == AGS_SUCCESS)
				{
					ZE_DX_THROW_FAILED(returnParams.pDevice->QueryInterface(IID_PPV_ARGS(&device)));
					returnParams.pDevice->Release();
					break;
				}
				agsDeInitialize(gpuCtxAMD);
				gpuCtxAMD = nullptr;
			}
			Settings::SetGpuVendor(VendorGPU::Unknown);
			break;
		}
		}

		// Failed to create GPU specific device
		if (device == nullptr)
		{
			ZE_DX_THROW_FAILED_NOINFO(D3D12CreateDevice(adapter.Get(), MINIMAL_D3D_LEVEL, IID_PPV_ARGS(&device)));
		}

#if _ZE_DEBUG_GFX_API
		DX::ComPtr<IInfoQueue> infoQueue;
		ZE_DX_THROW_FAILED(device.As(&infoQueue));

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

		D3D12_INFO_QUEUE_FILTER filter = { { 0 } };
		filter.DenyList.NumSeverities = 1;
		filter.DenyList.pSeverityList = severities;
		filter.DenyList.NumIDs = 3;
		filter.DenyList.pIDList = denyIds;

		ZE_DX_THROW_FAILED(infoQueue->PushStorageFilter(&filter));

#if _ZE_DEBUG_GPU_VALIDATION
		DX::ComPtr<IDebugDevice> debugDevice;
		ZE_DX_THROW_FAILED_NOINFO(device.As(&debugDevice));

		const D3D12_DEBUG_FEATURE debugFeature = D3D12_DEBUG_FEATURE_ALLOW_BEHAVIOR_CHANGING_DEBUG_AIDS;
		ZE_DX_THROW_FAILED(debugDevice->SetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_FEATURE_FLAGS,
			&debugFeature, sizeof(D3D12_DEBUG_FEATURE)));

		D3D12_DEBUG_DEVICE_GPU_BASED_VALIDATION_SETTINGS validationSettings;
		// Should cover all messages
		validationSettings.MaxMessagesPerCommandList = 1024;
		// Can avoid most cases of TDRs
		validationSettings.DefaultShaderPatchMode = D3D12_GPU_BASED_VALIDATION_SHADER_PATCH_MODE_GUARDED_VALIDATION;
		validationSettings.PipelineStateCreateFlags = D3D12_GPU_BASED_VALIDATION_PIPELINE_STATE_CREATE_FLAG_FRONT_LOAD_CREATE_GUARDED_VALIDATION_SHADERS;
		ZE_DX_THROW_FAILED(debugDevice->SetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_GPU_BASED_VALIDATION_SETTINGS,
			&validationSettings, sizeof(D3D12_DEBUG_DEVICE_GPU_BASED_VALIDATION_SETTINGS)));
#endif
#endif
		D3D12_COMMAND_QUEUE_DESC desc;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ZE_DX_THROW_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&mainQueue)));
		ZE_DX_SET_ID(mainQueue, "direct_queue");
		ZE_DX_THROW_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mainFence)));
		ZE_DX_SET_ID(mainFence, "direct_fence");

		desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		ZE_DX_THROW_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&computeQueue)));
		ZE_DX_SET_ID(computeQueue, "compute_queue");
		ZE_DX_THROW_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&computeFence)));
		ZE_DX_SET_ID(computeFence, "compute_fence");

		desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		ZE_DX_THROW_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&copyQueue)));
		ZE_DX_SET_ID(copyQueue, "copy_queue");
		ZE_DX_THROW_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&copyFence)));
		ZE_DX_SET_ID(copyFence, "copy_fence");

		copyList.Init(*this, QueueType::Main);
		copyResInfo.Size = 0;
		copyResInfo.Allocated = COPY_LIST_GROW_SIZE;

		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
		descHeapDesc.NodeMask = 0;
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descHeapDesc.NumDescriptors = descriptorCount;
		ZE_DX_THROW_FAILED(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap)));
		descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_FEATURE_DATA_D3D12_OPTIONS options = { 0 };
		ZE_WIN_THROW_FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)));
		allocator.Init(*this, options.ResourceHeapTier);
	}

	Device::~Device()
	{
		ZE_ASSERT(copyResInfo.Size == 0, "Copying data not finished before destroying Device!");

		copyList.Free();
		if (commandLists)
			commandLists.Free();
		if (copyResList != nullptr)
			Table::Clear(copyResInfo.Size, copyResList);

		switch (Settings::GetGpuVendor())
		{
		case VendorGPU::AMD:
		{
			agsDriverExtensionsDX12_DestroyDevice(gpuCtxAMD, device.Get(), nullptr);
			device.Detach();
			agsDeInitialize(gpuCtxAMD);
			break;
		}
		}
	}

	void Device::BeginUploadRegion()
	{
		ZE_ASSERT(copyResList == nullptr, "Finish previous upload first!");
		copyList.Open(*this);
		copyResList = Table::Create<UploadInfo>(COPY_LIST_GROW_SIZE);
	}

	void Device::StartUpload()
	{
		ZE_ASSERT(copyResList != nullptr, "Empty upload list!");

		U16 size = copyResInfo.Size - copyOffset;
		if (size)
		{
			D3D12_RESOURCE_BARRIER* barriers = new D3D12_RESOURCE_BARRIER[size];
			for (U16 i = 0; i < size; ++i)
			{
				auto& barrier = barriers[i];
				auto& copyInfo = copyResList[copyOffset + i];
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				barrier.Transition.StateAfter = copyInfo.FinalState;
				barrier.Transition.pResource = copyInfo.Destination;
			}
			copyOffset = copyResInfo.Size;
			copyList.GetList()->ResourceBarrier(size, barriers);
			delete[] barriers;

			copyList.Close(*this);
			Execute(mainQueue.Get(), copyList);
			copyList.Open(*this);
		}
	}

	void Device::EndUploadRegion()
	{
		ZE_ASSERT(copyResList != nullptr, "Start upload region first!");
		ZE_WIN_ENABLE_EXCEPT();

		copyList.Close(*this);
		U64 fenceVal = ++mainFenceVal;
		ZE_DX_THROW_FAILED(mainQueue->Signal(mainFence.Get(), fenceVal));
		WaitMain(fenceVal);
		copyList.Reset(*this);

		Table::Clear(copyResInfo.Size, copyResList);
		copyOffset = 0;
		copyResInfo.Size = 0;
		copyResInfo.Allocated = COPY_LIST_GROW_SIZE;
	}

	void Device::Execute(GFX::CommandList* cls, U32 count)
	{
		if (count == 1)
		{
			switch (cls->Get().dx12.GetList()->GetType())
			{
			default:
				ZE_FAIL("Incorrect type of command list!!!");
			case D3D12_COMMAND_LIST_TYPE_DIRECT:
				return ExecuteMain(*cls);
			case D3D12_COMMAND_LIST_TYPE_COMPUTE:
				return ExecuteCompute(*cls);
			case D3D12_COMMAND_LIST_TYPE_COPY:
				return ExecuteCopy(*cls);
			}
		}

		// Find max size for command lists to execute at once
		U32 mainCount = 0, computeCount = 0, copyCount = 0;
		for (U32 i = 0; i < count; ++i)
		{
			ZE_ASSERT(cls[i].Get().dx12.GetList() != nullptr, "Empty command list!");

			switch (cls[i].Get().dx12.GetList()->GetType())
			{
			case D3D12_COMMAND_LIST_TYPE_DIRECT:
			{
				++mainCount;
				break;
			}
			case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			{
				++computeCount;
				break;
			}
			case D3D12_COMMAND_LIST_TYPE_COPY:
			{
				++copyCount;
				break;
			}
			default:
				ZE_FAIL("Incorrect type of command list!!!");
			}
		}

		// Realloc if needed bigger list
		count = std::max(commandListsCount, mainCount);
		if (computeCount > count)
			count = computeCount;
		if (copyCount > count)
			count = copyCount;
		if (count > commandListsCount)
		{
			commandListsCount = count;
			commandLists = reinterpret_cast<ICommandList**>(realloc(commandLists, count * sizeof(ICommandList*)));
		}

		// Execute lists
		if (mainCount)
		{
			U32 i = 0, j = 0;
			do
			{
				if (cls[i].Get().dx12.GetList()->GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT)
					commandLists[i++] = cls[j].Get().dx12.GetList();
				++j;
			} while (i < mainCount);
			ZE_DX_THROW_FAILED_INFO(mainQueue->ExecuteCommandLists(mainCount, commandLists));
		}
		if (computeCount)
		{
			U32 i = 0, j = 0;
			do
			{
				if (cls[i].Get().dx12.GetList()->GetType() == D3D12_COMMAND_LIST_TYPE_COMPUTE)
					commandLists[i++] = cls[j].Get().dx12.GetList();
				++j;
			} while (i < computeCount);
			ZE_DX_THROW_FAILED_INFO(computeQueue->ExecuteCommandLists(computeCount, commandLists));
		}
		if (copyCount)
		{
			U32 i = 0, j = 0;
			do
			{
				if (cls[i].Get().dx12.GetList()->GetType() == D3D12_COMMAND_LIST_TYPE_COPY)
					commandLists[i++] = cls[j].Get().dx12.GetList();
				++j;
			} while (i < copyCount);
			ZE_DX_THROW_FAILED_INFO(copyQueue->ExecuteCommandLists(copyCount, commandLists));
		}
	}

	void Device::ExecuteMain(GFX::CommandList& cl)
	{
		Execute(mainQueue.Get(), cl.Get().dx12);
	}

	void Device::ExecuteCompute(GFX::CommandList& cl)
	{
		Execute(computeQueue.Get(), cl.Get().dx12);
	}

	void Device::ExecuteCopy(GFX::CommandList& cl)
	{
		Execute(copyQueue.Get(), cl.Get().dx12);
	}

	D3D12_RESOURCE_DESC1 Device::GetBufferDesc(U64 size) const noexcept
	{
		ZE_ASSERT(size, "Cannot create empty buffer!");

		D3D12_RESOURCE_DESC1 desc;
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
		desc.SamplerFeedbackMipRegion.Width = 0;
		desc.SamplerFeedbackMipRegion.Height = 0;
		desc.SamplerFeedbackMipRegion.Depth = 0;
		return desc;
	}

	std::pair<D3D12_RESOURCE_DESC1, U32> Device::GetTextureDesc(U32 width, U32 height, U16 count,
		DXGI_FORMAT format, GFX::Resource::Texture::Type type) const noexcept
	{
		ZE_ASSERT(width < D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION
			&& height < D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION,
			"Texture too big!");

		D3D12_RESOURCE_DESC1 desc;
		desc.Dimension = type == GFX::Resource::Texture::Type::Tex3D ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
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
		desc.SamplerFeedbackMipRegion.Width = 0;
		desc.SamplerFeedbackMipRegion.Height = 0;
		desc.SamplerFeedbackMipRegion.Depth = 0;

		D3D12_RESOURCE_ALLOCATION_INFO1 info;
		device->GetResourceAllocationInfo2(0, 1, &desc, &info);
		if (info.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
		{
			desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			device->GetResourceAllocationInfo2(0, 1, &desc, &info);
		}

		return { desc, static_cast<U32>(info.SizeInBytes) };
	}

	ResourceInfo Device::CreateBuffer(const D3D12_RESOURCE_DESC1& desc, bool dynamic)
	{
		if (dynamic)
			return allocator.AllocDynamicBuffer(*this, desc);
		return allocator.AllocBuffer(*this, desc);
	}

	ResourceInfo Device::CreateTexture(const std::pair<D3D12_RESOURCE_DESC1, U32>& desc)
	{
		if (desc.first.Alignment == D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
			return allocator.AllocTexture_64KB(*this, desc.second, desc.first);
		else if (desc.first.Alignment == D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
			return allocator.AllocTexture_4KB(*this, desc.second, desc.first);
		return allocator.AllocTexture_4MB(*this, desc.second, desc.first);
	}

	DX::ComPtr<IResource> Device::CreateTextureUploadBuffer(U64 size)
	{
		ZE_WIN_ENABLE_EXCEPT();

		auto desc = GetBufferDesc(size);
		D3D12_HEAP_PROPERTIES tempHeap;
		tempHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
		tempHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		tempHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		tempHeap.CreationNodeMask = 0;
		tempHeap.VisibleNodeMask = 0;

		DX::ComPtr<IResource> uploadRes;
		ZE_DX_THROW_FAILED(device->CreateCommittedResource2(&tempHeap,
			D3D12_HEAP_FLAG_CREATE_NOT_ZEROED, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, nullptr, IID_PPV_ARGS(&uploadRes)));
		return uploadRes;
	}

	void Device::UploadBuffer(IResource* dest, const D3D12_RESOURCE_DESC1& desc,
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

		DX::ComPtr<IResource> uploadRes;
		ZE_DX_THROW_FAILED(device->CreateCommittedResource2(&tempHeap,
			D3D12_HEAP_FLAG_CREATE_NOT_ZEROED, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, nullptr, IID_PPV_ARGS(&uploadRes)));

		// Map and copy data into upload buffer
		D3D12_RANGE range = { 0 };
		void* uploadBuffer = nullptr;
		ZE_DX_THROW_FAILED(uploadRes->Map(0, &range, &uploadBuffer));
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
			Table::Append<COPY_LIST_GROW_SIZE>(copyResInfo, copyResList, UploadInfo(finalState, reinterpret_cast<IResource*>(dest.pResource), reinterpret_cast<IResource*>(source.pResource)));
	}

	void Device::UpdateBuffer(IResource* res, const void* data, U64 size, D3D12_RESOURCE_STATES currentState)
	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = res;
		barrier.Transition.Subresource = 0;
		barrier.Transition.StateBefore = currentState;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

		copyList.GetList()->ResourceBarrier(1, &barrier);
		D3D12_RESOURCE_DESC1 desc = GetBufferDesc(size);
		UploadBuffer(res, desc, data, size, currentState);
	}

	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> Device::AddStaticDescs(U32 count) noexcept
	{
		ZE_ASSERT(dynamicDescCount == 0, "Cannot add dynamic descs before all static ones are created!");
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
		ZE_ASSERT(rangeStart.ID != 0,
			"Invalid location for normal descs! Resources created before RenderGraph finalization \
			should always be created with PackOption::StaticCreation!");

		U64 offest = static_cast<U64>(rangeStart.ID) * descriptorSize;
		rangeStart.GPU.ptr = descHeap->GetGPUDescriptorHandleForHeapStart().ptr + offest;
		rangeStart.CPU.ptr = descHeap->GetCPUDescriptorHandleForHeapStart().ptr + offest;
		dynamicDescCount += count;
		return rangeStart;
	}
}