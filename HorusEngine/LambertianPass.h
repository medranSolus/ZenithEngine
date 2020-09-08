#pragma once
#include "QueuePass.h"
#include "ICamera.h"
#include "ConstBufferShadow.h"

namespace GFX::Pipeline::RenderPass
{
	class LambertianPass : public Base::QueuePass
	{
		Camera::ICamera* mainCamera = nullptr;
		std::shared_ptr<GFX::Resource::ConstBufferShadow> shadowBuffer = nullptr;

	public:
		LambertianPass(Graphics& gfx, const std::string& name);
		virtual ~LambertianPass() = default;

		inline void BindMainCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }
		inline void BindLight(Light::ILight& light) noexcept { shadowBuffer->SetLight(light); }

		void Execute(Graphics& gfx) noexcept(!IS_DEBUG) override;
	};
}