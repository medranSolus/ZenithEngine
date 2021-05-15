#pragma once
#include "GFX/Pipeline/RenderPass/Base/QueuePass.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	class LambertianClassicPass : public Base::QueuePass
	{
		Camera::ICamera* mainCamera = nullptr;

	public:
		LambertianClassicPass(Graphics& gfx, std::string&& name);
		virtual ~LambertianClassicPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }

		void Execute(Graphics& gfx) override;
	};
}