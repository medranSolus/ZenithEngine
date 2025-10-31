#pragma once
#include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#include "ffx_upscale.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFfxFSR
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
		ffxContext Ctx = nullptr;
		UInt2 DisplaySize = { 0, 0 };
		FfxApiUpscaleQualityMode Quality = FFX_UPSCALE_QUALITY_MODE_NATIVEAA;
		bool SharpeningEnabled = true;
		float Sharpness = 0.7f;
		U64 SelectedVersion = UINT64_MAX;
		U64 PrevSelectedVersion = UINT64_MAX;
		U64 VersionCount = 0;
		std::unique_ptr<U64[]> VersionIds;
		std::unique_ptr<const char* []> VersionNames;
	};

	constexpr bool Evaluate() noexcept { return Settings::Upscaler == UpscalerType::FfxFsr; }

	PassDesc GetDesc() noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, bool firstUpdate = false);
	void* Initialize(Device& dev, RendererPassBuildData& buildData);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
	void DebugUI(void* data) noexcept;
}