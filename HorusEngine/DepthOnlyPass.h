#pragma once
#include "QueuePass.h"
#include "ICamera.h"

namespace GFX::Pipeline::RenderPass
{
	class DepthOnlyPass : public Base::QueuePass
	{
		Camera::ICamera* mainCamera = nullptr;

	public:
		DepthOnlyPass(Graphics& gfx, const std::string& name);
		virtual ~DepthOnlyPass() = default;

		constexpr void BindMainCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }

		void Execute(Graphics& gfx) override;
	};
}