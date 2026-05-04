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

	struct ExecuteData final : public PassExecuteData
	{
		FfxSssrContext Ctx = {};
		UInt2 RenderSize = { 0, 0 };
		float IblFactor = 0.55f;
		float TemporalStabilityFactor = 0.6f;
		float DepthBufferThickness = 0.02f;
		float RoughnessThreshold = 0.2f;
		float VarianceThreshold = 0.0f;
		U32 MaxTraversalIntersections = 104;
		U32 MinTraversalOccupancy = 4;
		U32 MostDetailedMip = 0;
		U32 SamplesPerQuad = 1;
		bool TemporalVarianceGuidedTracingEnabled = true;
		bool Initialized = false;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData();
	};

	constexpr bool Evaluate() noexcept { return Settings::IsEnabledSSSR(); }

	PassDesc GetDesc() noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}