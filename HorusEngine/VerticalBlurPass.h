#pragma once
#include "FullscreenPass.h"
#include "ConstBufferExCache.h"

namespace GFX::Pipeline::RenderPass
{
	class VerticalBlurPass : public Base::FullscreenPass
	{
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> direction;

	public:
		VerticalBlurPass(Graphics& gfx, const std::string& name);
		virtual ~VerticalBlurPass() = default;

		void Execute(Graphics& gfx) override;
	};
}