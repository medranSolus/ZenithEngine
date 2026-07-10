#include "RHI/DX11/Device.h"
#include "GFX/CommandList.h"

namespace ZE::RHI::DX11
{
	void Device::Execute(GFX::CommandList& cl) const noexcept
	{
		if (cl.Get().dx11.GetList() != nullptr)
		{
			ZE_DX_CHECK_FAILED(context->ExecuteCommandList(cl.Get().dx11.GetList(), FALSE), "ExecuteCommandLists caused debug layer messages!");
		}
	}

	void Device::MoveFrom(Device&& dev) noexcept
	{
#if _ZE_DEBUG_GFX_API
		debugManager = std::move(dev.debugManager);
		DX::DebugInfoManager::Register(debugManager);
#endif
#if _ZE_GFX_MARKERS
		tagManager = std::move(dev.tagManager);
#endif
		device = std::move(dev.device);
		context = std::move(dev.context);

		std::swap(gpuCtx, dev.gpuCtx);
		displayProps = std::move(dev.displayProps);
	}

	Expected<Device> Device::Create(const Window::MainWindow& window, U32 descriptorCount) noexcept
	{
		DX::ComPtr<DX::IFactory> factory;
		ZE_EXPECT_RET_FAILED(factory, DX::CreateFactory());
		DX::ComPtr<DX::IAdapter> adapter = DX::CreateAdapter(factory);
		if (adapter == nullptr)
		{
			ZE_DX_RET_FAILED_EXPECT(DX::Error::NO_ADAPTER_ERROR);
		}

		Device dev;
#if _ZE_DEBUG_GFX_API
		ZE_EXPECT_RET_FAILED(dev.debugManager, DX::DebugInfoManager::Create());
		DX::DebugInfoManager::Register(dev.debugManager);
#endif

		constexpr D3D_FEATURE_LEVEL FEATURE_LEVEL = D3D_FEATURE_LEVEL_11_1;
		// Initialize via hardware specific functions
		switch (Settings::GpuVendor)
		{
		case GFX::VendorGPU::AMD:
		{
			AGSConfiguration agsConfig = {};
			if (agsInitialize(AGS_MAKE_VERSION(AMD_AGS_VERSION_MAJOR, AMD_AGS_VERSION_MINOR, AMD_AGS_VERSION_PATCH),
				&agsConfig, &dev.gpuCtx.AMD, nullptr) == AGS_SUCCESS)
			{
				AGSDX11DeviceCreationParams deviceParams = {};
				deviceParams.pAdapter = adapter.Get();
				deviceParams.DriverType = D3D_DRIVER_TYPE_UNKNOWN;
				deviceParams.Software = nullptr;
				deviceParams.Flags = _ZE_DEBUG_GFX_API ? D3D11_CREATE_DEVICE_DEBUG : 0;
				deviceParams.pFeatureLevels = &FEATURE_LEVEL;
				deviceParams.FeatureLevels = 1;
				deviceParams.SDKVersion = D3D11_SDK_VERSION;
				deviceParams.pSwapChainDesc = nullptr;

				AGSDX11ExtensionParams extensionParams = {};
				const std::wstring appName = Utils::ToUTF16(Settings::GetAppName());
				extensionParams.pAppName = appName.c_str();
				extensionParams.pEngineName = Settings::ENGINE_NAME_WIDE;
				extensionParams.appVersion = Settings::GetAppVersion();
				extensionParams.engineVersion = Settings::ENGINE_VERSION;
				extensionParams.numBreadcrumbMarkers = _ZE_DEBUG_GFX_API ? 128 : 0;
				extensionParams.uavSlot = 7;
				extensionParams.crossfireMode = AGS_CROSSFIRE_MODE_DRIVER_AFR;
				AGSDX11ReturnedParams returnParams = {};
				if (agsDriverExtensionsDX11_CreateDevice(dev.gpuCtx.AMD, &deviceParams, &extensionParams, &returnParams) == AGS_SUCCESS)
				{
					bool success = false;
					if (SUCCEEDED(returnParams.pDevice->QueryInterface(IID_PPV_ARGS(&dev.device))))
					{
						if (SUCCEEDED(returnParams.pImmediateContext->QueryInterface(IID_PPV_ARGS(&dev.context))))
							success = true;
						else
							dev.device = nullptr;
					}
					returnParams.pDevice->Release();
					returnParams.pImmediateContext->Release();
					if (success)
						break;
				}
				agsDeInitialize(dev.gpuCtx.AMD);
				dev.gpuCtx.AMD = nullptr;
			}
			Settings::GpuVendor = GFX::VendorGPU::Unknown;
			break;
		}
		default:
			break;
		}

		// Failed to create GPU specific device
		if (dev.device == nullptr)
		{
			DX::ComPtr<ID3D11Device> tempDevice = nullptr;
			ZE_DX_RET_FAILED_EXPECT(D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
				_ZE_DEBUG_GFX_API ? D3D11_CREATE_DEVICE_DEBUG : 0, &FEATURE_LEVEL, 1,
				D3D11_SDK_VERSION, &tempDevice, nullptr, nullptr));
			ZE_DX_RET_FAILED_EXPECT(tempDevice.As(&dev.device));

			DX::ComPtr<ID3D11DeviceContext3> tempCtx;
			dev.device->GetImmediateContext3(&tempCtx);
			ZE_DX_RET_FAILED_EXPECT(tempCtx.As(&dev.context));
		}

#if _ZE_DEBUG_GFX_API
		DX::ComPtr<IInfoQueue> infoQueue;
		ZE_DX_RET_FAILED_EXPECT(dev.device.As(&infoQueue));

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

		ZE_DX_RET_FAILED_EXPECT(infoQueue->PushStorageFilter(&filter));
#endif
#if _ZE_GFX_MARKERS
		ZE_DX_RET_FAILED_EXPECT(dev.context.As(&dev.tagManager));
#endif

		// No RT on DX11
		Settings::RayTracingTier = GFX::RayTracingTier::None;
		// No support for 8 bit indices on DirectX
		Settings::SetU8IndexBuffers(false);
		Settings::SetGfxSupportSSSR(false);

		return dev;
	}

	void Device::Execute(GFX::CommandList* cls, U32 count) const noexcept
	{
		for (U32 i = 0; i < count; ++i)
			Execute(cls[i]);
	}
}