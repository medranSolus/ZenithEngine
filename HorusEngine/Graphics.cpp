#include "GfxExceptionMacros.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include <d3dcompiler.h>

namespace MsWrl = Microsoft::WRL;

namespace GFX
{
#pragma region Exceptions
#ifdef _DEBUG
	const char * Graphics::DebugException::what() const noexcept
	{
		std::ostringstream stream;
		stream << this->BasicException::what() << GetDebugInfo();
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}

	std::string Graphics::DebugException::GetDebugInfo() const noexcept
	{
		std::ostringstream stream;
		size_t size = debugInfo.size();
		if (size > 0)
		{
			stream << "\n\n[Error Info]";
			for (size_t i = 0; i < size; ++i)
				stream << "\n\n[" << i + 1 << "] " << debugInfo.at(i);
			stream << std::endl;
		}
		return stream.str();
	}
#endif

	const char * Graphics::GraphicsException::what() const noexcept
	{
		std::ostringstream stream;
		stream << this->WinApiException::what();
#ifdef _DEBUG
		stream << GetDebugInfo();
#endif
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
#pragma endregion

	Graphics::Graphics(HWND hWnd, unsigned int width, unsigned int height)
	{
		GFX_ENABLE_EXCEPT();
		DXGI_SWAP_CHAIN_DESC swapDesc = { 0 };
		swapDesc.OutputWindow = hWnd;
		swapDesc.BufferDesc.Width = 0; // Use window size
		swapDesc.BufferDesc.Height = 0;
		swapDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM; // RGB
		swapDesc.BufferDesc.RefreshRate.Numerator = 0; // Refresh in fullscreen
		swapDesc.BufferDesc.RefreshRate.Denominator = 0;
		swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED; // Entire window drawing so no scaling needed
		swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapDesc.SampleDesc.Count = 1; // AntiAliasing
		swapDesc.SampleDesc.Quality = 0;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Pipeline draws to this buffer
		swapDesc.Windowed = TRUE;
		swapDesc.BufferCount = 1; // Double buffering
		swapDesc.Flags = 0;

		UINT createFlags = 0;
#ifdef _DEBUG
		createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL features[]
		{
			D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0,
		};
		GFX_THROW_FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, nullptr,
			createFlags, nullptr, 0, D3D11_SDK_VERSION, &swapDesc, &swapChain, &device, features, &context));

		MsWrl::ComPtr<ID3D11Resource> backBuffer = nullptr;
		GFX_THROW_FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer)); // Get texture subresource (back buffer)
		GFX_THROW_FAILED(device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTarget)); // Create view to back buffer allowing writing data

		D3D11_DEPTH_STENCIL_DESC depthDesc = { 0 };
		depthDesc.DepthEnable = TRUE;
		depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
		depthDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
		MsWrl::ComPtr<ID3D11DepthStencilState> depthStencilState = nullptr;
		GFX_THROW_FAILED(device->CreateDepthStencilState(&depthDesc, &depthStencilState)); // Enable objects to overlap each other depending on Z coord
		context->OMSetDepthStencilState(depthStencilState.Get(), 1U);

		D3D11_TEXTURE2D_DESC depthTexDesc = { 0 };
		depthTexDesc.Width = width;
		depthTexDesc.Height = height;
		depthTexDesc.MipLevels = 0U; // Texture stuff
		depthTexDesc.ArraySize = 1U; // Only 1 texture
		depthTexDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
		depthTexDesc.SampleDesc.Count = 1U; // Antialiasing stuff
		depthTexDesc.SampleDesc.Quality = 0U;
		depthTexDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
		MsWrl::ComPtr<ID3D11Texture2D> depthTexture = nullptr;
		GFX_THROW_FAILED(device->CreateTexture2D(&depthTexDesc, nullptr, &depthTexture)); // Texture to render into with depth stencil

		D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc = { };
		depthViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
		depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
		depthViewDesc.Texture2D.MipSlice = 0U;
		GFX_THROW_FAILED(device->CreateDepthStencilView(depthTexture.Get(), &depthViewDesc, &depthStencil));
		context->OMSetRenderTargets(1U, renderTarget.GetAddressOf(), depthStencil.Get()); // Bind render target and depth stencil

		D3D11_VIEWPORT viewPort = { 0 };
		viewPort.Width = static_cast<FLOAT>(width);
		viewPort.Height = static_cast<FLOAT>(height);
		viewPort.MinDepth = 0;
		viewPort.MaxDepth = 1;
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;
		context->RSSetViewports(1U, &viewPort);

		ImGui_ImplDX11_Init(device.Get(), context.Get());
	}

	Graphics::~Graphics()
	{
		ImGui_ImplDX11_Shutdown();
	}

	void Graphics::DrawIndexed(UINT count) noexcept(!IS_DEBUG)
	{
		GFX_THROW_FAILED_INFO(context->DrawIndexed(count, 0U, 0U));
	}

	void Graphics::EndFrame()
	{
		if (guiEnabled)
		{
			ImGui::Render(); // Create data to render
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Pass it to DirectX
		}
		HRESULT result;
		GFX_SET_DEBUG_WATCH();
		if (FAILED(result = swapChain->Present(1U, 0U)))
		{
			if (result == DXGI_ERROR_DEVICE_REMOVED)
				throw GFX_DEV_REMOVED_EXCEPT(device->GetDeviceRemovedReason());
			else
				throw GFX_EXCEPT(result);
		}
	}

	void Graphics::BeginFrame(float red, float green, float blue) noexcept
	{
		const float color[] = { red, green, blue, 1.0f };
		context->ClearRenderTargetView(renderTarget.Get(), color);
		context->ClearDepthStencilView(depthStencil.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.0f, 0U);
		if (guiEnabled)
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}
	}
}
