#pragma once
#include "Data/CubemapSource.h"
#include "RenderGraphDesc.h"

namespace ZE::GFX::Pipeline::CoreRenderer
{
	// Options to start CoreRenderer with
	struct Params
	{
		// Path to location of the precomputed BRDF LUT texture
		std::string BrdfLutSource;
		// Path to location of the environment map texture
		Data::CubemapSource EnvMapSource;
		// Path to location of the irradiance map texture
		Data::CubemapSource IrrMapSource;
		// Path to location of skybox textures
		Data::CubemapSource SkyboxSource;
		// Dimensions of used shadow maps
		U32 ShadowMapSize = 1024;
		// Constant bias to be applied when performing shadow depth test
		S32 ShadowBias = 26;
		// Factor determining length of offset to be applied along surface normal vector during shadow computation
		float ShadowNormalOffset = 0.001f;
		// Sigma parameter to Gauss function used during computing blur
		float Sigma = 2.6f;
		// Gamma for current display
		float Gamma = 2.2f;
	};

	// Setup only core buffers and samplers slots for given graph
	void SetupRenderSlots(RenderGraphDesc& graphDesc) noexcept;
	// Setup only kernel for the gaussian filter based on blur sigma and radius members
	void SetupBlurKernel(RendererSettingsData& settingsData) noexcept;
	// Setup only core data members for given graph
	void SetupData(RendererSettingsData& settingsData, const Params& params, UInt2 outlineBuffSize) noexcept;

	// Get description of this render graph
	RenderGraphDesc GetDesc(const Params& params) noexcept;
}