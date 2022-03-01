#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/Pipeline/Info/World.h"
#include "GFX/TransformBuffer.h"

namespace ZE::GFX::Pipeline::RenderPass::ShadowMap
{
	constexpr U64 BUFFER_SHRINK_STEP = 2;

	struct Resources
	{
		RID RenderTarget;
		RID Depth;
	};

	struct Data
	{
		Info::World& World;
		U32 BindingIndex;
		Resource::PipelineStateGfx StateDepth;
		Resource::PipelineStateGfx StateNormal;
		std::vector<Resource::CBuffer> TransformBuffers;
		Matrix Projection;
	};

	void Setup(Device& dev, RendererBuildData& buildData, Data& passData,
		PixelFormat formatDS, PixelFormat formatRT, Matrix&& projection);
	Matrix Execute(RendererExecuteData& renderData, Data& data, const Resources& ids,
		const Float3& lightPos, const Float3& lightDir);
}