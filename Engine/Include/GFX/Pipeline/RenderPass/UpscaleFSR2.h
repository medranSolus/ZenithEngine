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

	struct ExecuteData
	{
		FfxFsr2Context Ctx;
		UInt2 DisplaySize = { 0, 0 };
		FfxFsr2QualityMode Quality = FFX_FSR2_QUALITY_MODE_QUALITY;
		bool SharpeningEnabled = true;
		float Sharpness = 0.7f;
	};

	constexpr bool Evaluate() noexcept { return Settings::GetUpscaler() == UpscalerType::Fsr2; }

	PassDesc GetDesc() noexcept;
	void Clean(Device& dev, void* data) noexcept;
	bool Update(Device& dev, const FfxInterface& ffxInterface, ExecuteData& passData, bool firstUpdate = false);
	void* Initialize(Device& dev, const FfxInterface& ffxInterface);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}