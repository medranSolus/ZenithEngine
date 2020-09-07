#pragma once
#include "BindingPass.h"
#include "ICamera.h"

namespace GFX::Pipeline::RenderPass
{
	class SkyboxPass : public Base::BindingPass
	{
		UINT indexCount;
		Camera::ICamera* mainCamera = nullptr;

	public:
		SkyboxPass(Graphics& gfx, const std::string& name);
		virtual ~SkyboxPass() = default;

		inline void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }

		void Execute(Graphics& gfx) noexcept(!IS_DEBUG) override;
	};
}