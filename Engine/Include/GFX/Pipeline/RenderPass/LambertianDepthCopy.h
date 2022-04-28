#pragma once
#include "GFX/Pipeline/PassDesc.h"

namespace ZE::GFX::Pipeline::RenderPass::LambertianDepthCopy
{
	struct Resources
	{
		RID SourceDepth;
		RID DepthCopyCompute;
	};

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}