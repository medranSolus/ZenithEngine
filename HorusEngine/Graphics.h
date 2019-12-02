#pragma once
#include "WinApiException.h"
#include "DXGIDebugInfoManager.h"
#include "GDIPlusManager.h"
#include "GUIManager.h"
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

namespace GFX
{
	namespace Resource
	{
		class IBindable;
	}
	class Graphics
	{
		friend class Resource::IBindable;

#ifdef _DEBUG
		DXGIDebugInfoManager debugInfoManager;
#endif
		WinAPI::GDIPlusManager gdiManager;
		GUIManager guiManager;
		bool guiEnabled = true;
		DirectX::XMMATRIX projection;
		DirectX::XMMATRIX camera;
		Microsoft::WRL::ComPtr<ID3D11Device> device = nullptr; // Resources allocation
		Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain = nullptr; // Using pipeline: https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-graphics-pipeline
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context = nullptr; // Configure pipeline
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTarget = nullptr; // Back buffer from swap chain
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencil = nullptr; // Information about Z-buffering (object overlaping)

	public:
		Graphics(HWND hWnd, unsigned int width, unsigned int height);
		Graphics(const Graphics &) = delete;
		Graphics & operator=(const Graphics &) = delete;
		~Graphics();

		constexpr GUIManager & Gui() noexcept { return guiManager; }
		constexpr DirectX::XMMATRIX GetProjection() const noexcept { return projection; }
		constexpr void SetProjection(DirectX::FXMMATRIX projection) noexcept { this->projection = projection; }
		constexpr DirectX::XMMATRIX GetCamera() const noexcept { return camera; }
		constexpr void SetCamera(DirectX::FXMMATRIX camera) noexcept { this->camera = camera; }
		constexpr void EnableGUI() noexcept { guiEnabled = true; }
		constexpr void DisableGUI() noexcept { guiEnabled = false; }
		constexpr void SwitchGUI() noexcept { guiEnabled = !guiEnabled; }
		constexpr bool IsGuiEnabled() const noexcept { return guiEnabled; }
#ifdef _DEBUG
		constexpr DXGIDebugInfoManager & GetInfoManager() { return debugInfoManager; }
#endif

		void DrawIndexed(UINT count) noexcept(!IS_DEBUG);
		void EndFrame();
		void BeginFrame(float red, float green, float blue) noexcept;

#pragma region Exceptions
#ifdef _DEBUG
		// Exception using DXGIDebugInfoManager
		class DebugException : public virtual Exception::BasicException
		{
			std::vector<std::string> debugInfo;

		public:
			DebugException(unsigned int line, const char * file, const std::vector<std::string> & info) noexcept : BasicException(line, file), debugInfo(info) {}

			inline const char * GetType() const noexcept override { return "DirectX Debug Exception"; }
			const char * what() const noexcept override;
			std::string GetDebugInfo() const noexcept;
		};
		class GraphicsException : public Exception::WinApiException, public DebugException
		{
		public:
			GraphicsException(unsigned int line, const char * file, HRESULT hResult, const std::vector<std::string> & info = std::vector<std::string>()) noexcept : BasicException(line, file), WinApiException(line, file, hResult), DebugException(line, file, info) {}
#else
		class GraphicsException : public Exception::WinApiException
		{
		public:
			GraphicsException(unsigned int line, const char * file, HRESULT hResult) noexcept : BasicException(line, file), WinApiException(line, file, hResult) {}
#endif
			inline const char * GetType() const noexcept override { return "DirectX Exception"; }
			const char * what() const noexcept override;
		};
		// Exception getting info from DXGI_ERROR_DEVICE_REMOVED error (driver crash, device hung, overheat, etc.)
		class DeviceRemovedException : public GraphicsException
		{
		public:
#ifdef _DEBUG
			DeviceRemovedException(unsigned int line, const char * file, HRESULT hResult, const std::vector<std::string> & info = std::vector<std::string>()) noexcept : BasicException(line, file), GraphicsException(line, file, hResult, info) {}
#else
			DeviceRemovedException(unsigned int line, const char * file, HRESULT hResult) noexcept : BasicException(line, file), GraphicsException(line, file, hResult) {}
#endif

			inline const char * GetType() const noexcept override { return "Graphics Removed Exception"; }
		};
#pragma endregion
	};
}
