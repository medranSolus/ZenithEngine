#pragma once
#include "GFX/Pipeline/RenderPass/Base/FullscreenPass.h"
#include "GFX/Resource/ConstBufferExCache.h"

namespace GFX::Pipeline::RenderPass
{
	class VerticalBlurPass : public Base::FullscreenPass
	{
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> direction;

	public:
		VerticalBlurPass(Graphics& gfx, std::string&& name);
		virtual ~VerticalBlurPass() = default;

		void Execute(Graphics& gfx) override;
	};
}