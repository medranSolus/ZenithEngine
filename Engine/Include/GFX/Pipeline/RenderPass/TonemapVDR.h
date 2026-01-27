#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapVDR
{
#pragma pack(push, 1)
	struct TonemapParams
	{
		float Exposure = 1.5f;
		float Contrast = 1.3f;
		float B = 0.0f;
		float C = 0.0f;
		float Shoulder = 0.995f;
	};
#pragma pack(pop)

	struct Resources
	{
		RID Scene;
		RID RenderTarget;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
		TonemapParams Params = {};
		float MidIn = 0.18f;
		float MidOut = 0.18f;
		float MaxRadiance = 64.0f;
	};

	constexpr bool Evaluate() noexcept { return Settings::Tonemapper == TonemapperType::FilmicVDR; }

	PassDesc GetDesc(PixelFormat outputFormat) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
	void DebugUI(void* data) noexcept;
}