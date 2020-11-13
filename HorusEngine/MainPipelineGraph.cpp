#include "MainPipelineGraph.h"
#include "RenderPasses.h"
#include "GfxResources.h"
#include "Math.h"

#define MakePass(pass, ...) std::make_unique<RenderPass::pass>(__VA_ARGS__)

namespace GFX::Pipeline
{
	inline void MainPipelineGraph::SetupSamplers(Graphics& gfx)
	{
		constexpr GFX::Resource::Sampler::Type types[]
		{
			GFX::Resource::Sampler::Type::Anisotropic,
			GFX::Resource::Sampler::Type::Linear,
			GFX::Resource::Sampler::Type::Point
		};
		UINT slot = 0U;
		samplers.emplace_back(gfx, GFX::Resource::Sampler::Type::Anisotropic, GFX::Resource::Sampler::CoordType::Border, slot++);
		samplers.back().Bind(gfx);
		for (auto& type : types)
		{
			samplers.emplace_back(gfx, type, GFX::Resource::Sampler::CoordType::Wrap, slot++);
			samplers.back().Bind(gfx);
			samplers.emplace_back(gfx, type, GFX::Resource::Sampler::CoordType::Reflect, slot++);
			samplers.back().Bind(gfx);
		}
	}

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

	MainPipelineGraph::MainPipelineGraph(Graphics& gfx, float hdrExposure, int radius, float sigma, float gamma, int bias, float normalOffset)
		: RenderGraph(gfx), bias(bias), radius(radius), sigma(sigma), gamma(gamma), hdrExposure(hdrExposure), normalOffset(normalOffset)
	{
		SetupSamplers(gfx);

		skyboxTexture = GFX::Resource::TextureCube::Get(gfx, "Skybox\\Space", ".png");
		AddGlobalSource(RenderPass::Base::SourceDirectBindable<GFX::Resource::TextureCube>::Make("skyboxTexture", skyboxTexture));

		shadowMapDepth = GfxResPtr<Resource::DepthStencil>(gfx, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, Resource::DepthStencil::Usage::DepthOnly);
		AddGlobalSource(RenderPass::Base::SourceDirectBuffer<Resource::DepthStencil>::Make("shadowMapDepth", shadowMapDepth));
		depthOnly = GfxResPtr<Resource::DepthStencilShaderInput>(gfx, 8U, Resource::DepthStencil::Usage::DepthOnly);
		AddGlobalSource(RenderPass::Base::SourceDirectBuffer<Resource::DepthStencilShaderInput>::Make("depthOnly", depthOnly));

		geometryBuffer = Resource::RenderTargetEx::Get(gfx, 4U, { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT });
		AddGlobalSource(RenderPass::Base::SourceDirectBuffer<Resource::RenderTargetEx>::Make("geometryBuffer", geometryBuffer));
		lightBuffer = Resource::RenderTargetEx::Get(gfx, 9U, { DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT });
		AddGlobalSource(RenderPass::Base::SourceDirectBuffer<Resource::RenderTargetEx>::Make("lightBuffer", lightBuffer));

		shadowMapTarget = GfxResPtr<Resource::RenderTargetShaderInput>(gfx, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 7U, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT);
		AddGlobalSource(RenderPass::Base::SourceDirectBuffer<Resource::RenderTargetShaderInput>::Make("shadowMapTarget", shadowMapTarget));
		sceneTarget = GfxResPtr<Resource::RenderTargetShaderInput>(gfx, 0U, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT);
		AddGlobalSource(RenderPass::Base::SourceDirectBuffer<Resource::RenderTargetShaderInput>::Make("sceneTarget", sceneTarget));

#pragma region CBuffers setup
		// Setup gamma cbuffer
		Data::CBuffer::DCBLayout gammaLayout;
		gammaLayout.Add(DCBElementType::Float, "gamma");
		gammaLayout.Add(DCBElementType::Float, "deGamma");
		gammaLayout.Add(DCBElementType::Float, "hdrExposure");
		Data::CBuffer::DynamicCBuffer gammaBuffer(std::move(gammaLayout));
		gammaBuffer["gamma"] = gamma;
		gammaBuffer["deGamma"] = 1.0f / gamma;
		gammaBuffer["hdrExposure"] = hdrExposure;
		gammaCorrection = GfxResPtr<GFX::Resource::ConstBufferExPixelCache>(gfx, "$gammaBuffer", gammaBuffer, 11U);
		AddGlobalSource(RenderPass::Base::SourceDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("gammaCorrection", gammaCorrection));

		// Setup blur cbuffers
		Data::CBuffer::DCBLayout kernelLayout;
		kernelLayout.Add(DCBElementType::SInteger, "radius");
		kernelLayout.Add(DCBElementType::UInteger, "width");
		kernelLayout.Add(DCBElementType::UInteger, "height");
		kernelLayout.Add(DCBElementType::Float, "intensity");
		kernelLayout.Add(DCBElementType::Array, "coefficients");
		kernelLayout["coefficients"].InitArray(DCBElementType::Float, MAX_RADIUS + 1);
		Data::CBuffer::DynamicCBuffer kernelBuffer(std::move(kernelLayout));
		kernelBuffer["width"] = gfx.GetWidth();
		kernelBuffer["height"] = gfx.GetHeight();
		kernelBuffer["intensity"] = hdrExposure < 1.0f ? 1.0f / hdrExposure : 1.0f;
		kernel = GfxResPtr<GFX::Resource::ConstBufferExPixelCache>(gfx, "$kernelBuffer", kernelBuffer, 12U);
		SetKernel();
		AddGlobalSource(RenderPass::Base::SourceDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("kernel", kernel));

		Data::CBuffer::DCBLayout directionLayout;
		directionLayout.Add(DCBElementType::Bool, "vertical");
		Data::CBuffer::DynamicCBuffer directionBuffer(std::move(directionLayout));
		directionBuffer["vertical"] = true;
		blurDirection = GfxResPtr<GFX::Resource::ConstBufferExPixelCache>(gfx, "$blurDirection", directionBuffer);
		AddGlobalSource(RenderPass::Base::SourceDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("blurDirection", blurDirection));

		// Setup shadow map cbuffer
		Data::CBuffer::DCBLayout biasLayout;
		biasLayout.Add(DCBElementType::Float, "mapSize");
		biasLayout.Add(DCBElementType::Float, "bias");
		biasLayout.Add(DCBElementType::Float, "normalOffset");
		Data::CBuffer::DynamicCBuffer biasBuffer(std::move(biasLayout));
		biasBuffer["mapSize"] = static_cast<float>(SHADOW_MAP_SIZE);
		biasBuffer["bias"] = static_cast<float>(bias) / SHADOW_MAP_SIZE;
		biasBuffer["normalOffset"] = normalOffset;
		shadowBias = GfxResPtr<GFX::Resource::ConstBufferExPixelCache>(gfx, "$shadowBias", biasBuffer, 10U);
		AddGlobalSource(RenderPass::Base::SourceDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("shadowBias", shadowBias));
#pragma endregion

		// Setup all passes
#pragma region Clear passes
		{
			auto pass = MakePass(ClearBufferPass, "clearDS");
			pass->SetSinkLinkage("buffer", "$.depthStencil");
			AppendPass(std::move(pass));
		}
		{
			auto pass = MakePass(ClearBufferPass, "clearDO");
			pass->SetSinkLinkage("buffer", "$.depthOnly");
			AppendPass(std::move(pass));
		}
		{
			auto pass = MakePass(ClearBufferPass, "clearGBuff");
			pass->SetSinkLinkage("buffer", "$.geometryBuffer");
			AppendPass(std::move(pass));
		}
		{
			auto pass = MakePass(ClearBufferPass, "clearLBuff");
			pass->SetSinkLinkage("buffer", "$.lightBuffer");
			AppendPass(std::move(pass));
		}
#pragma endregion
#pragma region Geometry passes
		{
			auto pass = MakePass(LambertianDepthOptimizedPass, gfx, "lambertianDepthOptimized");
			pass->SetSinkLinkage("geometryBuffer", "clearGBuff.buffer");
			pass->SetSinkLinkage("depthStencil", "clearDO.buffer");
			AppendPass(std::move(pass));
		}
		{
			auto pass = MakePass(LambertianClassicPass, gfx, "lambertianClassic");
			pass->SetSinkLinkage("geometryBuffer", "lambertianDepthOptimized.geometryBuffer");
			pass->SetSinkLinkage("depthStencil", "lambertianDepthOptimized.depthStencil");
			AppendPass(std::move(pass));
		}
#pragma endregion
#pragma region Light passes
		{
			auto pass = MakePass(DirectionalLightingPass, gfx, "dirLighting", SHADOW_MAP_SIZE);
			pass->SetSinkLinkage("geometryBuffer", "lambertianClassic.geometryBuffer");
			pass->SetSinkLinkage("depth", "lambertianClassic.depth");
			pass->SetSinkLinkage("lightBuffer", "clearLBuff.buffer");
			pass->SetSinkLinkage("gammaCorrection", "$.gammaCorrection");
			pass->SetSinkLinkage("shadowBias", "$.shadowBias");
			pass->SetSinkLinkage("shadowMapDepth", "$.shadowMapDepth");
			pass->SetSinkLinkage("shadowMapTarget", "$.shadowMapTarget");
			AppendPass(std::move(pass));
		}
		{
			auto pass = MakePass(SpotLightingPass, gfx, "spotLighting");
			pass->SetSinkLinkage("geometryBuffer", "lambertianClassic.geometryBuffer");
			pass->SetSinkLinkage("depth", "lambertianClassic.depth");
			pass->SetSinkLinkage("lightBuffer", "dirLighting.lightBuffer");
			pass->SetSinkLinkage("gammaCorrection", "$.gammaCorrection");
			pass->SetSinkLinkage("shadowBias", "$.shadowBias");
			pass->SetSinkLinkage("shadowMapDepth", "dirLighting.shadowMap.depth");
			pass->SetSinkLinkage("shadowMapTarget", "dirLighting.shadowMap.scratch");
			AppendPass(std::move(pass));
		}
		{
			auto pass = MakePass(PointLightingPass, gfx, "pointLighting", SHADOW_MAP_SIZE);
			pass->SetSinkLinkage("geometryBuffer", "lambertianClassic.geometryBuffer");
			pass->SetSinkLinkage("depth", "lambertianClassic.depth");
			pass->SetSinkLinkage("lightBuffer", "spotLighting.lightBuffer");
			pass->SetSinkLinkage("gammaCorrection", "$.gammaCorrection");
			pass->SetSinkLinkage("shadowBias", "$.shadowBias");
			AppendPass(std::move(pass));
		}
		{
			auto pass = MakePass(SSAOPass, gfx, "ssao");
			pass->SetSinkLinkage("geometryBuffer", "lambertianClassic.geometryBuffer");
			pass->SetSinkLinkage("depth", "lambertianClassic.depth");
			AppendPass(std::move(pass));
		}
		{
			auto pass = MakePass(SSAOBlurPass, gfx, "ssaoHalfBlur");
			pass->SetSinkLinkage("ssaoBuffer", "ssao.ssaoBuffer");
			pass->SetSinkLinkage("ssaoTarget", "ssao.ssaoScratch");
			pass->SetSinkLinkage("direction", "$.blurDirection");
			pass->SetSinkLinkage("ssaoKernel", "ssao.ssaoKernel");
			AppendPass(std::move(pass));
		}
		{
			auto pass = MakePass(SSAOBlurPass, gfx, "ssaoFullBlur");
			pass->SetSinkLinkage("ssaoBuffer", "ssaoHalfBlur.ssaoBuffer");
			pass->SetSinkLinkage("ssaoTarget", "ssaoHalfBlur.ssaoScratch");
			pass->SetSinkLinkage("direction", "$.blurDirection");
			pass->SetSinkLinkage("ssaoKernel", "ssao.ssaoKernel");
			AppendPass(std::move(pass));
		}
		{
			auto pass = MakePass(LightCombinePass, gfx, "lightCombiner");
			pass->SetSinkLinkage("geometryBuffer", "lambertianClassic.geometryBuffer");
			pass->SetSinkLinkage("lightBuffer", "pointLighting.lightBuffer");
			pass->SetSinkLinkage("ssaoBuffer", "ssaoFullBlur.ssaoBuffer");
			pass->SetSinkLinkage("gammaCorrection", "$.gammaCorrection");
			pass->SetSinkLinkage("renderTarget", "$.sceneTarget");
			AppendPass(std::move(pass));
		}
#pragma endregion
		{
			auto pass = MakePass(SkyboxPass, gfx, "skybox");
			pass->SetSinkLinkage("skyboxTexture", "$.skyboxTexture");
			pass->SetSinkLinkage("gammaCorrection", "$.gammaCorrection");
			pass->SetSinkLinkage("renderTarget", "lightCombiner.renderTarget");
			pass->SetSinkLinkage("depthStencil", "lambertianClassic.depthStencil");
			AppendPass(std::move(pass));
		}
#pragma region Post process effects
		{
			auto pass = MakePass(OutlineGenerationPass, gfx, "outlineGeneration");
			pass->SetSinkLinkage("depthStencil", "clearDS.buffer");
			AppendPass(std::move(pass));
		}
		const unsigned int blurTargetWidth = gfx.GetWidth() / 2;
		const unsigned int blurTargetHeight = gfx.GetHeight() / 2;
		AppendPass(MakePass(OutlineDrawBlurPass, gfx, "outlineDrawBlur", blurTargetWidth, blurTargetHeight));
		{
			auto pass = MakePass(HorizontalBlurPass, gfx, "horizontalBlur", blurTargetWidth, blurTargetHeight);
			pass->SetSinkLinkage("blurTarget", "outlineDrawBlur.blurTarget");
			pass->SetSinkLinkage("kernel", "$.kernel");
			pass->SetSinkLinkage("direction", "$.blurDirection");
			AppendPass(std::move(pass));
		}
		{
			auto pass = MakePass(VerticalBlurPass, gfx, "verticalBlur");
			pass->SetSinkLinkage("halfTarget", "horizontalBlur.halfTarget");
			pass->SetSinkLinkage("renderTarget", "skybox.renderTarget");
			pass->SetSinkLinkage("depthStencil", "outlineGeneration.depthStencil");
			pass->SetSinkLinkage("kernel", "$.kernel");
			pass->SetSinkLinkage("direction", "$.blurDirection");
			AppendPass(std::move(pass));
		}
		{
			auto pass = MakePass(WireframePass, gfx, "wireframe");
			pass->SetSinkLinkage("renderTarget", "verticalBlur.renderTarget");
			pass->SetSinkLinkage("depthStencil", "skybox.depthStencil");
			AppendPass(std::move(pass));
		}
		{
			auto pass = MakePass(HDRGammaCorrectionPass, gfx, "hdrGamma");
			pass->SetSinkLinkage("scene", "wireframe.renderTarget");
			pass->SetSinkLinkage("renderTarget", "$.backbuffer");
			pass->SetSinkLinkage("gammaCorrection", "$.gammaCorrection");
			AppendPass(std::move(pass));
		}
#pragma endregion
		SetSinkSource("backbuffer", "hdrGamma.renderTarget");
		Finalize();
	}

	void MainPipelineGraph::BindMainCamera(Camera::ICamera& camera)
	{
		dynamic_cast<RenderPass::LambertianDepthOptimizedPass&>(FindPass("lambertianDepthOptimized")).BindCamera(camera);
		dynamic_cast<RenderPass::LambertianClassicPass&>(FindPass("lambertianClassic")).BindCamera(camera);
		dynamic_cast<RenderPass::DirectionalLightingPass&>(FindPass("dirLighting")).BindCamera(camera);
		dynamic_cast<RenderPass::SpotLightingPass&>(FindPass("spotLighting")).BindCamera(camera);
		dynamic_cast<RenderPass::PointLightingPass&>(FindPass("pointLighting")).BindCamera(camera);
		dynamic_cast<RenderPass::SSAOPass&>(FindPass("ssao")).BindCamera(camera);
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
		if (ImGui::CollapsingHeader("Outline"))
		{
			ImGui::Columns(2, "##outline_options", false);
			ImGui::Text("Blur radius");
			ImGui::SetNextItemWidth(-1.0f);
			bool change = ImGui::SliderInt("##blur_radius", &radius, 1, MAX_RADIUS);
			ImGui::NextColumn();
			ImGui::Text("Outline range");
			ImGui::SetNextItemWidth(-1.0f);
			if (change || ImGui::InputFloat("##blur_sigma", &sigma, 0.1f, 0.0f, "%.1f"))
			{
				if (sigma < 0.1f)
					sigma = 0.1f;
				else if (sigma > 25.0f)
					sigma = 25.0f;
				SetKernel();
			}
			ImGui::Columns(1);
		}
		if (ImGui::CollapsingHeader("Display"))
		{
			ImGui::Columns(2, "##display_options", false);
			ImGui::Text("Gamma correction");
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::InputFloat("##gamma", &gamma, 0.1f, 0.0f, "%.1f"))
			{
				if (gamma < 1.0f)
					gamma = 1.0f;
				else if (gamma > 10.0f)
					gamma = 10.0f;
				gammaCorrection->GetBuffer()["gamma"] = gamma;
				gammaCorrection->GetBuffer()["deGamma"] = 1.0f / gamma;
			}
			ImGui::NextColumn();
			ImGui::Text("HDR exposure");
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::InputFloat("##hdr", &hdrExposure, 0.1f, 0.0f, "%.1f"))
			{
				if (hdrExposure < 0.1f)
					hdrExposure = 0.1f;
				gammaCorrection->GetBuffer()["hdrExposure"] = hdrExposure;
				kernel->GetBuffer()["intensity"] = hdrExposure < 1.0f ? 1.0f / hdrExposure : 1.0f;
			}
			ImGui::Columns(1);
		}
		if (ImGui::CollapsingHeader("Shadows"))
		{
			ImGui::Columns(2, "##shadow_options", false);
			ImGui::Text("Depth bias");
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::InputInt("##depth_bias", &bias))
				shadowBias->GetBuffer()["bias"] = static_cast<float>(bias) / SHADOW_MAP_SIZE;
			ImGui::NextColumn();
			ImGui::Text("Normal offset");
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::InputFloat("##normal_offset", &normalOffset, 0.001f, 0.0f, "%.3f"))
			{
				if (normalOffset < 0.0f)
					normalOffset = 0.0f;
				else if (normalOffset > 1.0f)
					normalOffset = 1.0f;
				shadowBias->GetBuffer()["normalOffset"] = normalOffset;
			}
			ImGui::Columns(1);
			dynamic_cast<RenderPass::LightCombinePass&>(FindPass("lightCombiner")).ShowWindow(gfx);
		}
		dynamic_cast<RenderPass::SSAOPass&>(FindPass("ssao")).ShowWindow(gfx);
	}
}