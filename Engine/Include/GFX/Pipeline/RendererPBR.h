#pragma once
#include "RenderGraph.h"
#include "WorldInfo.h"

namespace ZE::GFX::Pipeline
{
	// Global constant buffer data used by RendererPBR
	struct PBRData
	{
		static constexpr U32 SSAO_KERNEL_MAX_SIZE = 32;

		Matrix ViewProjection;
		Matrix ViewProjectionInverse;
		float NearClip;
		float FarClip;
		float Gamma;
		float GammaInverse;
		Float3 AmbientLight;
		float HDRExposure;
		UInt2 FrameDimmensions;

		struct
		{
			UInt2 NoiseDimmensions;
			float Bias;
			float SampleRadius;
			float Power;
			U32 KernelSize;
			Float4 Kernel[SSAO_KERNEL_MAX_SIZE];
		} SSAO;

		struct
		{
			// Must not exceed coefficients size
			S32 Radius;
			U32 Width;
			U32 Height;
			float Intensity;
			// Should be 6 * sigma - 1, current sigma for best effect 1.3 (but with reduced render target can be 2.6)
			Float4 Coefficients[8];
		} Blur;
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