#pragma once
#include "Exception/WinApiException.h"
#include "Pipeline/Resource/RenderTarget.h"
#include "DXGIDebugInfoManager.h"
#include "GUI/GUIManager.h"

namespace GFX
{
	class Graphics
	{
		friend class Resource::IBindable;

#ifdef _MODE_DEBUG
		Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> tagManager = nullptr;
		DXGIDebugInfoManager debugInfoManager;
#endif
		GUI::GUIManager guiManager;
		bool guiEnabled = true;
		Matrix projection;
		Matrix view;
		Microsoft::WRL::ComPtr<ID3D11Device> device = nullptr; // Resources allocation
		Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain = nullptr; // Using pipeline: https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-graphics-pipeline
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context = nullptr; // Configure pipeline
		GfxResPtr<Pipeline::Resource::RenderTarget> renderTarget; // Back buffer from swap chain

	public:
		Graphics(HWND hWnd, U32 width, U32 height);
		Graphics(const Graphics&) = delete;
		Graphics& operator=(const Graphics&) = delete;
		~Graphics();

		constexpr void EnableGUI() noexcept { guiEnabled = true; }
		constexpr void DisableGUI() noexcept { guiEnabled = false; }
		constexpr void SwitchGUI() noexcept { guiEnabled = !guiEnabled; }
		constexpr bool IsGuiEnabled() const noexcept { return guiEnabled; }
		constexpr GUI::GUIManager& Gui() noexcept { return guiManager; }

		constexpr const Matrix& GetProjection() const noexcept { return projection; }
		constexpr const Matrix& GetView() const noexcept { return view; }
		constexpr void SetProjection(Matrix&& projectionMatrix) noexcept { projection = std::move(projectionMatrix); }
		constexpr void SetView(Matrix&& cameraMatrix) noexcept { view = std::move(cameraMatrix); }

		constexpr U32 GetWidth() const noexcept { return renderTarget->GetWidth(); }
		constexpr U32 GetHeight() const noexcept { return renderTarget->GetHeight(); }
		constexpr float GetRatio() { return static_cast<float>(GetWidth()) / GetHeight(); }

		GfxResPtr<Pipeline::Resource::RenderTarget> GetBackBuffer() noexcept { return renderTarget; }
#ifdef _MODE_DEBUG
		constexpr DXGIDebugInfoManager& GetInfoManager() noexcept { return debugInfoManager; }
		void PushDrawTag(const std::string& tag) { tagManager->BeginEvent(Utils::ToUtf8(tag).c_str()); }
		void PopDrawTag() { tagManager->EndEvent(); }
#endif

		void DrawIndexed(U32 count) noexcept(_NO_DEBUG);
		void EndFrame();
		void BeginFrame() noexcept;
	};
}

#ifdef _MODE_DEBUG
#define DRAW_TAG_START(gfx, tag) gfx.PushDrawTag(tag)
#define DRAW_TAG_END(gfx) gfx.PopDrawTag();
#else
#define DRAW_TAG_START(gfx, tag)
#define DRAW_TAG_END(gfx)
#endif