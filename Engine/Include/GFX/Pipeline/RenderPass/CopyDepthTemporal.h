#pragma once
#include "GFX/Pipeline/PassDesc.h"

namespace ZE::GFX::Pipeline::RenderPass::CopyDepthTemporal
{
	struct Resources
	{
		RID SourceDepth;
		RID CopyDepth;
	};

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}