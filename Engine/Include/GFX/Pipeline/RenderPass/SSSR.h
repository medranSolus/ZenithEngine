#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/ChainPool.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_sssr.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::SSSR
{
	struct Resources
	{
		RID Color;
		RID Depth;
		RID MotionVectors;
		RID NormalMap;
		RID Roughness;
		RID EnvironmentMap;
		RID BrdfLut;
		RID SSSR;
	};

	struct ExecuteData
	{
		FfxSssrContext Ctx;
		ChainPool<CommandList> ListChain;
	};

	void Clean(Device& dev, void* data) noexcept;
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, U32 renderWidth, U32 renderHeight);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}