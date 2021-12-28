#pragma once
#include "GFX/Material/Factory.h"
#include "GFX/CommandList.h"
#include "FrameBuffer.h"

namespace ZE::GFX::Pipeline
{
	// All structures needed for execution of RenderPass
	struct RendererExecuteData
	{
		Device& Dev;
		CommandList& CL;
		FrameBuffer& Buffers;
		Material::Factory& Bindins;
		Resource::PipelineStateGfx* GraphicsPSO;
	};
}