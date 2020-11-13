#pragma once
#include "FullscreenPass.h"
#include "ConstBufferExCache.h"

namespace GFX::Pipeline::RenderPass
{
	class HorizontalBlurPass : public Base::FullscreenPass
	{
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> direction;

	public:
		HorizontalBlurPass(Graphics& gfx, const std::string& name, unsigned int width, unsigned int height);
		virtual ~HorizontalBlurPass() = default;

		void Execute(Graphics& gfx) override;
	};
}