#pragma once
#include "RenderGraph.h"
#include "DepthStencilShaderInput.h"
#include "RenderTargetEx.h"
#include "RenderTargetShaderInput.h"
#include "ConstBufferExCache.h"
#include "Sampler.h"
#include "TextureCube.h"
#include "ICamera.h"

namespace GFX::Pipeline
{
	class MainPipelineGraph : public RenderGraph
	{
		static constexpr int MAX_RADIUS = 7;
		static constexpr UINT SHADOW_MAP_SIZE = 1024U;

		int bias;
		int radius;
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

		inline void SetupSamplers(Graphics& gfx);
		void SetKernel() noexcept(!IS_DEBUG);

	public:
		MainPipelineGraph(Graphics& gfx, float hdrExposure = 1.5f, int radius = 7, float sigma = 2.6f, float gamma = 2.2f, int bias = 26, float normalOffset = 0.001f);
		virtual ~MainPipelineGraph() = default;

		void BindMainCamera(Camera::ICamera& camera);
		void SetKernel(int radius, float sigma) noexcept(!IS_DEBUG);
		void ShowWindow(Graphics& gfx);
	};
}