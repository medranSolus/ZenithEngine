#pragma once
#include "ShadowMapCubePass.h"

namespace GFX::Pipeline::RenderPass
{
	class PointLightingPass : public Base::QueuePass
	{
		ShadowMapCubePass shadowMapPass;
		Camera::ICamera* mainCamera = nullptr;

	public:
		PointLightingPass(Graphics& gfx, const std::string& name, UINT shadowMapSize);
		virtual ~PointLightingPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; shadowMapPass.BindCamera(camera); }
		inline std::vector<Base::BasePass*> GetInnerPasses() override { return { &shadowMapPass }; }

		void Reset() noexcept override;
		Base::BasePass& GetInnerPass(std::deque<std::string> nameChain) override;
		void Execute(Graphics& gfx) override;
	};
}