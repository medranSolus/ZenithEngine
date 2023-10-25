#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"

namespace ZE::GFX::Pipeline::RenderPass::Velocity
{
	struct Resources
	{
		RID CurrentDepth;
		RID MotionVectors;
		RID PrevDepth;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
	};

	void Clean(Device& dev, void* data) noexcept;
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat motionformat);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}