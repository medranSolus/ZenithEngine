#pragma once
#include "ShadowMapPass.h"

namespace GFX::Pipeline::RenderPass
{
	class DirectionalLightingPass : public Base::QueuePass
	{
		ShadowMapPass shadowMapPass;
		Camera::ICamera* mainCamera = nullptr;

	public:
		DirectionalLightingPass(Graphics& gfx, const std::string& name);
		virtual ~DirectionalLightingPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; shadowMapPass.BindCamera(camera); }
		inline std::vector<Base::BasePass*> GetInnerPasses() override { return { &shadowMapPass }; }

		void Reset() noexcept override;
		Base::BasePass& GetInnerPass(std::deque<std::string> nameChain) override;
		void Execute(Graphics& gfx) override;
	};
}