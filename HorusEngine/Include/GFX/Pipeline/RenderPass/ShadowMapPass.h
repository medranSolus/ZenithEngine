#pragma once
#include "GFX/Pipeline/RenderPass/Base/QueuePass.h"
#include "GFX/Light/ILight.h"
#include "GFX/Resource/ConstBufferVertex.h"
#include "GFX/Resource/ConstBufferExCache.h"

namespace GFX::Pipeline::RenderPass
{
	class ShadowMapPass : public Base::QueuePass
	{
		S32 depthBias = 40;
		float slopeBias = 5.0f;
		float biasClamp = 0.1f;

		Camera::ICamera* mainCamera = nullptr;
		const Light::ILight* shadowSource = nullptr;
		GfxResPtr<GFX::Resource::ConstBufferPixel<Float4>> positionBuffer;
		Float4x4 projection;

	public:
		ShadowMapPass(Graphics& gfx, std::string&& name, const Matrix& projectionMatrix);
		virtual ~ShadowMapPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }
		constexpr void BindLight(const Light::ILight& light) noexcept { shadowSource = &light; }

		void Execute(Graphics& gfx) override;
	};
}