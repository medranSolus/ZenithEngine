#pragma once
#include "ShadowMapPass.h"

namespace GFX::Pipeline::RenderPass
{
	class LightingPass : public Base::QueuePass
	{
		std::unique_ptr<ShadowMapPass> shadowMapPass = nullptr;
		Camera::ICamera* mainCamera = nullptr;

	public:
		LightingPass(Graphics& gfx, const std::string& name);
		virtual ~LightingPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; shadowMapPass->BindCamera(camera); }
		inline void ShowWindow(Graphics& gfx) { shadowMapPass->ShowWindow(gfx); }

		void Reset() noexcept override;
		Base::BasePass& GetInnerPass(std::deque<std::string> nameChain) override;
		void Execute(Graphics& gfx) override;
	};
}