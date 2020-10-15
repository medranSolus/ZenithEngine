#pragma once
#include "FullscreenPass.h"
#include "ConstBufferExCache.h"

namespace GFX::Pipeline::RenderPass
{
	class SSAOBlurPass : public Base::FullscreenPass
	{
		std::shared_ptr<Resource::IRenderTarget> ssaoScratchBuffer;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> direction;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> kernelBuffer;

	public:
		SSAOBlurPass(Graphics& gfx, const std::string& name);
		virtual ~SSAOBlurPass() = default;

		void Execute(Graphics& gfx) override;
	};
}