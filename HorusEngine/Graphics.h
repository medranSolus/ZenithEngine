#pragma once
#include "RenderTarget.h"
#include "WinApiException.h"
#include "DXGIDebugInfoManager.h"
#include "GUIManager.h"
#include "Utils.h"
#include "ImGui/imgui_impl_dx11.h"
#include <d3d11_1.h>

namespace GFX
{
	class Graphics
	{
		friend class Resource::IBindable;

#ifdef _DEBUG
		Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> tagManager = nullptr;
		DXGIDebugInfoManager debugInfoManager;
#endif
		GUI::GUIManager guiManager;
		bool guiEnabled = true;
		DirectX::XMMATRIX projection;
		DirectX::XMMATRIX view;
		Microsoft::WRL::ComPtr<ID3D11Device> device = nullptr; // Resources allocation
		Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain = nullptr; // Using pipeline: https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-graphics-pipeline
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context = nullptr; // Configure pipeline
		GfxResPtr<Pipeline::Resource::RenderTarget> renderTarget; // Back buffer from swap chain

	public:
		Graphics(HWND hWnd, unsigned int width, unsigned int height);
		Graphics(const Graphics&) = delete;
		Graphics& operator=(const Graphics&) = delete;
		~Graphics();

		constexpr GUI::GUIManager& Gui() noexcept { return guiManager; }
		constexpr const DirectX::XMMATRIX& GetProjection() const noexcept { return projection; }
		constexpr void SetProjection(DirectX::XMMATRIX&& projectionMatrix) noexcept { projection = std::move(projectionMatrix); }
		constexpr const DirectX::XMMATRIX& GetView() const noexcept { return view; }
		constexpr void SetView(DirectX::XMMATRIX&& cameraMatrix) noexcept { view = std::move(cameraMatrix); }
		constexpr void EnableGUI() noexcept { guiEnabled = true; }
		constexpr void DisableGUI() noexcept { guiEnabled = false; }
		constexpr void SwitchGUI() noexcept { guiEnabled = !guiEnabled; }
		constexpr bool IsGuiEnabled() const noexcept { return guiEnabled; }
		constexpr unsigned int GetWidth() const noexcept { return renderTarget->GetWidth(); }
		constexpr unsigned int GetHeight() const noexcept { return renderTarget->GetHeight(); }
		constexpr float GetRatio() { return static_cast<float>(GetWidth()) / GetHeight(); }
		inline GfxResPtr<Pipeline::Resource::RenderTarget> GetBackBuffer() noexcept { return renderTarget; }
#ifdef _DEBUG
		constexpr DXGIDebugInfoManager& GetInfoManager() noexcept { return debugInfoManager; }
		inline void PushDrawTag(const std::string& tag) { tagManager->BeginEvent(Utils::ToUtf8(tag).c_str()); }
		inline void PopDrawTag() { tagManager->EndEvent(); }
#endif

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

#ifdef _DEBUG
#define DRAW_TAG_START(gfx, tag) gfx.PushDrawTag(tag)
#define DRAW_TAG_END(gfx) gfx.PopDrawTag();
#else
#define DRAW_TAG_START(gfx, tag)
#define DRAW_TAG_END(gfx)
#endif