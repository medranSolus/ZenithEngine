#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/Resource/Texture/Pack.h"

namespace ZE::GFX::Pipeline::RenderPass::SSAO
{
	static constexpr U32 NOISE_SIZE = 32;
	static constexpr U32 NOISE_WIDTH = NOISE_SIZE / 4;
	static constexpr U32 NOISE_HEIGHT = NOISE_SIZE / 8;

	struct Resources
	{
		RID Depth;
		RID Normal;
		RID SSAO;
	};

	struct ExecuteData
	{
		CommandList CL;
		U32 BindingIndexSSAO;
		U32 BindingIndexBlur;
		Resource::PipelineStateCompute StateSSAO;
		Resource::PipelineStateCompute StateBlur;
		Resource::Texture::Pack Noise;
	};

	inline void Clean(void* data) { delete reinterpret_cast<ExecuteData*>(data); }

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}