#include "GFX/API/DX11/Device.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11
{
	Device::Device()
	{
		ZE_WIN_ENABLE_EXCEPT();

		// Create proper DXGI factory
		DX::ComPtr<IDXGIFactory2> oldFactory = nullptr;
		ZE_GFX_THROW_FAILED(CreateDXGIFactory2(ZE_NO_DEBUG ? 0 : DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&oldFactory)));
		DX::ComPtr<IDXGIFactory7> factory = nullptr;
		ZE_GFX_THROW_FAILED(oldFactory->QueryInterface(IID_PPV_ARGS(&factory)));

		DX::ComPtr<IDXGIAdapter4> adapter = nullptr;
		for (UINT i = 0; true; ++i)
		{
			// Get highest possible performant GPU
			ZE_GFX_THROW_FAILED(factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)));
			// Check if GPU is attached to the screen
			DX::ComPtr<IDXGIOutput> ouptut = nullptr;
			if (SUCCEEDED(adapter->EnumOutputs(0, &ouptut)))
				break;
		}

		DX::ComPtr<ID3D11Device> tempDevice = nullptr;
		D3D_FEATURE_LEVEL features[]
		{
			D3D_FEATURE_LEVEL_11_1
		};
		ZE_GFX_THROW_FAILED(D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
			ZE_NO_DEBUG ? 0 : D3D11_CREATE_DEVICE_DEBUG, features, 1,
			D3D11_SDK_VERSION, &tempDevice, nullptr, nullptr));

		ZE_GFX_THROW_FAILED(tempDevice->QueryInterface(IID_PPV_ARGS(&device)));
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