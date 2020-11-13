#include "RenderTarget.h"
#include "GfxExceptionMacros.h"
#include "ImGui/imgui_impl_win32.h"

namespace GFX
{
	Graphics::Graphics(HWND hWnd, unsigned int width, unsigned int height)
	{
		GFX_ENABLE_EXCEPT();
		DXGI_SWAP_CHAIN_DESC swapDesc = { 0 };
		swapDesc.OutputWindow = hWnd;
		swapDesc.BufferDesc.Width = 0; // Use window size
		swapDesc.BufferDesc.Height = 0;
		swapDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		swapDesc.BufferDesc.RefreshRate.Numerator = 0; // Refresh in fullscreen
		swapDesc.BufferDesc.RefreshRate.Denominator = 0;
		swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED; // Entire window drawing so no scaling needed
		swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapDesc.SampleDesc.Count = 1; // AntiAliasing
		swapDesc.SampleDesc.Quality = 0;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD; // http://aka.ms/dxgiflipmodel.
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Pipeline draws to this buffer
		swapDesc.Windowed = TRUE;
		swapDesc.BufferCount = 2; // Triple buffering
		swapDesc.Flags = 0;

		UINT createFlags = 0;
#ifdef _DEBUG
		createFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL features[]
		{
			D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0,
		};
		GFX_THROW_FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, nullptr,
			createFlags, nullptr, 0, D3D11_SDK_VERSION, &swapDesc, &swapChain, &device, features, &context));

		Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer = nullptr;
		GFX_THROW_FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))); // Get texture subresource (back buffer)
		renderTarget = GfxResPtr<Pipeline::Resource::RenderTarget>(*this, width, height, backBuffer, swapDesc.BufferDesc.Format); // Create view to back buffer allowing writing data
#ifdef _DEBUG
		GFX_THROW_FAILED(context->QueryInterface(IID_PPV_ARGS(&tagManager)));
#endif
		ImGui_ImplDX11_Init(device.Get(), context.Get());
	}

	Graphics::~Graphics()
	{
		ImGui_ImplDX11_Shutdown();
#ifdef _DEBUG
		Microsoft::WRL::ComPtr<ID3D11Debug> debug;
		device->QueryInterface(IID_PPV_ARGS(&debug));
		if (debug != nullptr)
			debug->ReportLiveDeviceObjects(D3D11_RLDO_IGNORE_INTERNAL);
#endif
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
#ifdef _DEBUG
			PushDrawTag("ImGui");
#endif
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Pass it to DirectX
#ifdef _DEBUG
			PopDrawTag();
#endif
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

	void Graphics::BeginFrame() noexcept
	{
		if (guiEnabled)
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}
	}

#pragma region Exceptions
#ifdef _DEBUG
	const char* Graphics::DebugException::what() const noexcept
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
			stream << "\n[Debug Error Info]";
			for (size_t i = 0; i < size; ++i)
				stream << "\n[" << i + 1 << "] " << debugInfo.at(i);
			stream << std::endl;
		}
		return stream.str();
	}
#endif

	const char* Graphics::GraphicsException::what() const noexcept
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
}