#pragma once
#include "QueuePass.h"
#include "ICamera.h"

namespace GFX::Pipeline::RenderPass
{
	class LambertianPass : public Base::QueuePass
	{
		Camera::ICamera* mainCamera = nullptr;

	public:
		LambertianPass(Graphics& gfx, const std::string& name);
		virtual ~LambertianPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }

		void Execute(Graphics& gfx) override;
	};
}