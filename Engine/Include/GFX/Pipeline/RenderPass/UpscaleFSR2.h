#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/ChainPool.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_fsr2.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR2
{
	struct Resources
	{
		RID Color;
		RID Depth;
		RID MotionVectors;
		RID ReactiveMask;
		RID Output;
	};

	struct ExecuteData
	{
		FfxFsr2Context Ctx;
	};

	void Clean(Device& dev, void* data) noexcept;
	ExecuteData* Setup(Device& dev);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}