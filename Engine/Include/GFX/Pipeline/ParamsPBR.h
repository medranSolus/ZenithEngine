#pragma once

namespace ZE::GFX::Pipeline
{
	// Options to start RendererPBR with
	struct ParamsPBR
	{
		// Path to location of skybox textures
		std::string SkyboxPath;
		// Extensions of skybox textures
		std::string SkyboxExt;
		// When computing render graph for passes minimize distances between dependant passes.
		// Disables kicking off work earlier, trying to run pass as late as possible.
		//
		// WARNING: may result in higher or lower memory reservation for frame in some cases
		//   but can sometimes lowerage GPU consumption if most of heavy work is done early, measure it for specific use case
		bool MinimizeRenderPassDistances = false;
		// Dimensions of used shadow maps
		U32 ShadowMapSize = 1024;
		// Sigma parameter to Gauss function used during computing blur
		float Sigma = 2.6f;
		// Level of exposure during tonemapping
		float HDRExposure = 1.5f;
		// Gamma for current display
		float Gamma = 2.2f;
	};
}