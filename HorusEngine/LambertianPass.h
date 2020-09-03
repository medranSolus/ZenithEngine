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

		inline void BindMainCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }

		void Execute(Graphics& gfx) noexcept(!IS_DEBUG) override;
	};
}