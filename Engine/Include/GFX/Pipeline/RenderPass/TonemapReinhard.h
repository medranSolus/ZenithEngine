#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapReinhard
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
		Float2 Params = { 1.5f, 1.0f }; // Exposure | Offset
		TonemapperType CurrentTonemapper = TonemapperType::None;
	};

	constexpr bool Evaluate() noexcept { return Settings::Tonemapper == TonemapperType::Reinhard || Settings::Tonemapper == TonemapperType::ReinhardLuma || Settings::Tonemapper == TonemapperType::ReinhardLumaJodie; }

	PassDesc GetDesc(PixelFormat outputFormat) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat outputFormat, bool firstCall = false);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
	void DebugUI(void* data) noexcept;
}