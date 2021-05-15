#pragma once
#include "ShadowMapCubePass.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	class PointLightingPass : public Base::QueuePass
	{
		ShadowMapCubePass shadowMapPass;
		Camera::ICamera* mainCamera = nullptr;

	public:
		PointLightingPass(Graphics& gfx, std::string&& name, U32 shadowMapSize);
		virtual ~PointLightingPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; shadowMapPass.BindCamera(camera); }
		std::vector<Base::BasePass*> GetInnerPasses() override { return { &shadowMapPass }; }

		void Reset() noexcept override;
		Base::BasePass& GetInnerPass(const std::deque<std::string>& nameChain) override;
		void Execute(Graphics& gfx) override;
	};
}