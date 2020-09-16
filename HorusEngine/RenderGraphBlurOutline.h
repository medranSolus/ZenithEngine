#pragma once
#include "RenderGraph.h"
#include "ConstBufferExCache.h"
#include "TextureCube.h"
#include "ICamera.h"
#include "BaseLight.h"

namespace GFX::Pipeline
{
	class RenderGraphBlurOutline : public RenderGraph
	{
		static constexpr int MAX_RADIUS = 7;
		int radius;
		float sigma;

		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> kernel;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> blurDirection;
		std::shared_ptr<GFX::Resource::TextureCube> skyboxTexture;

		void SetKernel() noexcept(!IS_DEBUG);

	public:
		RenderGraphBlurOutline(Graphics& gfx, int radius = 7, float sigma = 2.6f);
		virtual ~RenderGraphBlurOutline() = default;

		void BindMainCamera(Camera::ICamera& camera);
		void BindLight(Light::BaseLight& light);
		void SetKernel(int radius, float sigma) noexcept(!IS_DEBUG);
		void ShowWindow() noexcept;
	};
}