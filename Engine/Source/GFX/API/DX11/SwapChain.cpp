#include "GFX/API/DX11/SwapChain.h"

namespace ZE::GFX::API::DX11
{
	SwapChain::SwapChain(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput)
	{
		ZE_DX_ENABLE(dev.Get().dx11);

		// Retrieve factory used to create device
		DX::ComPtr<IDXGIDevice4> dxgiDevice = nullptr;
		ZE_DX_THROW_FAILED(dev.Get().dx11.GetDev().As(&dxgiDevice));
		DX::ComPtr<IDXGIAdapter> adapter = nullptr;
		ZE_DX_THROW_FAILED(dxgiDevice->GetAdapter(&adapter));
		DX::ComPtr<IDXGIFactory7> factory = nullptr;
		ZE_DX_THROW_FAILED(adapter->GetParent(IID_PPV_ARGS(&factory)));

		presentFlags = DX::CreateSwapChain(std::move(factory), dev.Get().dx11.GetDevice(), window.GetHandle(), swapChain, shaderInput
#if _ZE_DEBUG_GFX_API
			, dev.Get().dx11.GetInfoManager()
#endif
		);

		// Retrieve RTV
		ZE_DX_THROW_FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
		ZE_DX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateRenderTargetView1(backBuffer.Get(), nullptr, &rtv));
		if (shaderInput)
		{
			ZE_DX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateShaderResourceView1(backBuffer.Get(), nullptr, &srv));
		}
	}

	void SwapChain::Present(GFX::Device& dev) const
	{
		ZE_DX_ENABLE(dev.Get().dx11);
		ZE_DX_SET_DEBUG_WATCH();
		if (FAILED(ZE_WIN_EXCEPT_RESULT = swapChain->Present(0, presentFlags)))
		{
			if (ZE_WIN_EXCEPT_RESULT == DXGI_ERROR_DEVICE_REMOVED)
				throw ZE_DX_EXCEPT(dev.Get().dx11.GetDevice()->GetDeviceRemovedReason());
			else
				throw ZE_DX_EXCEPT(ZE_WIN_EXCEPT_RESULT);
		}
	}
}