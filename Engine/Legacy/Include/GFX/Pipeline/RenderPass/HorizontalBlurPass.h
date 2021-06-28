#pragma once
#include "GFX/Pipeline/RenderPass/Base/FullscreenPass.h"
#include "GFX/Resource/ConstBufferExCache.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	class HorizontalBlurPass : public Base::FullscreenPass
	{
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> direction;

	public:
		HorizontalBlurPass(Graphics& gfx, std::string&& name, U32 width, U32 height);
		virtual ~HorizontalBlurPass() = default;

		void Execute(Graphics& gfx) override;
	};
}