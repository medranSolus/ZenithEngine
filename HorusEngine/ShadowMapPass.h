#pragma once
#include "QueuePass.h"
#include "ILight.h"
#include "ConstBufferVertex.h"
#include "ConstBufferExCache.h"
#include "TextureDepthCube.h"

namespace GFX::Pipeline::RenderPass
{
	class ShadowMapPass : public Base::QueuePass
	{
		static constexpr UINT DEPTH_TEXTURE_SIZE = 1024U;

		int bias = 2;
		Light::ILight* shadowSource = nullptr;
		std::shared_ptr<GFX::Resource::ConstBufferVertex<DirectX::XMFLOAT4>> positionBuffer;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> biasBuffer;
		std::shared_ptr<GFX::Resource::TextureDepthCube> depthCube;
		DirectX::XMFLOAT4X4 projection;
		std::vector<DirectX::XMFLOAT3> cameraDirections;
		std::vector<DirectX::XMFLOAT3> cameraUps;

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		ShadowMapPass(Graphics& gfx, const std::string& name);
		virtual ~ShadowMapPass() = default;

		constexpr void BindLight(Light::ILight& light) noexcept { shadowSource = &light; }

		void Execute(Graphics& gfx) override;
		void ShowWindow(Graphics& gfx);
	};
}