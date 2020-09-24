#pragma once
#include "QueuePass.h"
#include "ICamera.h"

namespace GFX::Pipeline::RenderPass
{
	class LambertianDepthOptimizedPass : public Base::QueuePass
	{
		Camera::ICamera* mainCamera = nullptr;

	public:
		LambertianDepthOptimizedPass(Graphics& gfx, const std::string& name);
		virtual ~LambertianDepthOptimizedPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }

		void Execute(Graphics& gfx) override;
	};
}