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

	constexpr bool Evaluate() noexcept { return Settings::IsEnabledAsyncAO(); }

	PassDesc GetDesc() noexcept;
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}