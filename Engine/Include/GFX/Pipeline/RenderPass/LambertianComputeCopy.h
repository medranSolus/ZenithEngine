#pragma once
#include "GFX/Pipeline/PassDesc.h"

namespace ZE::GFX::Pipeline::RenderPass::LambertianComputeCopy
{
	struct Resources
	{
		RID SourceNormal;
		RID SourceDepth;
		RID CopyNormal;
		RID CopyDepth;
	};

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}