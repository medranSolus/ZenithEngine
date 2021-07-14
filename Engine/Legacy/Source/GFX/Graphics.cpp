#include "GFX/Graphics.h"
#include "Exception/GfxExceptionMacros.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

namespace ZE::GFX
{
	void Graphics::Resize(U32 width, U32 height)
	{
		if (renderTarget != nullptr)
		{
			ZE_GFX_ENABLE_NOINFO();
			// Delete backbuffer
			renderTarget->Release();

			ZE_GFX_THROW_FAILED(swapChain->ResizeBuffers(0, width, height,
				BACKBUFFER_FORMAT, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

			// Recreate backbuffer
			Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer = nullptr;
			ZE_GFX_THROW_FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
			(*renderTarget) = { *this, width, height, backBuffer, BACKBUFFER_FORMAT };
		}
	}

	void Graphics::SetFullscreen()
	{
		fullscreen = true;
		swapChain->SetFullscreenState(TRUE, nullptr);
		Resize(GetWidth(), GetHeight());
	}

	void Graphics::SetWindowed()
	{
		fullscreen = false;
		swapChain->SetFullscreenState(FALSE, nullptr);
		Resize(GetWidth(), GetHeight());
	}
}