#pragma once
#include "RenderGraph.h"
#include "Resource/DepthStencilShaderInput.h"
#include "Resource/RenderTargetEx.h"
#include "Resource/RenderTargetShaderInput.h"
#include "GFX/Resource/ConstBufferExCache.h"
#include "GFX/Resource/Sampler.h"
#include "GFX/Resource/TextureCube.h"
#include "Camera/ICamera.h"
#include <optional>

namespace ZE::GFX::Pipeline
{
	class MainPipelineGraph : public RenderGraph
	{
		static constexpr S32 MAX_RADIUS = 7;
		static constexpr U32 SHADOW_MAP_SIZE = 1024;

		S32 bias;
		S32 radius;
		float sigma;
		float gamma;
		float hdrExposure;
		float normalOffset;

		std::vector<GFX::Resource::Sampler> samplers;
		GfxResPtr<GFX::Resource::TextureCube> skyboxTexture;

		GfxResPtr<Resource::DepthStencil> shadowMapDepth;
		GfxResPtr<Resource::DepthStencilShaderInput> depthOnly;

		GfxResPtr<Resource::RenderTargetEx> geometryBuffer;
		GfxResPtr<Resource::RenderTargetEx> lightBuffer;

		GfxResPtr<Resource::RenderTargetShaderInput> shadowMapTarget;
		GfxResPtr<Resource::RenderTargetShaderInput> sceneTarget;

		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> gammaCorrection;
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> kernel;
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> blurDirection;
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> shadowBias;

		void SetupSamplers(Graphics& gfx);
		void SetKernel() noexcept;

	public:
		MainPipelineGraph(Graphics& gfx, const std::string& skyboxDir = "Skybox/Default", const std::string& skyboxExt = ".jpg",
			float hdrExposure = 1.5f, S32 radius = 7, float sigma = 2.6f, float gamma = 2.2f, S32 bias = 26, float normalOffset = 0.001f);
		virtual ~MainPipelineGraph() = default;

		void BindMainCamera(Camera::ICamera& camera);
		void SetKernel(S32 radius, float sigma) noexcept;
		std::optional<std::string> ChangeSkybox(Graphics& gfx, const std::string& path);
		void ShowWindow(Graphics& gfx);
	};
}