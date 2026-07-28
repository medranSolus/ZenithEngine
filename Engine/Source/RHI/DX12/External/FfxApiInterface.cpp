#if _ZE_FFX_API_ENABLED
#	include "GFX/External/InterfaceStorage.h"
ZE_WARNING_PUSH
#	include "dx12/ffx_api_dx12.h"
ZE_WARNING_POP

namespace ZE::RHI::DX12::External
{
	ffxReturnCode_t FfxApiInterface::ResourceAllocCallback(U32 effectId, D3D12_RESOURCE_STATES initialState,
		const D3D12_HEAP_PROPERTIES* heapProps, const D3D12_RESOURCE_DESC* desc, const struct FfxApiResourceDescription* ffxDesc,
		const D3D12_CLEAR_VALUE* clearVal, ID3D12Resource** resource) noexcept
	{
		auto* ffx = GFX::External::InterfaceStorage::GetConnectionFfxApi();
		ZE_ASSERT(ffx, "While FFX API is active the interface must be present!");
		auto& d3dFfx = ffx->Get().dx12;
		ZE_ASSERT(d3dFfx.device, "Device pointer should always be present!");

		D3D12_RESOURCE_DESC1 resDesc = {};
		resDesc.Dimension = desc->Dimension;
		resDesc.Alignment = desc->Alignment;
		resDesc.Width = desc->Width;
		resDesc.Height = desc->Height;
		resDesc.DepthOrArraySize = desc->DepthOrArraySize;
		resDesc.MipLevels = desc->MipLevels;
		resDesc.Format = desc->Format;
		resDesc.SampleDesc = desc->SampleDesc;
		resDesc.Layout = desc->Layout;
		resDesc.Flags = desc->Flags;
		resDesc.SamplerFeedbackMipRegion.Width = 0;
		resDesc.SamplerFeedbackMipRegion.Height = 0;
		resDesc.SamplerFeedbackMipRegion.Depth = 0;

		if (heapProps->Type == D3D12_HEAP_TYPE_DEFAULT && ffxDesc->flags & FFX_RESOURCE_FLAGS_ALIASABLE && resDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			ResourceAllocation* allocator = d3dFfx.AcquireRegion(effectId);
			if (allocator)
			{
				D3D12_RESOURCE_ALLOCATION_INFO1 info = {};
				d3dFfx.device->GetDevice()->GetResourceAllocationInfo3(0, 1, &resDesc, nullptr, nullptr, &info);

				if (allocator->availableSize >= info.SizeInBytes)
				{
					HRESULT hr = d3dFfx.device->GetDevice()->CreatePlacedResource1(allocator->aliasableHeap.Get(), allocator->nextOffset, &resDesc, initialState, clearVal, IID_PPV_ARGS(resource));
					if (FAILED(hr))
					{
						ZE_CODE_ERROR(ZE_DX_ERROR(hr), "Failed to create FFX API placed resource, falling back to committed resource path!");
					}
					else
					{
						allocator->availableSize -= info.SizeInBytes;
						allocator->nextOffset += info.SizeInBytes;
						d3dFfx.aliasableResources.emplace(reinterpret_cast<U64>(*resource));
					}
				}
				else
				{
					ZE_WARNING("Trying to allocate resource larger than available memory!");
				}
			}
		}
		if (*resource == nullptr)
		{
			HRESULT hr = d3dFfx.device->GetDevice()->CreateCommittedResource2(heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, initialState, clearVal, nullptr, IID_PPV_ARGS(resource));
			if (FAILED(hr))
			{
				ZE_CODE_ERROR(ZE_DX_ERROR(hr), "Failed to create FFX API committed resource!");
				return FFX_API_RETURN_ERROR_RUNTIME_ERROR;
			}
		}
		return FFX_API_RETURN_OK;
	}

	ffxReturnCode_t FfxApiInterface::ResourceDeallocCallback(U32 effectId, ID3D12Resource* resource) noexcept
	{
		auto* ffx = GFX::External::InterfaceStorage::GetConnectionFfxApi();
		ZE_ASSERT(ffx, "While FFX API is active the interface must be present!");
		auto& d3dFfx = ffx->Get().dx12;
		ZE_ASSERT(d3dFfx.device, "Device pointer should always be present!");

		if (d3dFfx.aliasableResources.contains(reinterpret_cast<U64>(resource)))
		{
			d3dFfx.aliasableResources.erase(reinterpret_cast<U64>(resource));

			D3D12_RESOURCE_DESC desc = resource->GetDesc();
			D3D12_RESOURCE_ALLOCATION_INFO1 info = {};
			d3dFfx.device->GetDevice()->GetResourceAllocationInfo1(0, 1, &desc, &info);

			if (d3dFfx.RemoveAllocation(effectId, info.SizeInBytes))
				return FFX_API_RETURN_ERROR_PARAMETER;
		}

		resource->Release();
		return FFX_API_RETURN_OK;
	}

	ffxReturnCode_t FfxApiInterface::HeapAllocCallback(U32 effectId, const D3D12_HEAP_DESC* heapDesc, bool aliasable, ID3D12Heap** heap, U64* startOffset) noexcept
	{
		auto* ffx = GFX::External::InterfaceStorage::GetConnectionFfxApi();
		ZE_ASSERT(ffx, "While FFX API is active the interface must be present!");
		auto& d3dFfx = ffx->Get().dx12;
		ZE_ASSERT(d3dFfx.device, "Device pointer should always be present!");

		if (aliasable && heapDesc->Properties.Type == D3D12_HEAP_TYPE_DEFAULT)
		{
			ResourceAllocation* allocator = d3dFfx.AcquireRegion(effectId);
			if (allocator)
			{
				if (allocator->availableSize >= heapDesc->SizeInBytes)
				{
					allocator->availableSize -= heapDesc->SizeInBytes;
					*startOffset = allocator->nextOffset;
					allocator->nextOffset += heapDesc->SizeInBytes;
					*heap = allocator->aliasableHeap.Get();
					allocator->aliasableHeap->AddRef();
				}
				else
				{
					ZE_WARNING("Trying to allocate heap larger than available memory!");
				}
			}
		}
		if (*heap == nullptr)
		{
			*startOffset = 0;
			HRESULT hr = d3dFfx.device->GetDevice()->CreateHeap1(heapDesc, nullptr, IID_PPV_ARGS(heap));
			if (FAILED(hr))
			{
				ZE_CODE_ERROR(ZE_DX_ERROR(hr), "Failed to create FFX API heap!");
				return FFX_API_RETURN_ERROR_RUNTIME_ERROR;
			}
		}
		return FFX_API_RETURN_OK;
	}

	ffxReturnCode_t FfxApiInterface::HeapDeallocCallback(U32 effectId, ID3D12Heap* heap, U64 startOffset, U64 heapSize) noexcept
	{
		heap->Release();
		if (startOffset != 0)
		{
			auto* ffx = GFX::External::InterfaceStorage::GetConnectionFfxApi();
			ZE_ASSERT(ffx, "While FFX API is active the interface must be present!");

			if (ffx->Get().dx12.RemoveAllocation(effectId, heapSize))
				return FFX_API_RETURN_ERROR_PARAMETER;
		}
		return FFX_API_RETURN_OK;
	}

	FfxApiConstantBufferAllocation FfxApiInterface::CBufferAllocCallback(void* data, const U64 size) noexcept
	{
		auto* ffx = GFX::External::InterfaceStorage::GetConnectionFfxApi();
		ZE_ASSERT(ffx, "While FFX API is active the interface must be present!");
		auto& d3dFfx = ffx->Get().dx12;
		ZE_ASSERT(d3dFfx.device, "Device pointer should always be present!");
		ZE_ASSERT(d3dFfx.ringBuffer, "DynamicCBuffer pointer should always be present!");

		FfxApiConstantBufferAllocation alloc = {};
		auto exp = d3dFfx.ringBuffer->Alloc(*d3dFfx.device, data, Utils::SafeCast<U32>(size));
		if (exp)
		{
			alloc.resource.resource = d3dFfx.ringBuffer->GetAllocBuffer(*exp);
			alloc.resource.description.type = FFX_API_RESOURCE_TYPE_BUFFER;
			alloc.resource.description.format = FFX_API_SURFACE_FORMAT_UNKNOWN;
			alloc.resource.description.size = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			alloc.resource.description.stride = 0;
			alloc.resource.description.alignment = 0;
			alloc.resource.description.mipCount = 1;
			alloc.resource.description.flags = FFX_API_RESOURCE_FLAGS_NONE;
			alloc.resource.description.usage = FFX_API_RESOURCE_USAGE_READ_ONLY;
			alloc.resource.state = FFX_API_RESOURCE_STATE_GENERIC_READ;

			alloc.handle = d3dFfx.ringBuffer->GetBindHandle(*exp);
		}
		return alloc;
	}

	FfxApiInterface::ResourceAllocation* FfxApiInterface::AcquireRegion(U32 effectId) noexcept
	{
		auto allocator = effectAllocs.find(effectId);
		if (allocator != effectAllocs.end())
			return &allocator->second;

		// First time allocation, add new region to acquire
		if (newAliasableRegion.aliasableHeap)
			return &effectAllocs.emplace(effectId, std::move(newAliasableRegion)).first->second;

		ZE_FAIL("No region provided for creating allocations, falling back to default method!");
		return nullptr;
	}

	bool FfxApiInterface::RemoveAllocation(U32 effectId, U64 bytes) noexcept
	{
		auto allocator = effectAllocs.find(effectId);
		if (allocator == effectAllocs.end())
		{
			ZE_FAIL("When doing deallocation there always should be proper allocator present!");
			return true;
		}

		allocator->second.startOffset += bytes;
		// No more allocations present
		if (allocator->second.startOffset == allocator->second.nextOffset)
			effectAllocs.erase(allocator);
		return false;
	}

	void FfxApiInterface::Destroy() noexcept
	{
		if (ffxApiDll)
		{
			[[maybe_unused]] const BOOL res = FreeLibrary(ffxApiDll);
			ZE_ASSERT(res, "Error unloading amd_fidelityfx_loader_dx12.dll!");
		}
	}

	void FfxApiInterface::MoveFrom(FfxApiInterface&& ffxInt) noexcept
	{
		ffxApiDll = std::exchange(ffxInt.ffxApiDll, nullptr);
		ffxCreateContext = ffxInt.ffxCreateContext;
		ffxFunctions = std::move(ffxInt.ffxFunctions);
		device = std::move(ffxInt.device);
		ringBuffer = std::move(ffxInt.ringBuffer);
		aliasableResources = std::move(ffxInt.aliasableResources);
		effectAllocs = std::move(ffxInt.effectAllocs);
		newAliasableRegion = std::move(ffxInt.newAliasableRegion);
	}

	Expected<FfxApiInterface> FfxApiInterface::Create(GFX::Device& dev) noexcept
	{
		FfxApiInterface ffxInt;
		ffxInt.ffxApiDll = LoadLibraryW(L"amd_fidelityfx_loader_dx12.dll");
		if (!ffxInt.ffxApiDll)
		{
			Status code = ZE_WIN_LAST_ERROR();
			ZE_CODE_ERROR(code, "Error loading amd_fidelityfx_loader_dx12.dll!");
			return std::unexpected(code);
		}

		ffxInt.ffxCreateContext = (PfnFfxCreateContext)GetProcAddress(ffxInt.ffxApiDll, "ffxCreateContext");
		ffxInt.ffxFunctions.DestroyContext = (PfnFfxDestroyContext)GetProcAddress(ffxInt.ffxApiDll, "ffxDestroyContext");
		ffxInt.ffxFunctions.Configure = (PfnFfxConfigure)GetProcAddress(ffxInt.ffxApiDll, "ffxConfigure");
		ffxInt.ffxFunctions.Query = (PfnFfxQuery)GetProcAddress(ffxInt.ffxApiDll, "ffxQuery");
		ffxInt.ffxFunctions.Dispatch = (PfnFfxDispatch)GetProcAddress(ffxInt.ffxApiDll, "ffxDispatch");

		if (ffxInt.ffxCreateContext == nullptr || ffxInt.ffxFunctions.DestroyContext == nullptr
			|| ffxInt.ffxFunctions.Configure == nullptr || ffxInt.ffxFunctions.Query == nullptr || ffxInt.ffxFunctions.Dispatch == nullptr)
		{
			Status code = ZE_WIN_LAST_ERROR();
			ZE_CODE_ERROR(code, "Error loading functions from amd_fidelityfx_loader_dx12.dll!");
			return std::unexpected(code);
		}

		// Device should not move it's location during the lifetime
		ffxInt.device = &dev.Get().dx12;
		return ffxInt;
	}

	ffxReturnCode_t FfxApiInterface::CreateFfxCtx(GFX::Device& dev, GFX::Pipeline::FrameBuffer& buffers, ffxContext* ctx, ffxCreateContextDescHeader& ctxHeader, RID aliasableRegion) noexcept
	{
		if (ffxApiDll)
		{
			// In case it got moved
			device = &dev.Get().dx12;

			ffxCreateBackendDX12AllocationCallbacksDesc allocCallbacksDescs = { FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12_ALLOCATION_CALLBACKS, ctxHeader.pNext };
			allocCallbacksDescs.pfnFfxResourceAllocator = FfxApiInterface::ResourceAllocCallback;
			allocCallbacksDescs.pfnFfxResourceDeallocator = FfxApiInterface::ResourceDeallocCallback;
			allocCallbacksDescs.pfnFfxHeapAllocator = FfxApiInterface::HeapAllocCallback;
			allocCallbacksDescs.pfnFfxHeapDeallocator = FfxApiInterface::HeapDeallocCallback;
			allocCallbacksDescs.pfnFfxConstantBufferAllocator = FfxApiInterface::CBufferAllocCallback;

			ffxCreateBackendDX12Desc backendDesc = { FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12, &allocCallbacksDescs.header };
			backendDesc.device = device->GetDevice();

			if (aliasableRegion != INVALID_RID)
			{
				auto& framebuffer = buffers.Get().dx12;
				newAliasableRegion.aliasableHeap = framebuffer.GetUAVHeap();
				newAliasableRegion.nextOffset = newAliasableRegion.startOffset = framebuffer.GetHeapOffset(aliasableRegion, dev.Get().dx12.IsTightAlignment());
				UInt2 packedSize = framebuffer.GetDimmensions(aliasableRegion);
				newAliasableRegion.availableSize = packedSize.X;
				newAliasableRegion.availableSize |= static_cast<U64>(packedSize.Y) << 32;
			}

			ctxHeader.pNext = &backendDesc.header;
			ffxReturnCode_t ret = ffxCreateContext(ctx, &ctxHeader, nullptr);
			ctxHeader.pNext = allocCallbacksDescs.header.pNext;

			return ret;
		}
		return FFX_API_RETURN_ERROR;
	}
}
#endif