#pragma once
#include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_sssr.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::SSSR
{
	struct Resources
	{
		RID Color;
		RID Depth;
		RID NormalMap;
		RID MaterialData;
		RID MotionVectors;
		RID EnvironmentMap;
		RID BrdfLut;
		RID SSSR;
	};

	struct ExecuteData
	{
		FfxSssrContext Ctx;
		UInt2 RenderSize = { 0, 0 };
		float IblFactor = 0.55f;
		float TemporalStabilityFactor = 0.7f;
		float DepthBufferThickness = 0.015f;
		float RoughnessThreshold = 0.2f;
		float VarianceThreshold = 0.0f;
		U32 MaxTraversalIntersections = 128;
		U32 MinTraversalOccupancy = 4;
		U32 MostDetailedMip = 0;
		U32 SamplesPerQuad = 1;
		bool TemporalVarianceGuidedTracingEnabled = true;
	};

	constexpr bool Evaluate(PassData& passData) noexcept { return Settings::IsEnabledSSSR(); }

	PassDesc GetDesc() noexcept;
	void Clean(Device& dev, void* data) noexcept;
	void Update(Device& dev, ExecuteData& passData, bool firstUpdate = false);
	void* Initialize(Device& dev, RendererPassBuildData& buildData);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}