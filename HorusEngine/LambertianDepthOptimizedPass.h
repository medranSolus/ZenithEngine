#pragma once
#include "QueuePass.h"
#include "ICamera.h"
#include "NullPixelShader.h"
#include "VertexShader.h"
#include "DepthStencilState.h"

namespace GFX::Pipeline::RenderPass
{
	class LambertianDepthOptimizedPass : public Base::QueuePass
	{
		Camera::ICamera* mainCamera = nullptr;
		std::shared_ptr<GFX::Resource::NullPixelShader> depthOnlyPS = nullptr;
		std::shared_ptr<GFX::Resource::VertexShader> depthOnlyVS = nullptr;
		std::shared_ptr<GFX::Resource::DepthStencilState> depthOnlyStencilState = nullptr;
		std::shared_ptr<GFX::Resource::DepthStencilState> lambertianStencilState = nullptr;

	public:
		LambertianDepthOptimizedPass(Graphics& gfx, const std::string& name);
		virtual ~LambertianDepthOptimizedPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }

		void Execute(Graphics& gfx) override;
	};
}