#pragma once
#include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#include "nvsdk_ngx_params.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleDLSS
{
	struct Resources
	{
		RID Color;
		RID Depth;
		RID MotionVectors;
		RID Output;
	};

	struct ExecuteData
	{
		UInt2 DisplaySize = { 0, 0 };
		NVSDK_NGX_PerfQuality_Value Quality = NVSDK_NGX_PerfQuality_Value_DLAA;
		bool SharpeningEnabled = true;
		float Sharpness = 0.0f;
		NVSDK_NGX_Parameter* NgxParam = nullptr;
		NVSDK_NGX_Handle* DlssHandle = nullptr;
	};

	constexpr bool Evaluate() noexcept { return Settings::Upscaler == UpscalerType::DLSS; }

	PassDesc GetDesc() noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData);
	void* Initialize(Device& dev, RendererPassBuildData& buildData);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
	void DebugUI(void* data) noexcept;
}