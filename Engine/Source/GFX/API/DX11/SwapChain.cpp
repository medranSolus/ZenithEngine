#include "GFX/API/DX11/SwapChain.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11
{
	SwapChain::SwapChain(const Window::WinAPI::WindowWinAPI& window, GFX::Device& dev, bool shaderInput)
	{
		ZE_GFX_ENABLE(dev.Get().dx11);

		// Retrieve factory used to create device
		DX::ComPtr<IDXGIDevice4> dxgiDevice = nullptr;
		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDev().As(&dxgiDevice));
		DX::ComPtr<IDXGIAdapter> adapter = nullptr;
		ZE_GFX_THROW_FAILED(dxgiDevice->GetAdapter(&adapter));
		DX::ComPtr<IDXGIFactory7> factory = nullptr;
		ZE_GFX_THROW_FAILED(adapter->GetParent(IID_PPV_ARGS(&factory)));

		presentFlags = DX::CreateSwapChain(std::move(factory), dev.Get().dx11.GetDevice(), window.GetHandle(), swapChain, shaderInput
#ifdef _ZE_MODE_DEBUG
			, dev.Get().dx11.GetInfoManager()
#endif
		);

		// Retrieve RTV
		DX::ComPtr<ID3D11Resource> backBuffer = nullptr;
		ZE_GFX_THROW_FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateRenderTargetView1(backBuffer.Get(), nullptr, &rtv));
		if (shaderInput)
		{
			ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateShaderResourceView1(backBuffer.Get(), nullptr, &srv));
		}
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