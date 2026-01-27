#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapReinhardX
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
		Float3 Params = { 1.5f, 1.0f, 2.0f }; // Exposure | Offset | MaxWhite
		TonemapperType CurrentTonemapper = TonemapperType::None;
	};

	constexpr bool Evaluate() noexcept { return Settings::Tonemapper == TonemapperType::ReinhardExtended || Settings::Tonemapper == TonemapperType::ReinhardLumaPreserveWhite; }

	PassDesc GetDesc(PixelFormat outputFormat) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat outputFormat, bool firstCall = false);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
	void DebugUI(void* data) noexcept;
}