#include "MainPipelineGraph.h"
#include "RenderPasses.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Pipeline
{
	void MainPipelineGraph::SetKernel() noexcept(!IS_DEBUG)
	{
		assert(radius <= MAX_RADIUS);
		auto& buffer = kernel->GetBuffer();
		buffer["radius"] = radius;
		float sum = 0.0f;
		for (int i = 0; i <= radius; ++i)
		{
			const float g = Math::Gauss(static_cast<float>(i), sigma);
			sum += g;
			buffer["coefficients"][i] = g;
		}
		for (int i = 0; i <= radius; ++i)
			buffer["coefficients"][i] = static_cast<float>(buffer["coefficients"][i]) / sum;
	}

	MainPipelineGraph::MainPipelineGraph(Graphics& gfx, float hdrExposure, int radius, float sigma, float gamma)
		: RenderGraph(gfx), radius(radius), sigma(sigma), gamma(gamma), hdrExposure(hdrExposure)
	{
		depthOnly = std::make_shared<Resource::DepthStencilShaderInput>(gfx, 8U, Resource::DepthStencil::Usage::DepthOnly);
		AddGlobalSource(RenderPass::Base::SourceDirectBuffer<Resource::DepthStencilShaderInput>::Make("depthOnly", depthOnly));

		geometryBuffer = Resource::RenderTargetEx::Get(gfx, 4U,
			{ DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8G8B8A8_UNORM });
		AddGlobalSource(RenderPass::Base::SourceDirectBuffer<Resource::RenderTargetEx>::Make("geometryBuffer", geometryBuffer));

		sceneTarget = std::make_shared<Resource::RenderTargetShaderInput>(gfx, 0U, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT);
		AddGlobalSource(RenderPass::Base::SourceDirectBuffer<Resource::RenderTargetShaderInput>::Make("sceneTarget", sceneTarget));

		// Setup gamma cbuffer
		Data::CBuffer::DCBLayout gammaLayout;
		gammaLayout.Add(DCBElementType::Float, "gamma");
		gammaLayout.Add(DCBElementType::Float, "deGamma");
		gammaLayout.Add(DCBElementType::Float, "hdrExposure");
		Data::CBuffer::DynamicCBuffer gammaBuffer(std::move(gammaLayout));
		gammaBuffer["gamma"] = gamma;
		gammaBuffer["deGamma"] = 1.0f / gamma;
		gammaBuffer["hdrExposure"] = hdrExposure;
		gammaCorrection = std::make_shared<GFX::Resource::ConstBufferExPixelCache>(gfx, "$gammaBuffer", gammaBuffer, 2U);
		AddGlobalSource(RenderPass::Base::SourceDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("gammaCorrection", gammaCorrection));

		// Setup blur cbuffers
		Data::CBuffer::DCBLayout kernelLayout;
		kernelLayout.Add(DCBElementType::SInteger, "radius");
		kernelLayout.Add(DCBElementType::Float, "intensity");
		kernelLayout.Add(DCBElementType::Array, "coefficients");
		kernelLayout["coefficients"].InitArray(DCBElementType::Float, MAX_RADIUS + 1);
		Data::CBuffer::DynamicCBuffer kernelBuffer(std::move(kernelLayout));
		kernelBuffer["intensity"] = hdrExposure < 1.0f ? 1.0f / hdrExposure : 1.0f;
		kernel = std::make_shared<GFX::Resource::ConstBufferExPixelCache>(gfx, "$kernelBuffer", kernelBuffer, 0U);
		SetKernel();
		AddGlobalSource(RenderPass::Base::SourceDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("kernel", kernel));

		Data::CBuffer::DCBLayout directionLayout;
		directionLayout.Add(DCBElementType::Bool, "vertical");
		Data::CBuffer::DynamicCBuffer directionBuffer(std::move(directionLayout));
		blurDirection = std::make_shared<GFX::Resource::ConstBufferExPixelCache>(gfx, "$blurDirection", directionBuffer, 3U);
		AddGlobalSource(RenderPass::Base::SourceDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("blurDirection", blurDirection));

		skyboxTexture = GFX::Resource::TextureCube::Get(gfx, "Skybox\\Space", ".png");
		AddGlobalSource(RenderPass::Base::SourceDirectBindable<GFX::Resource::TextureCube>::Make("skyboxTexture", skyboxTexture));

		// Setup all passes
		{
			auto pass = std::make_unique<RenderPass::ClearBufferPass>("clearDS");
			pass->SetSinkLinkage("buffer", "$.depthStencil");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::ClearBufferPass>("clearDO");
			pass->SetSinkLinkage("buffer", "$.depthOnly");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::ClearBufferPass>("clearGBuff");
			pass->SetSinkLinkage("buffer", "$.geometryBuffer");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::DepthOnlyPass>(gfx, "depthOnly");
			pass->SetSinkLinkage("depthStencil", "clearDO.buffer");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::LambertianDepthOptimizedPass>(gfx, "lambertianDepthOptimized");
			pass->SetSinkLinkage("geometryBuffer", "clearGBuff.buffer");
			pass->SetSinkLinkage("depthStencil", "depthOnly.depthStencil");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::LambertianClassicPass>(gfx, "lambertianClassic");
			pass->SetSinkLinkage("geometryBuffer", "lambertianDepthOptimized.geometryBuffer");
			pass->SetSinkLinkage("depthStencil", "lambertianDepthOptimized.depthStencil");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::LightingPass>(gfx, "lighting");
			pass->SetSinkLinkage("geometryBuffer", "lambertianClassic.geometryBuffer");
			pass->SetSinkLinkage("depth", "lambertianClassic.depth");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::LightCombinePass>(gfx, "combiner");
			pass->SetSinkLinkage("geometryBuffer", "lambertianClassic.geometryBuffer");
			pass->SetSinkLinkage("lightBuffer", "lighting.lightBuffer");
			pass->SetSinkLinkage("gammaCorrection", "$.gammaCorrection");
			pass->SetSinkLinkage("renderTarget", "$.sceneTarget");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::SkyboxPass>(gfx, "skybox");
			pass->SetSinkLinkage("skyboxTexture", "$.skyboxTexture");
			pass->SetSinkLinkage("gammaCorrection", "$.gammaCorrection");
			pass->SetSinkLinkage("renderTarget", "combiner.renderTarget");
			pass->SetSinkLinkage("depthStencil", "lambertianClassic.depthStencil");
			pass->SetSinkLinkage("depthStencil", "lambertianClassic.depthStencil");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::OutlineGenerationPass>(gfx, "outlineGeneration");
			pass->SetSinkLinkage("depthStencil", "clearDS.buffer");
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
			pass->SetSinkLinkage("renderTarget", "skybox.renderTarget");
			pass->SetSinkLinkage("depthStencil", "outlineGeneration.depthStencil");
			pass->SetSinkLinkage("kernel", "$.kernel");
			pass->SetSinkLinkage("direction", "$.blurDirection");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::WireframePass>(gfx, "wireframe");
			pass->SetSinkLinkage("renderTarget", "verticalBlur.renderTarget");
			pass->SetSinkLinkage("depthStencil", "verticalBlur.depthStencil");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<RenderPass::HDRGammaCorrectionPass>(gfx, "hdrGamma");
			pass->SetSinkLinkage("scene", "wireframe.renderTarget");
			pass->SetSinkLinkage("renderTarget", "$.backbuffer");
			pass->SetSinkLinkage("gammaCorrection", "$.gammaCorrection");
			AppendPass(std::move(pass));
		}
		SetSinkSource("backbuffer", "hdrGamma.renderTarget");
		Finalize();
	}

	void MainPipelineGraph::BindMainCamera(Camera::ICamera& camera)
	{
		dynamic_cast<RenderPass::DepthOnlyPass&>(FindPass("depthOnly")).BindCamera(camera);
		dynamic_cast<RenderPass::LambertianDepthOptimizedPass&>(FindPass("lambertianDepthOptimized")).BindCamera(camera);
		dynamic_cast<RenderPass::LambertianClassicPass&>(FindPass("lambertianClassic")).BindCamera(camera);
		dynamic_cast<RenderPass::LightingPass&>(FindPass("lighting")).BindCamera(camera);
		dynamic_cast<RenderPass::SkyboxPass&>(FindPass("skybox")).BindCamera(camera);
	}

	void MainPipelineGraph::SetKernel(int radius, float sigma) noexcept(!IS_DEBUG)
	{
		this->sigma = sigma;
		this->radius = radius;
		SetKernel();
	}

	void MainPipelineGraph::ShowWindow(Graphics& gfx)
	{
		if (ImGui::DragFloat("Gamma correction", &gamma, 0.1f, 0.1f, 7.9f, "%.1f"))
		{
			gammaCorrection->GetBuffer()["gamma"] = gamma;
			gammaCorrection->GetBuffer()["deGamma"] = 1.0f / gamma;
		}
		if (ImGui::DragFloat("HDR exposure", &hdrExposure, 0.1f, 0.1f, FLT_MAX, "%.1f"))
		{
			gammaCorrection->GetBuffer()["hdrExposure"] = hdrExposure;
			kernel->GetBuffer()["intensity"] = hdrExposure < 1.0f ? 1.0f / hdrExposure : 1.0f;
		}
		dynamic_cast<RenderPass::LightCombinePass&>(FindPass("combiner")).ShowWindow(gfx);
		dynamic_cast<RenderPass::LightingPass&>(FindPass("lighting")).ShowWindow(gfx);
		ImGui::NewLine();
		ImGui::Text("Blur Control");
		if (ImGui::SliderInt("Radius", &radius, 1, MAX_RADIUS) || ImGui::SliderFloat("Sigma", &sigma, 0.1f, 20.0f, "%.1f"))
			SetKernel();
	}
}