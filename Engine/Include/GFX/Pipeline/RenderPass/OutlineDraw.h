#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/TransformBuffer.h"
#

namespace ZE::GFX::Pipeline::RenderPass::OutlineDraw
{
	// Indicates that entity is inside view frustum
	struct InsideFrustum {};

	struct Resources
	{
		RID RenderTarget;
		RID DepthStencil;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx StateStencil;
		Resource::PipelineStateGfx StateRender;
		std::vector<Resource::CBuffer> TransformBuffers;
	};

	inline void Clean(void* data) { delete reinterpret_cast<ExecuteData*>(data); }

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}