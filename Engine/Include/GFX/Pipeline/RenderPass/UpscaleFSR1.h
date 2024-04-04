#pragma once
#include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_fsr1.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR1
{
	struct Resources
	{
		RID Color;
		RID Output;
	};

	struct ExecuteData
	{
		FfxFsr1Context Ctx;
		UInt2 DisplaySize = { 0, 0 };
		FfxFsr1QualityMode Quality = FFX_FSR1_QUALITY_MODE_ULTRA_QUALITY;
		bool SharpeningEnabled = true;
		float Sharpness = 0.8f;
	};

	constexpr bool Evaluate(PassData& passData) noexcept { return Settings::GetUpscaler() == UpscalerType::Fsr1; }

	PassDesc GetDesc(PixelFormat formatOutput) noexcept;
	void Clean(Device& dev, void* data) noexcept;
	bool Update(Device& dev, ExecuteData& passData, PixelFormat formatOutput, bool firstUpdate = false);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatOutput);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}