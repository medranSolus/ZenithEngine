#pragma once
#include "Pipeline/Resource/RenderTarget.h"
#include "DXGIDebugInfoManager.h"
#include "GUI/GUIManager.h"

namespace ZE::GFX
{
	class Graphics
	{
		friend class Resource::IBindable;
		friend class GPerf;

		static constexpr DXGI_FORMAT BACKBUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;

#ifdef _ZE_MODE_DEBUG
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
		bool fullscreen = false;

	public:
		Graphics() = default;
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
		void ToggleFullscreen() { fullscreen ? SetWindowed() : SetFullscreen(); }
#ifdef _ZE_MODE_DEBUG
		constexpr DXGIDebugInfoManager& GetInfoManager() noexcept { return debugInfoManager; }
		void PushDrawTag(const std::string& tag) { tagManager->BeginEvent(Utils::ToUtf8(tag).c_str()); }
		void PopDrawTag() { tagManager->EndEvent(); }
#endif
		// For best performance each thread group should be multiple of 32 (ideally as many as 2*processors on GPU)
		void Compute(U32 groupX, U32 groupY, U32 groupZ) noexcept { context->Dispatch(groupX, groupY, groupZ); }

		void Init(HWND hWnd, U32 width, U32 height);
		void DrawIndexed(U32 count) noexcept(ZE_NO_DEBUG);
		void ComputeFrame(U32 threadsX, U32 threadsY) noexcept;
		void EndFrame();
		void BeginFrame() noexcept;
		void Resize(U32 width, U32 height);
		void SetFullscreen();
		void SetWindowed();
	};
}

#ifdef _ZE_MODE_DEBUG
#define ZE_DRAW_TAG_START(gfx, tag) gfx.PushDrawTag(tag)
#define ZE_DRAW_TAG_END(gfx) gfx.PopDrawTag();
#else
#define ZE_DRAW_TAG_START(gfx, tag)
#define ZE_DRAW_TAG_END(gfx)
#endif