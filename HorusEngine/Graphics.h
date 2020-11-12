#pragma once
#include "WinApiException.h"
#include "DXGIDebugInfoManager.h"
#include "GUIManager.h"
#include "ImGui/imgui_impl_dx11.h"
#include <d3d11.h>
#include <DirectXMath.h>

namespace GFX
{
	namespace Resource
	{
		class IBindable;
	}

	namespace Pipeline::Resource
	{
		class RenderTarget;
	}

	class Graphics
	{
		friend class Resource::IBindable;

#ifdef _DEBUG
		DXGIDebugInfoManager debugInfoManager;
#endif
		GUI::GUIManager guiManager;
		bool guiEnabled = true;
		DirectX::XMMATRIX projection;
		DirectX::XMMATRIX view;
		Microsoft::WRL::ComPtr<ID3D11Device> device = nullptr; // Resources allocation
		Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain = nullptr; // Using pipeline: https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-graphics-pipeline
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context = nullptr; // Configure pipeline
		std::shared_ptr<Pipeline::Resource::RenderTarget> renderTarget; // Back buffer from swap chain

	public:
		Graphics(HWND hWnd, unsigned int width, unsigned int height);
		Graphics(const Graphics&) = delete;
		Graphics& operator=(const Graphics&) = delete;
		inline ~Graphics() { ImGui_ImplDX11_Shutdown(); }

		constexpr GUI::GUIManager& Gui() noexcept { return guiManager; }
		constexpr const DirectX::XMMATRIX& GetProjection() const noexcept { return projection; }
		constexpr void SetProjection(DirectX::XMMATRIX&& projectionMatrix) noexcept { projection = std::move(projectionMatrix); }
		constexpr const DirectX::XMMATRIX& GetView() const noexcept { return view; }
		constexpr void SetView(DirectX::XMMATRIX&& cameraMatrix) noexcept { view = std::move(cameraMatrix); }
		constexpr void EnableGUI() noexcept { guiEnabled = true; }
		constexpr void DisableGUI() noexcept { guiEnabled = false; }
		constexpr void SwitchGUI() noexcept { guiEnabled = !guiEnabled; }
		constexpr bool IsGuiEnabled() const noexcept { return guiEnabled; }
#ifdef _DEBUG
		constexpr DXGIDebugInfoManager& GetInfoManager() { return debugInfoManager; }
#endif
		inline float GetRatio() { return static_cast<float>(GetWidth()) / GetHeight(); }
		inline std::shared_ptr<Pipeline::Resource::RenderTarget> GetBackBuffer() noexcept { return renderTarget; }

		unsigned int GetWidth() const noexcept;
		unsigned int GetHeight() const noexcept;

		void DrawIndexed(UINT count) noexcept(!IS_DEBUG);
		void EndFrame();
		void BeginFrame() noexcept;

#pragma region Exceptions
#ifdef _DEBUG
		// Exception using DXGIDebugInfoManager
		class DebugException : public virtual Exception::BasicException
		{
			std::vector<std::string> debugInfo;

		public:
			inline DebugException(unsigned int line, const char* file, const std::vector<std::string>& info) noexcept
				: BasicException(line, file), debugInfo(info) {}
			DebugException(const DebugException&) = default;
			DebugException& operator=(const DebugException&) = default;
			virtual ~DebugException() = default;

			inline const char* GetType() const noexcept override { return "DirectX Debug Exception"; }

			const char* what() const noexcept override;
			std::string GetDebugInfo() const noexcept;
		};
		class GraphicsException : public Exception::WinApiException, public DebugException
		{
		public:
			inline GraphicsException(unsigned int line, const char* file, HRESULT hResult, const std::vector<std::string>& info = std::vector<std::string>()) noexcept
				: BasicException(line, file), WinApiException(line, file, hResult), DebugException(line, file, info) {}
#else
		class GraphicsException : public Exception::WinApiException
		{
		public:
			inline GraphicsException(unsigned int line, const char* file, HRESULT hResult) noexcept
				: BasicException(line, file), WinApiException(line, file, hResult) {}
#endif
			GraphicsException(const GraphicsException&) = default;
			GraphicsException& operator=(const GraphicsException&) = default;
			virtual ~GraphicsException() = default;

			inline const char* GetType() const noexcept override { return "DirectX Exception"; }

			const char* what() const noexcept override;
		};
		// Exception getting info from DXGI_ERROR_DEVICE_REMOVED error (driver crash, device hung, overheat, etc.)
		class DeviceRemovedException : public GraphicsException
		{
		public:
#ifdef _DEBUG
			inline DeviceRemovedException(unsigned int line, const char* file, HRESULT hResult, const std::vector<std::string>& info = std::vector<std::string>()) noexcept
				: BasicException(line, file), GraphicsException(line, file, hResult, info) {}
#else
			inline DeviceRemovedException(unsigned int line, const char* file, HRESULT hResult) noexcept
				: BasicException(line, file), GraphicsException(line, file, hResult) {}
#endif
			DeviceRemovedException(const DeviceRemovedException&) = default;
			DeviceRemovedException& operator=(const DeviceRemovedException&) = default;
			virtual ~DeviceRemovedException() = default;

			inline const char* GetType() const noexcept override { return "Graphics Removed Exception"; }
		};
#pragma endregion
	};
}