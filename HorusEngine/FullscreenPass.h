#pragma once
#include "BindingPass.h"

namespace GFX::Pipeline::RenderPass::Base
{
	class FullscreenPass : public virtual BindingPass
	{
	public:
		FullscreenPass(Graphics& gfx, const std::string& name);
		virtual ~FullscreenPass() = default;

		void Execute(Graphics& gfx) override;
	};
}