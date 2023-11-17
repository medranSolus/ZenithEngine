#pragma once
#include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_fsr1.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR1
{
	struct Resources
	{
		RID Color;
		RID Output;
	};

	struct ExecuteData
	{
		FfxFsr1Context Ctx;
	};

	void Clean(Device& dev, void* data) noexcept;
	ExecuteData* Setup(Device& dev, PixelFormat formatOutput);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}