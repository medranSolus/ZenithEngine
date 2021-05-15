#pragma once
#include "GFX/Pipeline/RenderPass/Base/FullscreenPass.h"
#include "GFX/Resource/ConstBufferExCache.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	class SSAOBlurPass : public Base::FullscreenPass
	{
		GfxResPtr<Resource::IRenderTarget> ssaoScratchBuffer;
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> direction;
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> kernelBuffer;

	public:
		SSAOBlurPass(Graphics& gfx, std::string&& name);
		virtual ~SSAOBlurPass() = default;

		void Execute(Graphics& gfx) override;
	};
}