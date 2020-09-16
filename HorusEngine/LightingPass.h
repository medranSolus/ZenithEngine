#pragma once
#include "ShadowMapPass.h"

namespace GFX::Pipeline::RenderPass
{
	class LightingPass : public Base::QueuePass
	{
		std::unique_ptr<ShadowMapPass> shadowMapPass = nullptr;

	public:
		LightingPass(Graphics& gfx, const std::string& name);
		virtual ~LightingPass() = default;

		void Reset() noexcept override;
		Base::BasePass& GetInnerPass(std::deque<std::string> nameChain) override;
		void Execute(Graphics& gfx) override;
	};
}