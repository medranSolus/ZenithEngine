#pragma once
#include "QueuePass.h"
#include "ILight.h"
#include "ConstBufferVertex.h"
#include "ConstBufferExCache.h"
#include "TextureDepthCube.h"

namespace GFX::Pipeline::RenderPass
{
	class ShadowMapCubePass : public Base::QueuePass
	{
		Camera::ICamera* mainCamera = nullptr;
		Light::ILight* shadowSource = nullptr;
		GfxResPtr<GFX::Resource::ConstBufferPixel<DirectX::XMFLOAT4>> positionBuffer;
		GfxResPtr<GFX::Resource::ConstBufferExGeometryCache> viewBuffer;
		GfxResPtr<GFX::Resource::TextureDepthCube> depthCube;
		DirectX::XMFLOAT4X4 projection;

		static inline Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		ShadowMapCubePass(Graphics& gfx, const std::string& name, UINT mapSize);
		virtual ~ShadowMapCubePass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }
		constexpr void BindLight(Light::ILight& light) noexcept { shadowSource = &light; }

		void Execute(Graphics& gfx) override;
	};
}