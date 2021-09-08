#include "GFX/API/DX12/Device.h"
#include "GFX/API/DX/GraphicsException.h"
#include "GFX/CommandList.h"

namespace ZE::GFX::API::DX12
{
	void Device::Wait(ID3D12Fence1* fence, U64 val)
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

	void Device::Execute(ID3D12CommandQueue* queue, GFX::CommandList& cl) noexcept(ZE_NO_DEBUG)
	{
		assert(cl.Get().dx12.GetList() != nullptr);
		ID3D12CommandList* lists[] = { cl.Get().dx12.GetList() };
		ZE_GFX_THROW_FAILED_INFO(queue->ExecuteCommandLists(1, lists));
	}

	Device::Device()
	{
		ZE_WIN_ENABLE_EXCEPT();

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
		ZE_GFX_THROW_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mainFence)));

		desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		ZE_GFX_THROW_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&computeQueue)));
		ZE_GFX_THROW_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&computeFence)));

		desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		ZE_GFX_THROW_FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&copyQueue)));
		ZE_GFX_THROW_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&copyFence)));

		D3D12_FEATURE_DATA_D3D12_OPTIONS options = { 0 };
		ZE_WIN_THROW_FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)));
		if (options.ResourceHeapTier == D3D12_RESOURCE_HEAP_TIER_1)
		{
			new(&allocator.Tier1) AllocatorTier1(*this, ALLOC_INIT_FREE_LIST_SIZE);
			allocTier = AllocTier::Tier1;
		}
		else
		{
			new(&allocator.Tier2) AllocatorTier2(*this, ALLOC_INIT_FREE_LIST_SIZE);
			allocTier = AllocTier::Tier2;
		}
	}

	Device::~Device()
	{
		if (allocTier == AllocTier::Tier1)
			allocator.Tier1.~AllocatorTier1();
		else
			allocator.Tier2.~AllocatorTier2();

#ifdef _ZE_MODE_DEBUG
		DX::ComPtr<ID3D12DebugDevice> debug;
		device->QueryInterface(IID_PPV_ARGS(&debug));
		if (debug != nullptr)
			debug->ReportLiveDeviceObjects(D3D12_RLDO_IGNORE_INTERNAL);
#endif
	}

	ResourceInfo Device::CreateBuffer(U32 size)
	{
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

		if (allocTier == AllocTier::Tier1)
			return allocator.Tier1.AllocBuffer(*this, size, desc);
		else
			return allocator.Tier2.Alloc_64KB(*this, size, desc);
	}

	ResourceInfo Device::CreateTexture(U32 width, U32 height, DXGI_FORMAT format)
	{
		assert(width < D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION&& height < D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION);

		D3D12_RESOURCE_DESC desc;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Width = width;
		desc.Height = height;
		desc.DepthOrArraySize = 1;
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
			if (allocTier == AllocTier::Tier1)
				return allocator.Tier1.AllocTexture_64KB(*this, static_cast<U32>(info.SizeInBytes), desc);
			else
				return allocator.Tier2.Alloc_64KB(*this, static_cast<U32>(info.SizeInBytes), desc);
		}
		else if (allocTier == AllocTier::Tier1)
			return allocator.Tier1.AllocTexture_4KB(*this, static_cast<U32>(info.SizeInBytes), desc);
		else
			return allocator.Tier2.Alloc_4KB(*this, static_cast<U32>(info.SizeInBytes), desc);
	}

	void Device::FreeBuffer(const ResourceInfo& info) noexcept
	{
		D3D12_RESOURCE_DESC desc = info.Resource->GetDesc();
		U32 size = static_cast<U32>(device->GetResourceAllocationInfo(0, 1, &desc).SizeInBytes);
		if (allocTier == AllocTier::Tier1)
			allocator.Tier1.RemoveBuffer(info.ID, size);
		else
			allocator.Tier2.Remove(info.ID, size);
	}

	void Device::FreeTexture(const ResourceInfo& info) noexcept
	{
		D3D12_RESOURCE_DESC desc = info.Resource->GetDesc();
		U32 size = static_cast<U32>(device->GetResourceAllocationInfo(0, 1, &desc).SizeInBytes);
		if (allocTier == AllocTier::Tier1)
			allocator.Tier1.RemoveTexture(info.ID, size);
		else
			allocator.Tier2.Remove(info.ID, size);
	}
}