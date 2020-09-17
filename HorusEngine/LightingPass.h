#pragma once
#include "ShadowMapPass.h"
#include "FullscreenPass.h"
#include "ConstBufferShadow.h"
#include "ICamera.h"

namespace GFX::Pipeline::RenderPass
{
	class LightingPass : public Base::QueuePass, public Base::FullscreenPass
	{
		std::unique_ptr<ShadowMapPass> shadowMapPass = nullptr;
		std::shared_ptr<GFX::Resource::ConstBufferShadow> shadowBuffer = nullptr;
		Camera::ICamera* mainCamera = nullptr;

	public:
		LightingPass(Graphics& gfx, const std::string& name);
		virtual ~LightingPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }

		void Reset() noexcept override;
		Base::BasePass& GetInnerPass(std::deque<std::string> nameChain) override;
		void Execute(Graphics& gfx) override;
	};
}