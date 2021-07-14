#include "GFX/API/DX11/SwapChain.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11
{
	SwapChain::SwapChain(const Window::WinAPI::WindowWinAPI& window, GFX::Device& dev)
	{
		ZE_GFX_ENABLE(dev.Get().dx11);

		// Retrieve factory used to create device
		DX::ComPtr<IDXGIDevice4> dxgiDevice = nullptr;
		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->QueryInterface(IID_PPV_ARGS(&dxgiDevice)));
		DX::ComPtr<IDXGIAdapter> adapter = nullptr;
		ZE_GFX_THROW_FAILED(dxgiDevice->GetAdapter(&adapter));
		DX::ComPtr<IDXGIFactory7> factory = nullptr;
		ZE_GFX_THROW_FAILED(adapter->GetParent(IID_PPV_ARGS(&factory)));

		DXGI_SWAP_CHAIN_DESC1 swapDesc;
		swapDesc.Width = 0; // Use window sizes
		swapDesc.Height = 0;
		swapDesc.Format = DX::GetDXFormat(PixelFormat::R8G8B8A8_UNorm);
		swapDesc.Stereo = FALSE;
		swapDesc.SampleDesc.Count = 1; // Used only in bitblt swap model
		swapDesc.SampleDesc.Quality = 0;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Pipeline draws to this buffer
		swapDesc.BufferCount = 2; // [2;16]
		swapDesc.Scaling = DXGI_SCALING_STRETCH;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //http://aka.ms/dxgiflipmodel
		swapDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapDesc.Flags = 0;

		// Check support for tearing (vsync-off), required in variable rate displays
		BOOL allowTearing = FALSE;
		ZE_GFX_THROW_FAILED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
			&allowTearing, sizeof(allowTearing)));
		if (allowTearing == TRUE)
		{
			swapDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			presentFlags = DXGI_PRESENT_ALLOW_TEARING;
		}

		DX::ComPtr<IDXGISwapChain1> tempChain = nullptr;
		ZE_GFX_THROW_FAILED(factory->CreateSwapChainForHwnd(dev.Get().dx11.GetDevice(),
			window.GetHandle(), &swapDesc, nullptr, nullptr, &tempChain));
		ZE_GFX_THROW_FAILED(tempChain->QueryInterface(IID_PPV_ARGS(&swapChain)));

		// Don't use Alt+Enter Windows handling, only borderless fulsscreen window
		ZE_GFX_THROW_FAILED(factory->MakeWindowAssociation(window.GetHandle(), DXGI_MWA_NO_ALT_ENTER));
	}

	void SwapChain::Present(GFX::Device& dev) const
	{
		ZE_GFX_ENABLE(dev.Get().dx11);
		ZE_GFX_SET_DEBUG_WATCH();
		if (FAILED(ZE_WIN_EXCEPT_RESULT = swapChain->Present(0, presentFlags)))
		{
			if (ZE_WIN_EXCEPT_RESULT == DXGI_ERROR_DEVICE_REMOVED)
				throw ZE_GFX_EXCEPT(dev.Get().dx11.GetDevice()->GetDeviceRemovedReason());
			else
				throw ZE_GFX_EXCEPT(ZE_WIN_EXCEPT_RESULT);
		}
	}
}