#pragma once
#include "BindingPass.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	class FullscreenPass : public virtual BindingPass
	{
	public:
		FullscreenPass(Graphics& gfx, std::string&& name, const std::string& vertexShaderName = "FullscreenVS");
		FullscreenPass(FullscreenPass&&) = default;
		FullscreenPass(const FullscreenPass&) = default;
		FullscreenPass& operator=(FullscreenPass&&) = default;
		FullscreenPass& operator=(const FullscreenPass&) = default;
		virtual ~FullscreenPass() = default;

		void Execute(Graphics& gfx) override;
	};
}