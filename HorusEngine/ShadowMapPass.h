#pragma once
#include "QueuePass.h"
#include "ILight.h"
#include "TextureDepthCube.h"

namespace GFX::Pipeline::RenderPass
{
	class ShadowMapPass : public Base::QueuePass
	{
		static constexpr UINT DEPTH_TEXTURE_SIZE = 1024U;

		Light::ILight* shadowSource = nullptr;
		std::shared_ptr<GFX::Resource::TextureDepthCube> depthCube;
		DirectX::XMFLOAT4X4 projection;
		std::vector<DirectX::XMFLOAT3> cameraDirections;
		std::vector<DirectX::XMFLOAT3> cameraUps;

	public:
		ShadowMapPass(Graphics& gfx, const std::string& name);
		virtual ~ShadowMapPass() = default;

		inline void BindLight(Light::ILight& light) noexcept { shadowSource = &light; }

		void Execute(Graphics& gfx) noexcept(!IS_DEBUG) override;
	};
}