#pragma once
#include "GFX/Pipeline/RenderPass/Base/BindingPass.h"
#include "Camera/ICamera.h"

namespace GFX::Pipeline::RenderPass
{
	class SkyboxPass : public Base::BindingPass
	{
		U32 indexCount;
		Camera::ICamera* mainCamera = nullptr;

	public:
		SkyboxPass(Graphics& gfx, std::string&& name);
		virtual ~SkyboxPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }

		void Execute(Graphics& gfx) override;
	};
}