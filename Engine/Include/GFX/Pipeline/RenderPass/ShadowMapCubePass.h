#pragma once
#include "GFX/Pipeline/RenderPass/Base/QueuePass.h"
#include "GFX/Light/ILight.h"
#include "GFX/Resource/ConstBufferVertex.h"
#include "GFX/Resource/ConstBufferExCache.h"
#include "GFX/Resource/TextureDepthCube.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	class ShadowMapCubePass : public Base::QueuePass
	{
		Camera::ICamera* mainCamera = nullptr;
		const Light::ILight* shadowSource = nullptr;
		GfxResPtr<GFX::Resource::ConstBufferPixel<Float4>> positionBuffer;
		GfxResPtr<GFX::Resource::ConstBufferExGeometryCache> viewBuffer;
		GfxResPtr<GFX::Resource::TextureDepthCube> depthCube;
		Float4x4 projection;

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		ShadowMapCubePass(Graphics& gfx, std::string&& name, U32 mapSize);
		virtual ~ShadowMapCubePass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }
		constexpr void BindLight(const Light::ILight& light) noexcept { shadowSource = &light; }

		void Execute(Graphics& gfx) override;
	};
}