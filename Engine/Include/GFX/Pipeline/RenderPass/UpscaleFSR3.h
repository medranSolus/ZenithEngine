#pragma once
#include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_fsr3upscaler.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR3
{
	struct Resources
	{
		RID Color;
		RID Depth;
		RID MotionVectors;
		RID ReactiveMask;
		RID Output;
		RID DilatedDepth;
		RID DilatedMotion;
		RID PrevNearDepth;
	};

	struct ExecuteData
	{
		FfxFsr3UpscalerContext Ctx;
		UInt2 DisplaySize = { 0, 0 };
		FfxFsr3UpscalerQualityMode Quality = FFX_FSR3UPSCALER_QUALITY_MODE_NATIVEAA;
		bool SharpeningEnabled = true;
		float Sharpness = 0.7f;
	};

	constexpr bool Evaluate() noexcept { return Settings::Upscaler == UpscalerType::Fsr3; }

	PassDesc GetDesc() noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, bool firstUpdate = false);
	void* Initialize(Device& dev, RendererPassBuildData& buildData);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
	void DebugUI(void* data) noexcept;
}