#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapCollection
{
#pragma pack(push, 1)
	struct TonemapParams
	{
		Float4 Params0 = {};
		float Params1 = 0.0f;
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
		float Exposure = 1.5f;
		TonemapParams Params = {};
		TonemapperType CurrentTonemapper = TonemapperType::LPM;
	};

	constexpr bool Evaluate() noexcept { return Settings::Tonemapper != TonemapperType::LPM; }

	PassDesc GetDesc(PixelFormat outputFormat) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat outputFormat, bool firstCall = false);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
	void DebugUI(void* data) noexcept;
}