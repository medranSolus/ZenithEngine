#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/Pipeline/Info/World.h"
#include "GFX/TransformBuffer.h"
#

namespace ZE::GFX::Pipeline::RenderPass::OutlineDraw
{
	struct Resources
	{
		RID RenderTarget;
		RID DepthStencil;
	};

	struct Data
	{
		Info::World& World;
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
		std::vector<Resource::CBuffer> TransformBuffers;
	};

	inline void Clean(void* data) { delete reinterpret_cast<Data*>(data); }

	Data* Setup(Device& dev, RendererBuildData& buildData,
		Info::World& worldData, PixelFormat formatRT, PixelFormat formatDS);
	void Execute(RendererExecuteData& renderData, PassData& passData);
}