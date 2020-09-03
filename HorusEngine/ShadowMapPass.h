#pragma once
#include "QueuePass.h"
#include "ICamera.h"

namespace GFX::Pipeline::RenderPass
{
	class ShadowMapPass : public Base::QueuePass
	{
		Camera::ICamera* shadowCamera = nullptr;

	public:
		ShadowMapPass(Graphics& gfx, const std::string& name);
		virtual ~ShadowMapPass() = default;

		inline void BindCamera(Camera::ICamera& camera) noexcept { shadowCamera = &camera; }

		void Execute(Graphics& gfx) noexcept(!IS_DEBUG) override;
	};
}