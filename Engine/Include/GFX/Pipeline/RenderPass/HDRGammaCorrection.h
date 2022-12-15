#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"

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

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat outputFormat);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}