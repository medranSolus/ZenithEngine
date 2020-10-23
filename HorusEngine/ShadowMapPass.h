#pragma once
#include "QueuePass.h"
#include "ICamera.h"
#include "ILight.h"
#include "ConstBufferVertex.h"
#include "ConstBufferExCache.h"

namespace GFX::Pipeline::RenderPass
{
	class ShadowMapPass : public Base::QueuePass
	{
		Camera::ICamera* mainCamera = nullptr;
		Light::ILight* shadowSource = nullptr;
		std::shared_ptr<GFX::Resource::ConstBufferVertex<DirectX::XMFLOAT4>> positionBuffer;
		DirectX::XMFLOAT4X4 projection;

	public:
		ShadowMapPass(Graphics& gfx, const std::string& name, const DirectX::XMMATRIX& projectionMatrix);
		virtual ~ShadowMapPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }
		constexpr void BindLight(Light::ILight& light) noexcept { shadowSource = &light; }

		void Execute(Graphics& gfx) override;
	};
}