#include "GFX/API/DX11/Device.h"
#include "GFX/API/DX/GraphicsException.h"
#include "GFX/CommandList.h"

namespace ZE::GFX::API::DX11
{
	void Device::Execute(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG)
	{
		if (cl.Get().dx11.GetList() != nullptr)
		{
			ZE_GFX_THROW_FAILED_INFO(context->ExecuteCommandList(cl.Get().dx11.GetList(), FALSE));
		}
	}

	Device::Device()
	{
		ZE_WIN_ENABLE_EXCEPT();

		DX::ComPtr<IDXGIAdapter4> adapter = DX::CreateAdapter(
#ifdef _ZE_MODE_DEBUG
			debugManager
#endif
		);
		DX::ComPtr<ID3D11Device> tempDevice = nullptr;
		D3D_FEATURE_LEVEL features[]
		{
			D3D_FEATURE_LEVEL_11_1
		};
		ZE_GFX_THROW_FAILED(D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
			ZE_NO_DEBUG ? 0 : D3D11_CREATE_DEVICE_DEBUG, features, 1,
			D3D11_SDK_VERSION, &tempDevice, nullptr, nullptr));
		ZE_GFX_THROW_FAILED(tempDevice->QueryInterface(IID_PPV_ARGS(&device)));

#ifdef _ZE_MODE_DEBUG
		DX::ComPtr<ID3D11InfoQueue> infoQueue;
		ZE_GFX_THROW_FAILED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)));

		// Set breaks on dangerous messages
		infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);

		// Suppress non important messages
		D3D11_MESSAGE_SEVERITY severities[] = { D3D11_MESSAGE_SEVERITY_INFO };
		D3D11_INFO_QUEUE_FILTER filter = { 0 };
		filter.DenyList.NumSeverities = 1;
		filter.DenyList.pSeverityList = severities;

		ZE_GFX_THROW_FAILED(infoQueue->PushStorageFilter(&filter));
#endif
		DX::ComPtr<ID3D11DeviceContext3> tempCtx;
		device->GetImmediateContext3(&tempCtx);
		ZE_GFX_THROW_FAILED(tempCtx->QueryInterface(IID_PPV_ARGS(&context)));
	}

	Device::~Device()
	{
#ifdef _ZE_MODE_DEBUG
		DX::ComPtr<ID3D11Debug> debug;
		device->QueryInterface(IID_PPV_ARGS(&debug));
		if (debug != nullptr)
			debug->ReportLiveDeviceObjects(D3D11_RLDO_IGNORE_INTERNAL);
#endif
	}
}