#pragma once
#include "FullscreenPass.h"
#include "ConstBufferExCache.h"

namespace GFX::Pipeline::RenderPass
{
	class SSAOBlurPass : public Base::FullscreenPass
	{
		GfxResPtr<Resource::IRenderTarget> ssaoScratchBuffer;
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> direction;
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> kernelBuffer;

	public:
		SSAOBlurPass(Graphics& gfx, const std::string& name);
		virtual ~SSAOBlurPass() = default;

		void Execute(Graphics& gfx) override;
	};
}