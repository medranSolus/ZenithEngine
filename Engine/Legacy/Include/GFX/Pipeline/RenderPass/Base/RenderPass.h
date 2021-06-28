#pragma once
#include "BindingPass.h"
#include "GFX/Pipeline/Resource/IRenderTarget.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	class RenderPass : public BindingPass
	{
		void BindResources(Graphics& gfx);

	protected:
		GfxResPtr<Resource::IRenderTarget> renderTarget;
		GfxResPtr<Resource::DepthStencil> depthStencil;

		void BindAll(Graphics& gfx) { BindResources(gfx); BindRenderAll(gfx); }

		using BindingPass::BindingPass;

	public:
		RenderPass(RenderPass&&) = default;
		RenderPass(const RenderPass&) = default;
		RenderPass& operator=(RenderPass&&) = default;
		RenderPass& operator=(const RenderPass&) = default;
		virtual ~RenderPass() = default;

		void Finalize() override;
	};
}