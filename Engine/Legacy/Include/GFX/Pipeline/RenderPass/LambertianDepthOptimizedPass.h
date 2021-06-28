#pragma once
#include "GFX/Pipeline/RenderPass/Base/QueuePass.h"
#include "GFX/Resource/Shader.h"
#include "GFX/Resource/NullShader.h"
#include "GFX/Resource/DepthStencilState.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	class LambertianDepthOptimizedPass : public Base::QueuePass
	{
		Camera::ICamera* mainCamera = nullptr;
		GfxResPtr<GFX::Resource::NullPixelShader> depthOnlyPS;
		GfxResPtr<GFX::Resource::VertexShader> depthOnlyVS;
		GfxResPtr<GFX::Resource::DepthStencilState> depthOnlyStencilState;
		GfxResPtr<GFX::Resource::DepthStencilState> lambertianStencilState;

	public:
		LambertianDepthOptimizedPass(Graphics& gfx, std::string&& name);
		virtual ~LambertianDepthOptimizedPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }

		void Execute(Graphics& gfx) override;
	};
}