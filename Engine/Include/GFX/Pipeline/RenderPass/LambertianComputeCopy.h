#pragma once
#include "GFX/Pipeline/PassDesc.h"

namespace ZE::GFX::Pipeline::RenderPass::LambertianComputeCopy
{
	struct Resources
	{
		RID SourceDepth;
		RID SourceNormal;
		RID CopyDepth;
		RID CopyNormal;
	};

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}