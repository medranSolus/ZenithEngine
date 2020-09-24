#pragma once
#include "QueuePass.h"
#include "ICamera.h"

namespace GFX::Pipeline::RenderPass
{
	class LambertianClassicPass : public Base::QueuePass
	{
		Camera::ICamera* mainCamera = nullptr;

	public:
		LambertianClassicPass(Graphics& gfx, const std::string& name);
		virtual ~LambertianClassicPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }

		void Execute(Graphics& gfx) override;
	};
}