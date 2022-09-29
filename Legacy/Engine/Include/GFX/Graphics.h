#pragma once
#include "Pipeline/Resource/RenderTarget.h"
#include "DXGIDebugInfoManager.h"
#include "GUI/GUIManager.h"

namespace ZE::GFX
{
	class Graphics
	{
		friend class Resource::IBindable;

		Matrix projection;
		Matrix view;
		GfxResPtr<Pipeline::Resource::RenderTarget> renderTarget; // Back buffer from swap chain
		bool fullscreen = false;

	public:
		constexpr const Matrix& GetProjection() const noexcept { return projection; }
		constexpr const Matrix& GetView() const noexcept { return view; }
		constexpr void SetProjection(Matrix&& projectionMatrix) noexcept { projection = std::move(projectionMatrix); }
		constexpr void SetView(Matrix&& cameraMatrix) noexcept { view = std::move(cameraMatrix); }

		constexpr U32 GetWidth() const noexcept { return renderTarget->GetWidth(); }
		constexpr U32 GetHeight() const noexcept { return renderTarget->GetHeight(); }
		constexpr float GetRatio() { return static_cast<float>(GetWidth()) / GetHeight(); }

		GfxResPtr<Pipeline::Resource::RenderTarget> GetBackBuffer() noexcept { return renderTarget; }

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