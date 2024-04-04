#pragma once
#include "RenderGraphDesc.h"

namespace ZE::GFX::Pipeline::CoreRenderer
{
	// Options to start CoreRenderer with
	struct Params
	{
		// Path to location of skybox textures
		std::string SkyboxPath;
		// Extensions of skybox textures
		std::string SkyboxExt;
		// When computing render graph for passes minimize distances between dependant passes.
		// Disables kicking off work earlier, trying to run pass as late as possible.
		//
		// WARNING: may result in higher or lower memory reservation for frame in some cases
		//   but can sometimes leverage GPU consumption if most of heavy work is done early, measure it for specific use case
		bool MinimizeRenderPassDistances = false;
		// Dimensions of used shadow maps
		U32 ShadowMapSize = 1024;
		// Constant bias to be applied when performing shadow depth test
		S32 ShadowBias = 26;
		// Factor determining length of offset to be applied along surface normal vector during shadow computation
		float ShadowNormalOffset = 0.001f;
		// Sigma parameter to Gauss function used during computing blur
		float Sigma = 2.6f;
		// Level of exposure during tonemapping
		float HDRExposure = 1.5f;
		// Gamma for current display
		float Gamma = 2.2f;
	};

	// Setup only core buffers and samplers slots for given graph
	void SetupRenderSlots(RenderGraphDesc& graphDesc) noexcept;
	// Setup only core data members for given graph
	void SetupData(RendererSettingsData& settingsData, const Params& params, UInt2 outlineBuffSize) noexcept;

	// Get description of this render graph
	RenderGraphDesc GetDesc(const Params& params) noexcept;
}