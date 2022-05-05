#pragma once
#include "GFX/Pipeline/RenderLevel.h"

namespace ZE::GFX::Pipeline::RenderPass::LambertianComputeCopy
{
	struct Resources
	{
		RID SourceDepth;
		RID SourceNormal;
		RID DepthCopy;
		RID NormalCopy;
	};

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}