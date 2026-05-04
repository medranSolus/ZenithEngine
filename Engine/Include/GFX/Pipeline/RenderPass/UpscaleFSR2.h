#pragma once
#include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_fsr2.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR2
{
	struct Resources
	{
		RID Color;
		RID Depth;
		RID MotionVectors;
		RID ReactiveMask;
		RID Output;
	};

	struct ExecuteData final : public PassExecuteData
	{
		FfxFsr2Context Ctx = {};
		UInt2 DisplaySize = { 0, 0 };
		FfxFsr2QualityMode Quality = FFX_FSR2_QUALITY_MODE_QUALITY;
		bool SharpeningEnabled = true;
		float Sharpness = 0.7f;
		bool Initialized = false;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData();
	};

	constexpr bool Evaluate() noexcept { return Settings::Upscaler == UpscalerType::Fsr2; }

	PassDesc GetDesc() noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}