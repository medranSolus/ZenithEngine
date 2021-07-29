#include "GFX/API/DX12/SwapChain.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12
{
	SwapChain::SwapChain(const Window::WinAPI::WindowWinAPI& window, GFX::Device& dev)
	{
		ZE_GFX_ENABLE(dev.Get().dx12);

		// Retrieve factory used to create device
		DX::ComPtr<IDXGIDevice4> dxgiDevice = nullptr;
		ZE_GFX_THROW_FAILED(dev.Get().dx12.GetDevice()->QueryInterface(IID_PPV_ARGS(&dxgiDevice)));

		presentFlags = DX::CreateSwapChain(std::move(dxgiDevice), dev.Get().dx12.GetMainQueue(), window.GetHandle(), swapChain
#ifdef _ZE_MODE_DEBUG
			, dev.Get().dx12.GetInfoManager()
#endif
		);
	}

	void SwapChain::Present(GFX::Device& dev) const
	{
		ZE_GFX_ENABLE(dev.Get().dx12);
		ZE_GFX_SET_DEBUG_WATCH();
		if (FAILED(ZE_WIN_EXCEPT_RESULT = swapChain->Present(0, presentFlags)))
		{
			if (ZE_WIN_EXCEPT_RESULT == DXGI_ERROR_DEVICE_REMOVED)
				throw ZE_GFX_EXCEPT(dev.Get().dx12.GetDevice()->GetDeviceRemovedReason());
			else
				throw ZE_GFX_EXCEPT(ZE_WIN_EXCEPT_RESULT);
		}
	}
}