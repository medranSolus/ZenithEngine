#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapAgX
{
	struct Resources
	{
		RID Scene;
		RID RenderTarget;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
		Float4 Params = { 1.1f, 1.08f, 1.09f, 0.17f }; // Exposure | Saturation boost | Contrast enhancement | Pivot point for contrast (mid-gray)
	};

	constexpr bool Evaluate() noexcept { return Settings::Tonemapper == TonemapperType::AgX; }

	PassDesc GetDesc(PixelFormat outputFormat) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
	void DebugUI(void* data) noexcept;
}