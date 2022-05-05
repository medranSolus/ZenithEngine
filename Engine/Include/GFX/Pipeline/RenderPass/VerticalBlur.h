#pragma once
#include "GFX/Pipeline/RenderLevel.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/Resource/CBuffer.h"

namespace ZE::GFX::Pipeline::RenderPass::VerticalBlur
{
	struct Resources
	{
		RID OutlineBlur;
		RID RenderTarget;
		RID DepthStencil;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
	};

	inline void Clean(void* data) { delete reinterpret_cast<ExecuteData*>(data); }

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}