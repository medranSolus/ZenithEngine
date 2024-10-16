#include "RHI/DX11/Device.h"
#include "GFX/CommandList.h"

namespace ZE::RHI::DX11
{
	void Device::Execute(GFX::CommandList& cl)
	{
		if (cl.Get().dx11.GetList() != nullptr)
		{
			ZE_DX_THROW_FAILED_INFO(context->ExecuteCommandList(cl.Get().dx11.GetList(), FALSE));
		}
	}

	Device::Device(const Window::MainWindow& window, U32 descriptorCount)
		: descriptorCount(descriptorCount)
	{
		ZE_WIN_ENABLE_EXCEPT();
		// No support for 8 bit indices on DirectX
		Settings::SetU8IndexBuffers(false);
		// No RT on DX11
		Settings::RayTracingTier = GFX::RayTracingTier::None;

		DX::ComPtr<DX::IAdapter> adapter = DX::CreateAdapter(
#if _ZE_DEBUG_GFX_API
			debugManager
#endif
		);
		DX::ComPtr<ID3D11Device> tempDevice = nullptr;
		D3D_FEATURE_LEVEL features[]
		{
			D3D_FEATURE_LEVEL_11_1
		};
		ZE_DX_THROW_FAILED(D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
			_ZE_DEBUG_GFX_API ? D3D11_CREATE_DEVICE_DEBUG : 0, features, 1,
			D3D11_SDK_VERSION, &tempDevice, nullptr, nullptr));
		ZE_DX_THROW_FAILED(tempDevice.As(&device));

#if _ZE_DEBUG_GFX_API
		DX::ComPtr<IInfoQueue> infoQueue;
		ZE_DX_THROW_FAILED(device.As(&infoQueue));

		// Set breaks on dangerous messages
		infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);

		// Suppress non important messages
		D3D11_MESSAGE_SEVERITY severities[] = { D3D11_MESSAGE_SEVERITY_INFO };
		// Ignore bug when setting object names
		D3D11_MESSAGE_ID hide[] = { D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS };

		D3D11_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumSeverities = 1;
		filter.DenyList.pSeverityList = severities;
		filter.DenyList.NumIDs = 1;
		filter.DenyList.pIDList = hide;

		ZE_DX_THROW_FAILED(infoQueue->PushStorageFilter(&filter));
#endif
		DX::ComPtr<ID3D11DeviceContext3> tempCtx;
		device->GetImmediateContext3(&tempCtx);
		ZE_DX_THROW_FAILED(tempCtx.As(&context));
#if _ZE_GFX_MARKERS
		ZE_DX_THROW_FAILED(context.As(&tagManager));
#endif
	}

	void Device::Execute(GFX::CommandList* cls, U32 count)
	{
		for (U32 i = 0; i < count; ++i)
			Execute(cls[i]);
	}
}