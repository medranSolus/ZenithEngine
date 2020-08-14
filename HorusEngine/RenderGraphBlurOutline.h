#pragma once
#include "RenderGraph.h"
#include "ConstBufferExCache.h"

namespace GFX::Pipeline
{
	class RenderGraphBlurOutline : public RenderGraph
	{
		static constexpr int maxRadius = 7;

		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> kernel;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> blurDirection;

	public:
		RenderGraphBlurOutline(Graphics& gfx);
		virtual ~RenderGraphBlurOutline() = default;

		void SetKernel(int radius, float sigma);
	};
}