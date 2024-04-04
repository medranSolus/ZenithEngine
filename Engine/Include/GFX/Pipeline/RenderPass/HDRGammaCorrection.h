#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::HDRGammaCorrection
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
	};

	inline void Clean(Device& dev, void* data) noexcept { reinterpret_cast<ExecuteData*>(data)->State.Free(dev); delete reinterpret_cast<ExecuteData*>(data); }

	PassDesc GetDesc(PixelFormat outputFormat) noexcept;
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}