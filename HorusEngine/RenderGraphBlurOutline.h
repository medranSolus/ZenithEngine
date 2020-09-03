#pragma once
#include "RenderGraph.h"
#include "LambertianPass.h"
#include "ConstBufferExCache.h"

namespace GFX::Pipeline
{
	class RenderGraphBlurOutline : public RenderGraph
	{
		static constexpr int maxRadius = 7;
		int radius;
		float sigma;

		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> kernel;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> blurDirection;

		void SetKernel();

	public:
		RenderGraphBlurOutline(Graphics& gfx, int radius = 7, float sigma = 2.6f);
		virtual ~RenderGraphBlurOutline() = default;

		inline void BindMainCamera(Camera::ICamera& camera) { dynamic_cast<RenderPass::LambertianPass&>(FindPass("lambertian")).BindMainCamera(camera); }

		void BindShadowCamera(Camera::ICamera& camera);
		void SetKernel(int radius, float sigma);
		void ShowWindow() noexcept;
	};
}