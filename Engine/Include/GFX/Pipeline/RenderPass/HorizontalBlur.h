#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"

namespace ZE::GFX::Pipeline::RenderPass::HorizontalBlur
{
	struct Resources
	{
		RID Outline;
		RID RenderTarget;
	};

	struct Data
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
	};

	inline void Clean(void* data) { delete reinterpret_cast<Data*>(data); }

	Data* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT);
	void Execute(RendererExecuteData& renderData, PassData& passData);
}