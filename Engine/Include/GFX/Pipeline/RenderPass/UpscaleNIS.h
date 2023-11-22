#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/Resource/PipelineStateCompute.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleNIS
{
	struct Resources
	{
		RID Color;
		RID Output;
	};

	struct ExecuteData
	{
		U32 BlockHeight;
		U32 BindingIndex;
		Resource::PipelineStateCompute StateUpscale;
		Resource::Texture::Pack Coefficients;
	};

	void Clean(Device& dev, void* data) noexcept;
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}