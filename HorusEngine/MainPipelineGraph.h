#pragma once
#include "RenderGraph.h"
#include "DepthStencilShaderInput.h"
#include "RenderTargetEx.h"
#include "RenderTargetShaderInput.h"
#include "ConstBufferExCache.h"
#include "TextureCube.h"
#include "ICamera.h"
#include "BaseLight.h"

namespace GFX::Pipeline
{
	class MainPipelineGraph : public RenderGraph
	{
		static constexpr int MAX_RADIUS = 7;

		int radius;
		float sigma;
		float gamma;
		float hdrExposure;

		std::shared_ptr<Resource::DepthStencilShaderInput> depthOnly;
		std::shared_ptr<Resource::RenderTargetEx> geometryBuffer;
		std::shared_ptr<Resource::RenderTargetShaderInput> sceneTarget;
		std::shared_ptr<Resource::RenderTargetShaderInput> blurSSAO;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> gammaCorrection;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> kernel;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> blurDirection;
		std::shared_ptr<GFX::Resource::TextureCube> skyboxTexture;

		void SetKernel() noexcept(!IS_DEBUG);

	public:
		MainPipelineGraph(Graphics& gfx, float hdrExposure = 1.5f, int radius = 7, float sigma = 2.6f, float gamma = 2.2f);
		virtual ~MainPipelineGraph() = default;

		void BindMainCamera(Camera::ICamera& camera);
		void SetKernel(int radius, float sigma) noexcept(!IS_DEBUG);
		void ShowWindow(Graphics& gfx);
	};
}