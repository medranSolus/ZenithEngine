#pragma once
#include "RenderGraph.h"
#include "WorldInfo.h"

namespace ZE::GFX::Pipeline
{
	// Global constant buffer data used by RendererPBR
	struct PBRData
	{
		Matrix ViewProjection;
		Matrix ViewProjectionInverse;
		float NearClip;
		float FarClip;
		float Gamma;
		float GammaInverse;
		float HDRExposure;
		// Add gauss blur kernel
	};

	// Physically Based Renderer
	class RendererPBR : public RenderGraph
	{
		static constexpr U64 MESH_LIST_GROW_SIZE = 64;

		WorldInfo worldData;
		PBRData settingsData;

	public:
		RendererPBR() = default;
		ZE_CLASS_DELETE(RendererPBR);
		~RendererPBR();

		constexpr void SetActiveScene(const Data::Scene& scene) noexcept { worldData.ActiveScene = &scene; }

		void Init(Device& dev, CommandList& mainList, Resource::Texture::Library& texLib,
			U32 width, U32 height, bool minimizePassDistances, U32 shadowMapSize);
		void SetCurrentCamera(Device& dev, Data::EID camera);
		void UpdateWorldData() noexcept;
	};
}