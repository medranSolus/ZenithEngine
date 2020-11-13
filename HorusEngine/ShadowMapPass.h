#pragma once
#include "QueuePass.h"
#include "ILight.h"
#include "ConstBufferVertex.h"
#include "ConstBufferExCache.h"

namespace GFX::Pipeline::RenderPass
{
	class ShadowMapPass : public Base::QueuePass
	{
		int depthBias = 40;
		float slopeBias = 5.0f;
		float biasClamp = 0.1f;

		Camera::ICamera* mainCamera = nullptr;
		Light::ILight* shadowSource = nullptr;
		GfxResPtr<GFX::Resource::ConstBufferPixel<DirectX::XMFLOAT4>> positionBuffer;
		DirectX::XMFLOAT4X4 projection;

	public:
		ShadowMapPass(Graphics& gfx, const std::string& name, const DirectX::XMMATRIX& projectionMatrix);
		virtual ~ShadowMapPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }
		constexpr void BindLight(Light::ILight& light) noexcept { shadowSource = &light; }

		void Execute(Graphics& gfx) override;
	};
}