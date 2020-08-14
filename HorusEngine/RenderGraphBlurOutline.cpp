#include "RenderGraphBlurOutline.h"
#include "RenderPasses.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Pipeline
{
	RenderGraphBlurOutline::RenderGraphBlurOutline(Graphics& gfx) : RenderGraph(gfx)
	{
		// Setup blur cbuffers
		Data::CBuffer::DCBLayout kernelLayout;
		kernelLayout.Add(DCBElementType::Integer, "radius");
		kernelLayout.Add(DCBElementType::Array, "coefficients");
		kernelLayout["coefficients"].InitArray(DCBElementType::Float, maxRadius + 1);
		Data::CBuffer::DynamicCBuffer kernelBuffer(std::move(kernelLayout));
		kernel = std::make_shared<GFX::Resource::ConstBufferExPixelCache>(gfx, "$kernelBuffer", kernelBuffer, 0U);
		SetKernel(7, 2.6f);
		AddGlobalSource(RenderPass::Base::SourceDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("kernel", kernel));

		Data::CBuffer::DCBLayout directionLayout;
		directionLayout.Add(DCBElementType::Bool, "vertical");
		Data::CBuffer::DynamicCBuffer directionBuffer(std::move(directionLayout));
		blurDirection = std::make_shared<GFX::Resource::ConstBufferExPixelCache>(gfx, "$blurDirection", directionBuffer, 1U);
		AddGlobalSource(RenderPass::Base::SourceDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("blurDirection", blurDirection));

		// Setup all passes
		{
			auto pass = std::make_unique<RenderPass::ClearBufferPass>("clearRT");
			pass->SetSinkLinkage("buffer", "$.backbuffer");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::ClearBufferPass>("clearDS");
			pass->SetSinkLinkage("buffer", "$.depthStencil");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::LambertianPass>(gfx, "lambertian");
			pass->SetSinkLinkage("renderTarget", "clearRT.buffer");
			pass->SetSinkLinkage("depthStencil", "clearDS.buffer");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::OutlineGenerationPass>(gfx, "outlineGeneration");
			pass->SetSinkLinkage("depthStencil", "lambertian.depthStencil");
			AppendPass(std::move(pass));
		}
		const unsigned int blurTargetWidth = gfx.GetWidth() / 2;
		const unsigned int blurTargetHeight = gfx.GetHeight() / 2;
		AppendPass(std::make_unique<RenderPass::OutlineDrawBlurPass>(gfx, "outlineDrawBlur", blurTargetWidth, blurTargetHeight));
		{
			auto pass = std::make_unique<RenderPass::HorizontalBlurPass>(gfx, "horizontalBlur", blurTargetWidth, blurTargetHeight);
			pass->SetSinkLinkage("blurTarget", "outlineDrawBlur.blurTarget");
			pass->SetSinkLinkage("kernel", "$.kernel");
			pass->SetSinkLinkage("direction", "$.blurDirection");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::VerticalBlurPass>(gfx, "verticalBlur");
			pass->SetSinkLinkage("halfTarget", "horizontalBlur.halfTarget");
			pass->SetSinkLinkage("renderTarget", "lambertian.renderTarget");
			pass->SetSinkLinkage("depthStencil", "outlineGeneration.depthStencil");
			pass->SetSinkLinkage("kernel", "$.kernel");
			pass->SetSinkLinkage("direction", "$.blurDirection");
			AppendPass(std::move(pass));
		}
		SetSinkSource("backbuffer", "verticalBlur.renderTarget");
		Finalize();
	}

	void RenderGraphBlurOutline::SetKernel(int radius, float sigma)
	{
		assert(radius <= maxRadius);
		auto& buffer = kernel->GetBuffer();
		buffer["radius"] = radius;
		float sum = 0.0f;
		for (int i = 0; i <= radius; ++i)
		{
			const float g = gauss(static_cast<float>(i), sigma);
			sum += g;
			buffer["coefficients"][i] = g;
		}
		for (int i = 0; i <= radius; ++i)
			buffer["coefficients"][i] = static_cast<float>(buffer["coefficients"][i]) / sum;
	}
}