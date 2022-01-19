#pragma once
#include "RenderGraph.h"

namespace ZE::GFX::Pipeline
{
	// Global constant buffer data used by RendererPBR
	struct PBRData
	{
		Float3 CameraPos;
		float NearClip;
		float FarClip;
		float Gamma;
		float GammaInverse;
		float HDRExposure;
		Matrix ViewProjection;
		Matrix ViewProjectionInverse;
		// Add gauss blur kernel
	};

	// Physically Based Renderer
	class RendererPBR : public RenderGraph
	{
		PBRData settingsData;

	public:
		RendererPBR() = default;
		ZE_CLASS_DELETE(RendererPBR);
		virtual ~RendererPBR() = default;

		void Init(Device& dev, CommandList& mainList, Resource::Texture::Library& texLib,
			U32 width, U32 height, bool minimizePassDistances, U32 shadowMapSize);
	};
}