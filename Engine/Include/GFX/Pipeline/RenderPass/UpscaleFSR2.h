#pragma once
#include "GFX/Resource/Texture/Pack.h"
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/ChainPool.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR2
{
	struct Resources
	{
		RID Depth;
	};

	struct ExecuteData
	{
		FfxFsr2Context Ctx;

		ChainPool<CommandList> ListChain;
		U32 BindingIndexPrefilter;
		U32 BindingIndexSSAO;
		U32 BindingIndexDenoise;
		Resource::PipelineStateCompute StatePrefilter;
		Resource::PipelineStateCompute StateSSAO;
		Resource::PipelineStateCompute StateDenoise;
		Resource::Texture::Pack HilbertLUT;
	};

	void Clean(Device& dev, void* data) noexcept;
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}