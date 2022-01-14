#pragma once
#include "GFX/Binding/Library.h"
#include "GFX/Resource/PipelineStateGfx.h"
#include "FrameBuffer.h"

namespace ZE::GFX::Pipeline
{
	// All structures needed for execution of RenderPass
	struct RendererExecuteData
	{
		Device& Dev;
		CommandList& CL;
		FrameBuffer& Buffers;
		Binding::Library& Bindings;
		Resource::PipelineStateGfx* GraphicsPSO;
	};
}