#include "GFX/Graphics.h"
#include "Exception/GfxExceptionMacros.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

namespace ZE::GFX
{
	Graphics::~Graphics()
	{
		if (fullscreen)
			swapChain->SetFullscreenState(FALSE, nullptr);
		ImGui_ImplDX11_Shutdown();
#ifdef _ZE_MODE_DEBUG
		Microsoft::WRL::ComPtr<ID3D11Debug> debug;
		device->QueryInterface(IID_PPV_ARGS(&debug));
		if (debug != nullptr)
			debug->ReportLiveDeviceObjects(D3D11_RLDO_IGNORE_INTERNAL);
#endif
	}

	void Graphics::Init(HWND hWnd, U32 width, U32 height)
	{
		ZE_GFX_ENABLE_NOINFO();
		DXGI_SWAP_CHAIN_DESC swapDesc = { 0 };
		swapDesc.OutputWindow = hWnd;
		swapDesc.BufferDesc.Width = width; // Use window size
		swapDesc.BufferDesc.Height = height;
		swapDesc.BufferDesc.Format = BACKBUFFER_FORMAT;
		swapDesc.BufferDesc.RefreshRate.Numerator = 0; // Refresh in fullscreen
		swapDesc.BufferDesc.RefreshRate.Denominator = 0;
		swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // Entire window drawing so no scaling needed
		swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapDesc.SampleDesc.Count = 1; // AntiAliasing
		swapDesc.SampleDesc.Quality = 0;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // http://aka.ms/dxgiflipmodel.
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Pipeline draws to this buffer
		swapDesc.Windowed = TRUE;
		swapDesc.BufferCount = 2; // Triple buffering
		swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		// This needed and resize buffers for fullscreen:
		// https://forums.codeguru.com/showthread.php?500867-RESOLVED-How-do-I-remove-the-window-border-to-get-fullscreen

		UINT createFlags = 0;
#ifdef _ZE_MODE_DEBUG
		createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL features[]
		{
			D3D_FEATURE_LEVEL_11_1
		};
		ZE_GFX_THROW_FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
			createFlags, nullptr, 0, D3D11_SDK_VERSION, &swapDesc, &swapChain, &device, features, &context));

		Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer = nullptr;
		ZE_GFX_THROW_FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))); // Get texture subresource (back buffer)
		renderTarget = GfxResPtr<Pipeline::Resource::RenderTarget>(*this, width, height, backBuffer, BACKBUFFER_FORMAT); // Create view to back buffer allowing writing data
#ifdef _ZE_MODE_DEBUG
		ZE_GFX_THROW_FAILED(context->QueryInterface(IID_PPV_ARGS(&tagManager)));
#endif
		ImGui_ImplDX11_Init(device.Get(), context.Get());

		// Don't use OS ALT+ENTER handling
		// First, retrieve the underlying DXGI Device from the D3D Device
		Microsoft::WRL::ComPtr<IDXGIDevice1> dxgiDevice;
		ZE_GFX_THROW_FAILED(device.As(&dxgiDevice));
		// Identify the physical adapter (GPU or card) this device is running on.
		Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
		ZE_GFX_THROW_FAILED(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));
		// And obtain the factory object that created it.
		Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
		ZE_GFX_THROW_FAILED(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));
		// Disable OS handling
		ZE_GFX_THROW_FAILED(dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
	}

	void Graphics::DrawIndexed(U32 count) noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_THROW_FAILED_INFO(context->DrawIndexed(count, 0, 0));
	}

	void Graphics::ComputeFrame(U32 threadsX, U32 threadsY) noexcept
	{
		assert(threadsX != 0 && threadsY != 0);
		context->Dispatch(GetWidth() / threadsX + static_cast<bool>(GetWidth() % threadsX),
			GetHeight() / threadsY + static_cast<bool>(GetHeight() % threadsY), 1);
	}

	void Graphics::EndFrame()
	{
		if (guiEnabled)
		{
			ImGui::Render(); // Create data to render
#ifdef _ZE_MODE_DEBUG
			PushDrawTag("ImGui");
#endif
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Pass it to DirectX
#ifdef _ZE_MODE_DEBUG
			PopDrawTag();
#endif
		}
		HRESULT result;
		ZE_GFX_SET_DEBUG_WATCH();
		if (FAILED(result = swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING)))
		{
			if (result == DXGI_ERROR_DEVICE_REMOVED)
				throw ZE_GFX_DEV_REMOVED_EXCEPT(device->GetDeviceRemovedReason());
			else
				throw ZE_GFX_EXCEPT(result);
		}
	}

	void Graphics::BeginFrame() noexcept
	{
		if (guiEnabled)
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}
	}

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