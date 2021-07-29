#include "GFX/API/DX12/Device.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12
{
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
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Suppress non important messages
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_MESSAGE_ID denyIds[2] =
		{
			// D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
			// Bug in Visual Studio Graphics Debugger while capturing frame
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
		};

		D3D12_INFO_QUEUE_FILTER filter;
		filter.DenyList.NumSeverities = 1;
		filter.DenyList.pSeverityList = severities;
		filter.DenyList.NumIDs = 2;
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
	}

	Device::~Device()
	{
#ifdef _ZE_MODE_DEBUG
		DX::ComPtr<ID3D12DebugDevice> debug;
		device->QueryInterface(IID_PPV_ARGS(&debug));
		if (debug != nullptr)
			debug->ReportLiveDeviceObjects(D3D12_RLDO_IGNORE_INTERNAL);
#endif
	}
}