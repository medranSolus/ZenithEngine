#pragma once
#include "RenderGraph.h"
#include "LambertianPass.h"
#include "ConstBufferExCache.h"
#include "TextureCube.h"

namespace GFX::Pipeline
{
	class RenderGraphBlurOutline : public RenderGraph
	{
		static constexpr int maxRadius = 7;
		int radius;
		float sigma;

		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> kernel;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> blurDirection;
		std::shared_ptr<GFX::Resource::TextureCube> skyboxTexture;

		void SetKernel();

	public:
		RenderGraphBlurOutline(Graphics& gfx, int radius = 7, float sigma = 2.6f);
		virtual ~RenderGraphBlurOutline() = default;

		void BindMainCamera(Camera::ICamera& camera);
		void BindShadowCamera(Camera::ICamera& camera);
		void SetKernel(int radius, float sigma);
		void ShowWindow() noexcept;
	};
}