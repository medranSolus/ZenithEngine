#pragma once
#include "GFX/Pipeline/PassDesc.h"
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

	struct Data
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
		Resource::CBuffer Direction;
	};

	inline void Clean(void* data) { delete reinterpret_cast<Data*>(data); }

	Data* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS);
	void Execute(RendererExecuteData& renderData, PassData& passData);
}