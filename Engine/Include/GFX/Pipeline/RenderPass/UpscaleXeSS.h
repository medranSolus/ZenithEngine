#pragma once
#include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#include "xess/xess.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleXeSS
{
	struct Resources
	{
		RID Color;
		RID Depth;
		RID MotionVectors;
		RID ResponsiveMask;
		RID Output;
	};

	constexpr void Clean(Device& dev, void* data) noexcept {}

	void* Setup(Device& dev);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}