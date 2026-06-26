#include "RHI/DX11/SwapChain.h"

namespace ZE::RHI::DX11
{
	Expected<SwapChain> SwapChain::Create(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput) noexcept
	{
		// Retrieve factory used to create device
		DX::ComPtr<DX::IDevice> dxgiDevice;
		ZE_DX_RET_FAILED_EXPECT(dev.Get().dx11.GetDev().As(&dxgiDevice));
		DX::ComPtr<IDXGIAdapter> adapter;
		ZE_DX_RET_FAILED_EXPECT(dxgiDevice->GetAdapter(&adapter));
		DX::ComPtr<DX::IFactory> factory;
		ZE_DX_RET_FAILED_EXPECT(adapter->GetParent(IID_PPV_ARGS(&factory)));

		SwapChain swapChain;
		ZE_EXPECT_RET_FAILED(swapChain.swapChain, DX::CreateSwapChain(std::move(factory), dev.Get().dx12.GetQueueMain(), window.GetHandle(), shaderInput, swapChain.presentFlags));

		// Retrieve RTV
		ZE_DX_RET_FAILED_EXPECT(swapChain.swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChain.backBuffer)));
		ZE_DX_RET_FAILED_EXPECT(dev.Get().dx11.GetDevice()->CreateRenderTargetView1(swapChain.backBuffer.Get(), nullptr, &swapChain.rtv));
		if (shaderInput)
		{
			ZE_DX_RET_FAILED_EXPECT(dev.Get().dx11.GetDevice()->CreateShaderResourceView1(swapChain.backBuffer.Get(), nullptr, &swapChain.srv));
		}
		return swapChain;
	}

	Status SwapChain::Present(GFX::Device& dev) const noexcept
	{
		HRESULT hr = swapChain->Present(0, presentFlags);
		if (FAILED(hr))
		{
			if (hr == DXGI_ERROR_DEVICE_REMOVED)
				hr = dev.Get().dx11.GetDevice()->GetDeviceRemovedReason();
			return ZE_DX_ERROR(hr);
		}
		return {};
	}
}