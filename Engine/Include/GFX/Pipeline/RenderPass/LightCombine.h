#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"

namespace ZE::GFX::Pipeline::RenderPass::LightCombine
{
	struct Resources
	{
		RID GBufferColor;
		RID LightColor;
		RID LightSpecular;
		RID SSAO;
		RID RenderTarget;
	};

	struct ExecuteData
	{
		U32 BindingIndexAO;
		U32 BindingIndexNoAO;
		Resource::PipelineStateGfx StateAO;
		Resource::PipelineStateGfx StateNoAO;
	};

	void Clean(Device& dev, void* data) noexcept;
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat outputFormat);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}