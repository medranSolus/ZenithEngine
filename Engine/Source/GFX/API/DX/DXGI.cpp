#include "GFX/API/DX/DXGI.h"
#include "GFX/API/DX/GraphicsException.h"
#include "Settings.h"

namespace ZE::GFX::API::DX
{
	ComPtr<IDXGIFactory7> CreateFactory(
#ifdef _ZE_MODE_DEBUG
		DebugInfoManager& debugManager
#endif
	)
	{
		ZE_WIN_ENABLE_EXCEPT();

		// Create proper DXGI factory
		ComPtr<IDXGIFactory2> oldFactory = nullptr;
		ZE_GFX_THROW_FAILED(CreateDXGIFactory2(ZE_NO_DEBUG ? 0 : DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&oldFactory)));
		ComPtr<IDXGIFactory7> factory = nullptr;
		ZE_GFX_THROW_FAILED(oldFactory->QueryInterface(IID_PPV_ARGS(&factory)));

		return factory;
	}

	ComPtr<IDXGIAdapter4> CreateAdapter(
#ifdef _ZE_MODE_DEBUG
		DebugInfoManager& debugManager
#endif
	)
	{
		ZE_WIN_ENABLE_EXCEPT();

		ComPtr<IDXGIFactory7> factory = CreateFactory(
#ifdef _ZE_MODE_DEBUG
			debugManager
#endif
		);
		ComPtr<IDXGIAdapter4> adapter = nullptr;
		for (UINT i = 0; true; ++i)
		{
			// Get highest possible performant GPU
			ZE_GFX_THROW_FAILED(factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)));
			// Check if GPU is attached to the screen
			ComPtr<IDXGIOutput> ouptut = nullptr;
			if (SUCCEEDED(adapter->EnumOutputs(0, &ouptut)))
				break;
		}
		return adapter;
	}

	UINT CreateSwapChain(ComPtr<IDXGIFactory7> factory, IUnknown* device, HWND window, ComPtr<IDXGISwapChain4>& swapChain
#ifdef _ZE_MODE_DEBUG
		, DebugInfoManager& debugManager
#endif
	)
	{
		ZE_WIN_ENABLE_EXCEPT();

		DXGI_SWAP_CHAIN_DESC1 swapDesc;
		swapDesc.Width = 0; // Use window sizes
		swapDesc.Height = 0;
		swapDesc.Format = DX::GetDXFormat(Settings::GetBackbufferFormat());
		swapDesc.Stereo = FALSE;
		swapDesc.SampleDesc.Count = 1; // Used only in bitblt swap model
		swapDesc.SampleDesc.Quality = 0;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Pipeline draws to this buffer
		swapDesc.BufferCount = Settings::GetBackbufferCount(); // [2;16]
		swapDesc.Scaling = DXGI_SCALING_STRETCH;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //http://aka.ms/dxgiflipmodel
		swapDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapDesc.Flags = 0;

		// Check support for tearing (vsync-off), required in variable rate displays
		BOOL allowTearing = FALSE;
		UINT presentFlags = 0;
		ZE_GFX_THROW_FAILED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
			&allowTearing, sizeof(allowTearing)));
		if (allowTearing == TRUE)
		{
			swapDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			presentFlags = DXGI_PRESENT_ALLOW_TEARING;
		}

		DX::ComPtr<IDXGISwapChain1> tempChain = nullptr;
		ZE_GFX_THROW_FAILED(factory->CreateSwapChainForHwnd(device,
			window, &swapDesc, nullptr, nullptr, &tempChain));
		ZE_GFX_THROW_FAILED(tempChain->QueryInterface(IID_PPV_ARGS(&swapChain)));

		// Don't use Alt+Enter Windows handling, only borderless fulsscreen window
		ZE_GFX_THROW_FAILED(factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER));
		return presentFlags;
	}
}