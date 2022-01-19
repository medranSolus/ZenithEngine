#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/Resource/CBuffer.h"

namespace ZE::GFX::Pipeline::RenderPass::OutlineDraw
{
	struct Resources
	{
		RID RenderTarget;
		RID DepthStencil;
	};

	struct Data
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
		Resource::CBuffer Color;
	};

	inline void Clean(void* data) { delete reinterpret_cast<Data*>(data); }

	Data* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS);
	void Execute(RendererExecuteData& renderData, PassData& passData);
}