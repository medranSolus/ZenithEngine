#pragma once
#include "BindingPass.h"
#include "GFX/Pipeline/Resource/IRenderTarget.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	class ComputePass : public BindingPass
	{
	protected:
		GfxResPtr<Resource::IRenderTarget> computeTarget;

	public:
		ComputePass(Graphics& gfx, std::string&& name, const std::string& shaderName);
		ComputePass(ComputePass&&) = default;
		ComputePass(const ComputePass&) = default;
		ComputePass& operator=(ComputePass&&) = default;
		ComputePass& operator=(const ComputePass&) = default;
		virtual ~ComputePass() = default;

		void Finalize() override;
		void Compute(Graphics& gfx, U32 groupX, U32 groupY, U32 groupZ);
		void ComputeFrame(Graphics& gfx, U32 threadsX, U32 threadsY);
	};
}